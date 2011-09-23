/* ----------------------------------------------------------------
 * rm_pbs.c
 *
 *      PBS server implementation of the OGSA Basic Execution Services
 *
 * Copyright (C) Arkaitz Ruiz Alvarez.
 *                  arkaitz@cs.virginia.edu
 *               Platform Computing Corporation.
 *               All Rights Reserved.
 *
 * This file is part of Besserver.
 *
 * Besserver is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

//Includes
#include "rm.h"
#include "rm_util.h"
#include "faults.h"
#include "job.h"
#include "../config.h"
#include <pbs_error.h>
#include <pbs_ifl.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

#define PBS "PBS Resource Manager"

//GLOBAL VARIABLES

//! error code of the last error ocurred
int errorno = 0;
//! server to connect to
char* server;
//! stores information about the cluster
struct rm_clusterInfo* clusterInfo;
//! stores the resources of the cluster
struct rm_resource* resourceList;
//! number of resources in @resourceList
int * nresources;
//!Compatibility with previous versions of PBS
size_t cred_len = 0;
//!Compatibility with previous versions of PBS
char* cred_buf;
//!Compatibility with previous versions of PBS
int cred_type;

/**
 * Updates the error code. 
 *
 * It takes the PBS error variable (pbs_errno) and 
 * fills the errono variable of this module.
 */
void updateErrorNo()
{
   switch(pbs_errno){
      case PBSE_NONE: errorno = BESE_OK; break;
      case PBSE_UNKJOBID: errorno = BESE_NO_ACTIVITY; break;
      case PBSE_PERM: errorno = BESE_PERMISSION; break;
      default:fprintf(stderr, "Unkwon PBS error number %u", pbs_errno); break;
   } 
}

/**
 * Converts a PBS job status struct to a jobcard struct
 *
 * @param jobInfo struct with the information about the job
 * @param soap is used to allocate memory deallocatable by the gSOAP library
*/ 
void convertJobInfo(struct soap* soap,
		 struct jobcard* jobInfo, 
		 struct batch_status* status)
{	
   struct attrl* attrList = status->attribs;
   //default value
   jobInfo->tcpu= 1;

   while (attrList != NULL)
   {
      if(!strcmp(attrList->name, ATTR_v))
         getRequestInfo(soap, attrList->value, jobInfo);
      attrList = attrList->next;
   }
}

/**
 * Connects to the PBS queue server and loads cluster info.
 * @param soap is used to allocate memory
 * @param serverName is a string with the hostname in which the queue is running
 * @return 0 if connection established, 1 if error
 */
int rm_initialize(struct soap* soap, char* serverName){
   int connectionIdentifier;
   int error_code = BESE_OK;

   if (!serverName)
	   return BESE_BAD_ARG;
   server = (char*) malloc(strlen(serverName) + 1);
   nresources = (int*) malloc(sizeof(int));
   strcpy(server,serverName);
   connectionIdentifier = pbs_connect(serverName);
   if (connectionIdentifier <= 0 )
	   return BESE_BACKEND;
   pbs_disconnect(connectionIdentifier);
   error_code = rm_getClusterInfo(soap, &clusterInfo);
   if (error_code != BESE_OK)
      return error_code;
   else {
      printf("Looking for resources now");
      error_code = rm_getResourceList(soap, NULL, &resourceList, nresources);
      return error_code;
   }
}
/**
 * Prints in stderr the error message of the last error. 
 *
 * It gets the error 
 * description of the last error that happened in the PBS queue server.
 * @param userMessage message to append to the output. It may contain 
 * additional information from the main program
 */
void printError(char* userMessage)
{
    char *errorMessage;
    int connectionIdentifier = pbs_connect(server);
    errorMessage = pbs_geterrmsg(connectionIdentifier);
    pbs_disconnect(connectionIdentifier);
    fprintf(stderr, "%s\n%s\n", userMessage, errorMessage);
}


/**
 * Terminates a job. 
 * @param jobid is the PID assigned by the queue
 * @return 0 if correct, non-zero if error
 */
int rm_terminateJob(struct soap* s, char* jobid, char* user)
{
   int connectionIdentifier = pbs_connect(server);
   if (connectionIdentifier < 1 )
	   return BESE_BACKEND;
   int rc = pbs_deljob(connectionIdentifier, jobid, NULL);
   updateErrorNo();
   pbs_disconnect(connectionIdentifier);
   return BESE_OK;
}

/**
 * Gets the information about a job. 
 *
 * It stores the info in a module variable.
 * In order to retrieve it, use @see readJobInfo.
 * @param jobid is the PID assigned by the queue
 * @return 0 if correct, non-zero if error
 */
int rm_getJobInfo(struct soap* soap, char* jobid, char* user, 
		struct jobcard** jobInfo )
{
   //! stores the status of a job
   struct batch_status* status;
   int connectionIdentifier;
   struct jobcard* job;

   connectionIdentifier = pbs_connect(server);
   if(!connectionIdentifier)
      return BESE_BACKEND;
   status = pbs_statjob(connectionIdentifier, jobid, NULL, NULL);
   pbs_disconnect(connectionIdentifier);
   if(status == NULL)
      return BESE_NO_ACTIVITY;
   job = (struct jobcard*)soap_malloc(soap, sizeof(struct jobcard));
   if (!job)
	   return BESE_MEM_ALLOC;
   memset(job, 0, sizeof(struct jobcard));

   fillJobStatusDefaults(job);
   convertJobInfo(soap, job, status);
   *jobInfo = job;
   pbs_statfree(status);
   return BESE_OK;
}

/**
 * Gets the status of the job. 
 *
 * It maps the different states of PBS jobs to
 * pending and running. It does not make a difference between finished, 
 * cancelled, terminated and unknown jobs since PBS does not store this info.
 * @param jobid is the PID assigned by the queue
 * @return 0 if correct, non-zero if error
 */
int rm_getJobStatus(struct soap* s, char* jobid, char* user, struct bes__ActivityStatusType** jobStatus)
{
   struct bes__ActivityStatusType *activityStatus;
   int connectionIdentifier;
   //! stores the status of a job
   struct batch_status* status;

   if (!jobid || !jobStatus) {
      return BESE_BAD_ARG;
   }
   connectionIdentifier = pbs_connect(server);
   if (!connectionIdentifier)
	   return BESE_BACKEND;
   status = pbs_statjob(connectionIdentifier,jobid,NULL,NULL);
   pbs_disconnect(connectionIdentifier);
   if(status == NULL)
   {
      return BESE_NO_ACTIVITY;
   }
   activityStatus = (struct bes__ActivityStatusType*)soap_malloc(s, sizeof(struct bes__ActivityStatusType));
   if (activityStatus == NULL) {
      return BESE_MEM_ALLOC;
   }
   memset(activityStatus, 0, sizeof(struct bes__ActivityStatusType));
   struct attrl* attrList = status->attribs;
   while (attrList != NULL)
   {
      if(!strcmp(attrList->name, ATTR_state))
      {
        if(!strcmp(attrList->value, "T")) {
           activityStatus->state = Pending;
        }
        else if(!strcmp(attrList->value, "Q")) {
           activityStatus->state = Pending;
        }         
        else if(!strcmp(attrList->value,"H")) {
           activityStatus->state = Pending;
	}         
        else if(!strcmp(attrList->value,"W")){
           activityStatus->state = Pending;
        }         
        else if(!strcmp(attrList->value,"R")){
           activityStatus->state = Running;
        }
        else if(!strcmp(attrList->value,"E")) {
           activityStatus->state = Finished;
        }
        pbs_statfree(status);
	*jobStatus = activityStatus;
        return BESE_OK;
     }
     attrList = attrList->next;
  }
  pbs_statfree(status);
  return BESE_NO_ACTIVITY;
}

/**
 * Gets a filtered list of jobs in the queue.
 *
 * This function issues a qstat and then parses the output file to add to 
 * the list the jobs that pass the filters.
 * @param soap is needed to allocate memory that can be deallocated by the 
 * gsoap library after.
 * @param filters are the filters that are going to be applied. See 
 * HPCBP Advanced Filter Extension.
 * @param joblist list of jobs to return
 * @param numjobs number of jobs returned
 */
int rm_getJobList(struct soap*soap, struct rm_filter* filters,
		struct rm_job** joblist, int* numjobs)
{
   char filename[128];
   char command[256];
   char ignore[128];
   struct rm_filter* filter;
   struct rm_job* jobList;
   int numJobs, fd, jobid, scan;
   FILE * in;

   strcpy(filename, "/tmp/queueStatus.XXXXXX");
   fd = mkstemp(filename);
   if (fd == -1) {
      return BESE_SYS_ERR;
   }
   close(fd);

   sprintf(command,"qstat -f | grep '\\(Job Id\\)\\|\\(Job_Owner\\)\\|\\(qtime\\)\\|\\(job_state\\)' > %s ",filename);
   //printf(command);

   system(command);   

   in = fopen(filename, "r");
   if (in == NULL)
      return BESE_SYS_ERR;

   numJobs = 0;
   scan = fscanf(in, "Job Id: %d.%s\n", &jobid, ignore);
   filter = filters;
   while(scan!=EOF){
      char state[64];
      char user[32];
      char week[16],month[16];
      int day, hour, minute, second, year;
      int skip = 0, found = 0, filterPresent = 0;
      fscanf(in,"\tJob_Owner = %s\n",user);
      fscanf(in,"job_state = %s\n",state);
      fscanf(in,"qtime = %s %s %d %d:%d:%d %d\n",week,month,&day,&hour,&minute,&second,&year);

      //Chek for the users if we have to
      filter=filters;
      found=0;
      filterPresent=0;
      while((filter != NULL)&&(!found)){
         if(filter->user != NULL){
            filterPresent = 1;
            if(!strncmp(filter->user, user, strlen(filter->user)))
               found=1;
         }
         filter = filter->next;
      }
      if(!found && filterPresent)
         skip = 1;

      //Chek for the state
      filter = filters;
      found = 0;
      filterPresent = 0;
      while((filter != NULL) && (!skip) && (!found)){
         if(filter->state != NULL){
            filterPresent = 1;
            if(!strcmp(filter->state, "Pending"))
               if(!strcmp(state,"Q"))
                  found = 1;
            if(!strcmp(filter->state, "Running"))
               if(!strcmp(state,"R"))
                  found = 1;
         }
         filter = filter->next;
      }
      if(!found && filterPresent)
         skip = 1;

      //Check for the ID interval
      filter = filters;
      found = 0;
      filterPresent = 0;
      while((filter != NULL) && (!skip) && (!found)){
         if((filter->startRange>0)&&(filter->endRange>0)){
            filterPresent=1;
            if((jobid>=filter->startRange) &&(jobid<=filter->endRange))
               found=1;
         }
         filter = filter->next;
      }
      if(!found && filterPresent)
         skip = 1;

      //Check for the time interval
      filter = filters;
      found = 0;
      filterPresent = 0;
      while((filter != NULL)&&(skip == 0)&&(!found)){
         if((filter->endTime!=NULL)&&(filter->startTime!=NULL)){
            char date[64];
            filterPresent=1;
            sprintf(date,"%4d%2d%2d%T%2d:%2d:%2d",year,monthToInt(month),day,hour,minute,second);
            if((strcmp(date,filter->endTime)<0)&&(strcmp(date,filter->startTime)>0))
               found=1;
         }
         filter=filter->next;
      }
      if(!found&&filterPresent)
         skip=1;

      if(!skip){
         struct rm_job* newJob=(struct rm_job *)soap_malloc(soap, 
			 sizeof(struct rm_job));
	 char jobidStr[sizeof(int)*8+1];
	 if (!newJob)
		 return BESE_MEM_ALLOC;
	 memset(newJob, 0, sizeof(struct rm_job));
	 sprintf(jobidStr,"%d",jobid);
         newJob->jobid = soap_strdup(soap, jobidStr);
         newJob->next = jobList;
         jobList = newJob;
	 numJobs++;
      }

      scan = fscanf(in,"Job Id: %d.%s\n",&jobid,ignore);
   }
   fclose(in);
   unlink(filename);
   *joblist = jobList;
   *numjobs = numJobs;

   return BESE_OK;
}


/**
 * Gets the factory attributes. 
 *
 * This function uses @see loadResourceFile 
 * and also queries the PBS queue.
 * @param soap is needed to allocate memory that can be deallocated by the 
 * gsoap library after.
 * @param clusterInf a struct of type clusterInfo with the information needed for the
 * factory attributes document
 */
int rm_getClusterInfo(struct soap*soap, struct rm_clusterInfo** clusterInf
		/*,int compactResources*/)
{
   char outputFile[256];   
   FILE* fd;
   int rc;
   char resource[128];
   int connectionIdentifier = pbs_connect(server);
   struct rm_clusterInfo* clusterInfo;
   struct rm_resource* resourcesInfo;
   struct batch_status* status;


   if (!clusterInf) {
      return BESE_BAD_ARG;
   }
   clusterInfo = (struct rm_clusterInfo*) soap_malloc(soap,
		   sizeof(struct rm_clusterInfo));
   if (clusterInfo == NULL)
      return BESE_MEM_ALLOC;
   memset(clusterInfo, 0, sizeof(struct rm_clusterInfo));
   
   //First, contact the PBS queue
   status = pbs_statserver(connectionIdentifier,NULL,NULL);
   if(status != NULL)
   {
   //Loop over the list of attributes returned
      struct attrl* attributeList = status->attribs;
      while(attributeList != NULL)
      {
	  //Server_host for the CommonName element
         if(!strcmp(attributeList->name, "server_host"))
         {
            clusterInfo->CommonName = soap_strdup(soap,
			   attributeList->value);
         }
	 //Server_state for the IsAcceptingNewActivities element
         else if(!strcmp(attributeList->name, "server_state"))
         {
            if(!strcmp(attributeList->value, "Active"))
               clusterInfo->IsAcceptingNewActivities = true_;
            else
               clusterInfo->IsAcceptingNewActivities = false_;
         }//total_jobs for the TotalNumberOfActivities element
         else if(!strcmp(attributeList->name, "total_jobs"))
         {
            //clusterInfo->TotalNumberOfActivities = 
	//	    atoi(attributeList->value);
         }//pbs_version for the LocalResourceManagerType element
         else if(!strcmp(attributeList->name, "pbs_version"))
         {
            char* pbsStr = (char*) soap_malloc(soap, strlen(PBS) + 
			              strlen(attributeList->value) + 10);
	    sprintf(pbsStr, "%s %s %s", PBS, "Version", attributeList->value);
            clusterInfo->LocalResourceManagerType = pbsStr;
         }
         //fprintf(stderr,"Attribute: %s - Value: %s\n",attributeList->name,attributeList->value);
         attributeList = attributeList->next;
      }
   }
 
   pbs_statfree(status);
   pbs_disconnect(connectionIdentifier); 
   *clusterInf = clusterInfo;

   return BESE_OK;
}

/**
 * Submits a job to the queuing system. 
 *
 * It generates a script file and sends 
 * it to the queue.
 * @param jc stores all the information about the job
 * @param soap is used to allocate memory that can be freed by the gsoap 
 * library
 * @param message is the description of the error ocurred (if any)
 * @return job PID if correct,  negative integer if error
 */
int rm_submitJob(struct soap* soap, struct jobcard *jc,
	       char* user, char ** return_jobid)
{
  static char fname[] = "submitJob";
  char scriptname[64];
  int fd, i, rc, jobid = 0, rr = 0;
  FILE *script;

  fprintf(stderr, "In submitJob...\n");

  if (!jc || !jc->executable||!jc->username) {
    fprintf(stderr, "Need to have the executable name and username\n");
    return BESE_BAD_ARG;
  }

  if(jc->num_hostnames>0){
    fprintf(stderr,"Candidate Hosts unsupported by PBS");
    return BESE_BAD_ARG;
  }

  strcpy(scriptname, "/tmp/besserver.XXXXXX");
  //Compose the script
  fd = mkstemp(scriptname);
  if (fd == -1) {
    perror("submitJob: mkstemp");
    fprintf(stderr, "Can not write PBS script file to disk\n");
    return BESE_SYS_ERR;
  }
  script = fdopen(fd, "w");
  if (script == NULL) {
    perror("submitJob: fdopen");
    fprintf(stderr, "Can not write PBS script file to disk\n");
    return BESE_SYS_ERR;
  }

  fprintf(script, "#!/bin/sh\n");

  if(jc->jobname!=NULL)
     fprintf(script,"#PBS -%s %s\n","N",jc->jobname);
  //List of resources for the job
  if (jc->icpu || jc->ipmem || jc->ivmem)
     fprintf(script,"#PBS -l ");
  /* FIXME add suport for range type to the new rm code
   * switch(jc->tcpuType)
  {
      case EXACT:fprintf(script,"nodes=%u",jc->tcpu.exact);break;
      case LOWERBOUND:fprintf(script,"nodes=%u",jc->tcpu.lowerBound.exclusive?
                         jc->tcpu.lowerBound.bound+1:jc->tcpu.lowerBound.bound);
                      break;
      case UPPERBOUND:fprintf(script,"nodes=%u",jc->tcpu.upperBound.exclusive?
                         jc->tcpu.upperBound.bound:jc->tcpu.upperBound.bound+1);break;
      case RANGE:fprintf(script,"nodes=%u",jc->tcpu.range.exclusiveLower?
                         jc->tcpu.range.lowerBound+1:jc->tcpu.range.lowerBound);break;
      default:fprintf(script,"nodes=1");
  }*/
  if (jc->icpu > 0 )
     fprintf(script,"nodes=%d", jc->icpu);
  
  if(jc->ipmem!=0)
     fprintf(script,",pmem=%u",jc->ipmem);
  if(jc->ivmem!=0)
     fprintf(script,",pvmem=%u",jc->ivmem);
  fprintf(script,"\n");
  //End of list of resources
  if(jc->wd==NULL)
     jc->wd="~";
  if(jc->username!=NULL)
     fprintf(script,"#PBS -%s %s\n","u",jc->username);
  fprintf(script,"#PBS -j eo\n");
  if(jc->jobproject!=NULL)
     fprintf(script,"#PBS -%s %s\n","A",jc->jobproject);
  //Store the submitting info in environment variables for later use
  char list[3000];
  if(jc->wd!=NULL)
     sprintf(list,"PBS_O_HOME=/home/%s,PBS_O_LOGNAME=%s,PBS_O_SHELL=/bin/bash,PBS_O_WORKDIR=%s",jc->username,jc->username,jc->wd);
  else
     sprintf(list,"PBS_O_HOME=/home/%s,PBS_O_LOGNAME=%s,PBS_O_SHELL=/bin/bash,PBS_O_WORKDIR=/home/%s",jc->username,jc->username,jc->username);
  addRequestInfo(soap, list, jc);
  
  struct envvar* variableList = jc->environment;
  for(; variableList != NULL; variableList = variableList->next)
  {
      strcat(list, ",");
      strcat(list, trim(soap, variableList->name));
      strcat(list, "=");
      strcat(list, trim(soap, variableList->val));
  }
  fprintf(script,"#PBS -v %s\n", trim(soap, list));

  fprintf(script,"cd %s\n", jc->wd);

  //Stage in files
  struct fileStage* currentFile=jc->files;
  while(currentFile!=NULL)
  {
     if(currentFile->source != NULL){
        //Obtain myproxy credentials and start proxy
        if((currentFile->credential != NULL) && 
	   (!strncmp(currentFile->source,"myproxy://",10))){
           fprintf(script,"%s %s %s %s\n", CRED_SCRIPT,
                    currentFile->source,
                    currentFile->credential->username,
		    currentFile->credential->password);
        }
        if((currentFile->credential != NULL) && 
	   (!strncmp(currentFile->target,"myproxy://",10))){
           fprintf(script,"%s %s %s %s\n", CRED_SCRIPT,
                    currentFile->target,
                    currentFile->credential->username,
		    currentFile->credential->password);
        }
	//Stage in files
	if(currentFile->credential == NULL)
           fprintf(script,"%s in %s %s\n", STAGE_SCRIPT, 
	        currentFile->source, currentFile->filename);
        else if((currentFile->credential->username != NULL) && 
		(currentFile->credential->password != NULL))
           fprintf(script,"%s in %s %s %s\n", STAGE_SCRIPT, currentFile->source,
                   currentFile->filename, currentFile->credential->username,
		   currentFile->credential->password);
     }
     currentFile=currentFile->next;
  }
  
  //The executable
  if (jc->executable)
    fprintf(script, "%s", jc->executable);
  for (i = 0; i < jc->num_args; i++) 
    fprintf(script, " %s", trim(soap, jc->args[i]));
  if(jc->input!=NULL)
     fprintf(script," < %s ", trim(soap, jc->input));
  if(jc->output!=NULL)
     fprintf(script," > %s ", trim(soap, jc->output));
  if(jc->error!=NULL)
     fprintf(script," 2> %s ", trim(soap, jc->error));
  fprintf(script, "\n");
  
  //Stage out files
  currentFile = jc->files;
  while(currentFile != NULL)
  {
     if(currentFile->target != NULL)
     {
        if(currentFile->credential == NULL)
           fprintf(script,"%s out %s %s\n", STAGE_SCRIPT, currentFile->target,
                currentFile->filename);
        else if((currentFile->credential->username != NULL) && 
		(currentFile->credential->password != NULL))
           fprintf(script,"%s out %s %s %s\n", STAGE_SCRIPT, currentFile->target,
                currentFile->filename, currentFile->credential->username, 
		currentFile->credential->password);
	if(currentFile->delete)
	  fprintf(script,"rm %s\n", currentFile->filename);
     }
     currentFile=currentFile->next;
  }

   //Remove files which DeleteOnTermination equals true
  currentFile = jc->files;
  while(currentFile != NULL){
     if(currentFile->delete){
        fprintf(script, "rm %s\n", currentFile->filename);
     }
     currentFile=currentFile->next;
  }

  fclose(script);
  close(fd);

  if (chmod(scriptname, S_IRWXU)) {
    perror("submitJob: chmod");
    return BESE_BACKEND;
  }

  jobid = runQsubScriptAsUser(scriptname,list,jc->username);
  
  //Delete the temporal script file
  unlink(scriptname);
  
  if(jobid <= 0){
    fprintf(stderr, "PBS error code");
    return BESE_BACKEND;
  }

  *return_jobid = (char*)soap_malloc(soap, sizeof(int)*8+1);
  sprintf(*return_jobid,"%d", jobid);

  return BESE_OK;
}

int
rm_getResourceList(struct soap *soap, struct rm_filter *filter, 
                   struct rm_resource **resourcelist, int *num_resources)
{
   int rc;
   FILE* fd;
   char outputFile[255];
   char resource[255];
   //Get the information about the current nodes
   strcpy(outputFile, "/tmp/nodeInfo.XXXXXX");

   rc = executeScriptSameUser(PBS_SCRIPT, "pbs.py", "factory", 
		"", outputFile);
   strcpy(resource, outputFile);
   strcat(resource, "RS");
   if (rc)
   {
	//There was an error
	unlink(resource);
	return rc;
   }
   else
   {
	int scriptReturn;
	fd = fopen(outputFile,"r");
	fscanf(fd, "%d",&scriptReturn);
	fclose(fd);
	unlink(outputFile);
	if(scriptReturn)
	{
	   unlink(resource);
	   return BESE_OTHER;
	}
   }

   rc = loadResourceFile(soap, resourcelist, resource, num_resources);
   unlink(resource);

   return rc;
}

int runQsubScriptAsUser(char *scriptname, char*variables,char *user)
{
    static char fname[] = "runBsubScriptAsUser";
    FILE *fp;
    pid_t pid;
    int pfd[2], jobid = 0;
    struct passwd *pw;
    char *arg0, buf[512];
    
    if (!scriptname || !user) {
        return -1;
    }
#ifdef ROOTACCESS    
    if ((pw = getpwnam(user)) == NULL) {
        fprintf(stderr, "%s: couldn't get user %s from passwd\n", fname, user);
        return -1;
    }
#endif

    if (pipe(pfd) < 0) {
        perror("runBsubScriptAsUser: pipe");
        return -1;
    }
    
    if ((pid = fork()) < 0) {
        perror("runBsubScriptAsUser: fork");
        return -1;
    }
    
    if (pid == 0) {
        /* child process */
        close(pfd[0]);
        if (pfd[1] != STDOUT_FILENO) {
            if (dup2(pfd[1], STDOUT_FILENO) != STDOUT_FILENO) {
                perror("runBsubScriptAsUser (child): dup2");
                _exit(1);
            }
            close(pfd[1]);
        }
#ifdef ROOTACCESS	
        if (seteuid(0)) {
            perror("runBsubScriptAsUser (child): setuid 0");
            _exit(1);
        }
        if (setgid(pw->pw_gid)) {
            perror("runBsubScriptAsUser (child): setgid");
            _exit(1);
        }
        if (setuid(pw->pw_uid)) {
            perror("runBsubScriptAsUser (child): setuid");
            _exit(1);
        }
#endif	
        execl(QSUB, QSUBEXEC, scriptname,"-v",variables,(char*)NULL);
        perror("runBsubScriptAsUser (child): execl");
	close(pfd[0]);
        _exit(1);
    }
            
    /* In the parent */
    close(pfd[1]);
    fp = fdopen(pfd[0], "r");
    if (fp == NULL) {
        perror("runBsubScriptAsUser: fdopen");
    }
    while (fgets(buf, 512, fp)) {
        jobid=atoi(buf);//sscanf(buf, "%d@%s\n", &jobid,server);
    }
    fclose(fp);
    close(pfd[0]);
    
    if (waitpid(pid, NULL, 0) < 0) {
        perror("runBsubScriptAsUser: waitpid");
        return -1;
    }
    
    return jobid;
}

/**
 * Gets the error number code of the last error.
 *
 * It provides access to local module variable errorno. See errors.h for a list
 * of error codes.
 * @return the error number code
*/
int getErrNo(){
   return errorno;
}

