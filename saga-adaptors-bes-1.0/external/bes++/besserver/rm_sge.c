/* ----------------------------------------------------------------
 * rm_pbs.c
 *
 *      SGE server implementation of the OGSA Basic Execution Services
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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

//! Complete route of the python script to call, defined in Makefile
#define SCRIPT_SGE "./scripts/"
//! Name of the python script to call
#define SCRIPT_SGE_EXEC  "sge.py"

//! Maximum length of the environment variables string
#define MAXVAR_BUFFER 4096

//GLOBAL VARIABLES

//! Python script for SGE communication (absolute route)
char* script_sge;
//! Name of the Python script for SGE communication
char* script_sge_exec;
//! information about the cluster (factory)
struct clusterInfo* clusterInfo;

/**
 * Loads the cluster info and sets the location of the SGE python script.
 * @param soap is used to allocate memory deallocatable by the gSOAP library
 * @param serverName is not used for the SGE implementation
 * @return 0 if connection established, 1 if error
 */
int rm_initialize(struct soap* soap, char* serverName)
{
   char* scriptLocation = (char*) malloc(strlen(SCRIPT_SGE) + 
		   strlen(SCRIPT_SGE_EXEC) + 5);
   memset(scriptLocation, 0 ,sizeof(strlen(SCRIPT_SGE) + 
		   strlen(SCRIPT_SGE_EXEC) + 5));

   scriptLocation = strcat(scriptLocation,SCRIPT_SGE);
   scriptLocation = strcat(scriptLocation,"/");
   scriptLocation = strcat(scriptLocation,SCRIPT_SGE_EXEC);
   script_sge = scriptLocation;
   script_sge_exec = SCRIPT_SGE_EXEC;

   return BESE_OK;
}

/**
 * Prints an error message to stderr
 * @param message string to print
 */
void printError(char* message)
{
   fprintf(stderr,message);
}

/**
 * Returns 0 as the code of the last error that occurred since this does
 * not apply to the SGE implementation
 * @return 0
 */
int getErrNo()
{
   return 0;
}

/**
 * Terminates a job. 
 * @param jobid is the PID assigned by the queue
 * @param user is a string with the account of the user who requested the 
 * job deletion
 * @return 0 if correct, non-zero if error
 */
int rm_terminateJob(struct soap*s, char* jobid, char* user)
{
   char outputFile[256];   
   FILE* fd;
   int rc;
   int deleted;

   strcpy(outputFile, "/tmp/besserver.XXXXXX");

   rc = executeScript(user, script_sge, script_sge_exec, "delete", 
		jobid, outputFile);
   if (rc)
   {
	//There was an error deleting the job
        unlink(outputFile);
	return rc;
   }
   else {
	fd = fopen(outputFile, "r");
   	fscanf(fd, "%d", &deleted);
        unlink(outputFile);
	if (deleted)
	   return BESE_BACKEND;
	else
	   return BESE_OK;
   }
}

/**
 * Gets the status of the job. 
 *
 * It maps the different states of SGE jobs to
 * pending and running. It does not make a difference between finished, 
 * cancelled, terminated and unknown jobs since SGE does not store this info.
 * @param jobid is the PID assigned by the queue
 * @param user is a string with the account of the user who requested the 
 * job status
 * @param jobStatus is an output parameter with the status of the job -NULL
 * if error-
 * @return 0 if correct, non-zero if error
 */

int rm_getJobStatus(struct soap*s, char* jobid, char* user, struct bes__ActivityStatusType** jobStatus)
{
   struct bes__ActivityStatusType *activityStatus;
   char outputFile[256];   
   FILE* fd;
   int rc;
   char status;

   if (!jobid || !jobStatus) {
      return BESE_BAD_ARG;
   }

   activityStatus = (struct bes__ActivityStatusType*)soap_malloc(s, sizeof(struct bes__ActivityStatusType));
   if (activityStatus == NULL) {
      return BESE_MEM_ALLOC;
   }
   memset(activityStatus, 0, sizeof(struct bes__ActivityStatusType));

   strcpy(outputFile, "/tmp/besserver.XXXXXX");

   rc = executeScript(user, script_sge, script_sge_exec, "status", 
		jobid, outputFile);
   if (rc)
   {
	//There was an error
	*jobStatus = NULL; 
	unlink(outputFile);
	return BESE_BACKEND;
   }
   else {
	fd = fopen(outputFile,"r");
   	fscanf(fd,"%c",&status);
	unlink(outputFile);
	switch(status){
	   case 'u': return BESE_NO_ACTIVITY;
	   case 'd': activityStatus->state = Finished; break;
	   case 't': activityStatus->state = Pending; break;
	   case 'r': activityStatus->state = Running; break;
	   case 'R': activityStatus->state = Running; break;
	   case 's': activityStatus->state = Pending; break;
	   case 'S': activityStatus->state = Pending; break;
	   case 'T': activityStatus->state = Pending; break;
	   case 'w': activityStatus->state = Pending; break;
	   case 'h': activityStatus->state = Pending; break;
           default: return BESE_BACKEND;
	}
 	*jobStatus = activityStatus;
	return BESE_OK;
   }
}

/**
 * Gets the information about a job. 
 *
 * @param jobid is the PID assigned by the queue
 * @param soap
 * @param jobInfo is an output parameter which contains the information
 * about the job submission request and the job state (NULL if error).
 * @return 0 if correct, non-zero if error
 */
int rm_getJobInfo(struct soap* soap, char* jobid, char*user, struct jobcard** jobInfo )
{
   char outputFile[256];
   char *list;
   int size;
   char character;
   int rc;
   FILE* fd;

   *jobInfo = NULL;
   strcpy(outputFile, "/tmp/besserver.XXXXXX");

   rc = executeScriptSameUser( script_sge, script_sge_exec, "activity", 
		jobid, outputFile);
   if (rc)
   {
	//There was an error
	*jobInfo = NULL; 
	unlink(outputFile);
	return rc;
   }
   else {
	fd = fopen(outputFile, "r");
	character = fgetc(fd);
	size = 0;
        while(character != EOF){
		size++;
		character = fgetc(fd);
	}
	fseek (fd, 0 , SEEK_SET );
	list = (char*) soap_malloc( soap, sizeof(char)*size + 5);
	memset(list, 0 , sizeof(char)*size + 5);
   	fscanf(fd, "%s", list);
	fclose(fd);
	unlink(outputFile);
	if ((list==NULL) && (!strcmp(list,"list=NULL")))
		return BESE_NO_ACTIVITY;
   }

   *jobInfo = (struct jobcard*) soap_malloc(soap, sizeof(struct jobcard));
   fillJobStatusDefaults(*jobInfo);
   getRequestInfo(soap, list, *jobInfo);
   return BESE_OK;
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
 * @return a list of job ids
 */
int rm_getJobList(struct soap* s, struct rm_filter* filters,
		struct rm_job** joblist, int* numjobs)
{
   struct rm_job* jobList;
   char filename[128];
   char command[256];
   struct rm_filter* filter;
   int jobid;
   int scan;
   char ignore[128];
   int numJobs = 0;

   strcpy(filename, "/tmp/queueStatus.XXXXXX");
   int fd = mkstemp(filename);
   if (fd == -1) {
      return BESE_SYS_ERR;
   }
   close(fd);

   sprintf(command, "qstat -f | grep '\\(Job Id\\)\\|\\(Job_Owner\\)\\|\\(qtime\\)\\|\\(job_state\\)' > %s ", filename);
   //printf(command); 

   system(command);   

   FILE* in = fopen(filename, "r");
   if (in == NULL)
      return BESE_SYS_ERR;

   scan = fscanf(in, "Job Id: %d.%s\n", &jobid, ignore);
   filter = filters;
   while (scan != EOF) {
      char state[64];
      char user[32];
      char week[16],month[16];
      int day, hour, minute, second, year;
      int skip = 0, found = 0, filterPresent = 0;
      fscanf(in, "\tJob_Owner = %s\n", user);
      fscanf(in, "job_state = %s\n", state);
      fscanf(in, "qtime = %s %s %d %d:%d:%d %d\n", week, month, &day, &hour,
		      &minute, &second, &year);

      //Chek for the users if we have to
      filter = filters;
      found = 0;
      filterPresent = 0;
      while ((filter != NULL) && (!found)){
         if (filter->user != NULL){
            filterPresent = 1;
            if (!strncmp(filter->user, user, strlen(filter->user)))
               found = 1;
         }
         filter = filter->next;
      }
      if (!found && filterPresent)
         skip = 1;

      //Chek for the state
      filter = filters;
      found = 0;
      filterPresent = 0;
      while ((filter != NULL) && (!skip) && (!found)){
         if (filter->state != NULL){
            filterPresent = 1;
            if (!strcmp(filter->state, "Pending"))
               if (!strcmp(state, "Q"))
                  found = 1;
            if (!strcmp(filter->state, "Running"))
               if(!strcmp(state, "R"))
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
      while((filter!=NULL)&&(!skip)&&(!found)){
         if((filter->startRange>0)&&(filter->endRange>0)){
            filterPresent=1;
            if((jobid>=filter->startRange) &&(jobid<=filter->endRange))
               found=1;
         }
         filter=filter->next;
      }
      if(!found&&filterPresent)
         skip=1;

      //Check for the time interval
      filter=filters;
      found=0;
      filterPresent=0;
      while((filter!=NULL)&&(skip==0)&&(!found)){
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
         struct rm_job* newJob=(struct rm_job *)soap_malloc(s, 
			 sizeof(struct rm_job));
	 char jobidStr[sizeof(int)*8+1];
	 if (!newJob)
		 return BESE_MEM_ALLOC;
	 memset(newJob, 0, sizeof(struct rm_job));
	 sprintf(jobidStr,"%d",jobid);
         newJob->jobid = soap_strdup(s, jobidStr);
         newJob->next = jobList;
         jobList = newJob;
	 numJobs++;
      }

      scan=fscanf(in,"Job Id: %d.%s\n",&jobid,ignore);
   }
   fclose(in);
   unlink(filename);

   *joblist = jobList;
   *numjobs = numJobs;

   return BESE_OK;
}

char * readLineFromFile(struct soap* s, FILE* fd)
{
   char line[256];
   int rc = fscanf(fd,"%s\n",line);
   if (rc == 0)
   {
      fclose(fd);
      return NULL;
   }
   return soap_strdup(s, line);
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

   rc = executeScriptSameUser(SCRIPT_SGE, SCRIPT_SGE_EXEC, "factory", 
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


/**
 * Gets the factory attributes. 
 *
 * This function uses @see loadResourceFile 
 * and also queries the SGE queue.
 * @param soap is needed to allocate memory that can be deallocated by the 
 * gsoap library after.
 * @return a struct of type clusterInfo with the information needed for the
 * factory attributes document
 */
int rm_getClusterInfo(struct soap* s, struct rm_clusterInfo** clusterInf
		/*,int compactResources*/)
{
   struct rm_clusterInfo* clusterInfo;
   struct rm_resource* resourcesInfo;
   char outputFile[256];  
   char resourceFile[256]; 
   FILE* fd;
   int rc;
   long int jobId;
   int num_resources;

   if (!clusterInf) {
      return BESE_BAD_ARG;
   }
   clusterInfo = (struct rm_clusterInfo*) soap_malloc(s,
		   sizeof(struct rm_clusterInfo));
   if (clusterInfo == NULL)
      return BESE_MEM_ALLOC;
   memset(clusterInfo, 0, sizeof(struct rm_clusterInfo));

   memset(resourceFile,0, sizeof(resourceFile));
   memset(outputFile,0, sizeof(outputFile));
   strcpy(outputFile, "/tmp/besserver.XXXXXX");
   strcat(resourceFile, outputFile);
   strcat(resourceFile, "R");

   /*if (!compact_resources)
      rc = executeScriptSameUser(script_sge, script_sge_exec, "factory", 
		resourceFile, outputFile);
   else*/
      rc = executeScriptSameUser(script_sge, script_sge_exec, "factory", 
		resourceFile, outputFile);
   if (rc)
   {
	//There was an error
	unlink(outputFile);
	return BESE_BACKEND;
   }
   else
   {
	int scriptReturn;
	fd = fopen(outputFile,"r");
	fscanf(fd, "%d",&scriptReturn);
	fclose(fd);
	unlink(outputFile);
	if (scriptReturn != 0)
           return BESE_BACKEND;
   }

   rc = loadResourceFile(s, &resourcesInfo, resourceFile, &num_resources);
   unlink(resourceFile);

   if(rc)
      return BESE_BACKEND;

   //Load the file with the SGE queue properties
   fd = fopen(outputFile, "r");
  
   if(!strcmp(readLineFromFile(s, fd), "0"))
      clusterInfo->IsAcceptingNewActivities = false_;
   else
      clusterInfo->IsAcceptingNewActivities = true_;

   clusterInfo->CommonName = readLineFromFile(s, fd);
   clusterInfo->LongDescription = readLineFromFile(s, fd);
   clusterInfo->LocalResourceManagerType = readLineFromFile(s, fd);
   fclose(fd);
   unlink(outputFile);

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
int rm_submitJob( struct soap* soap, struct jobcard *jc, 
		char* user, char** return_jobid)
{
  static char fname[] = "submitJob";
  char scriptname[64];
  int fd, i, rc, jobid = 0, rr = 0;
  FILE *script;
  FILE *result;

  fprintf(stderr, "In submitJob...\n");

  if (!jc || !jc->executable || !jc->username) {
    fprintf(stderr, "Need to have the executable name and username\n");
    return BESE_BAD_ARG;
  }

  //TO-DO: implement candidate hosts
  if(jc->num_hostnames>0){
    fprintf(stderr, "Candidate Hosts unsupported by SGE\n");
    return BESE_BAD_ARG;
  }

  /*if(jc->osname){
     int osNotFound=1;
     OSListNode* os=clusterInfo->osList;
     while(os->OperatingSystem!=NULL){
        if(!strcmp(os->OperatingSystem,jc->osname)){
           osNotFound=0;
           break;
        }
        os=os->next;
     }
     if(osNotFound){
       *message="OS not present in the cluster";
       return BESE_BAD_ARG;
     }
  }

  if(jc->cpuarch){
     int archNotFound=1;
     ArchListNode* arch=clusterInfo->archList;
     while(arch->CPUArchitecture!=NULL){
        if(!strcmp(arch->CPUArchitecture,jc->cpuarch)){
           archNotFound=0;
           break;
        }
        arch=arch->next;
     }
     if(archNotFound){
       *message="Architecture not present in the cluster";
       return BESE_BAD_ARG;
     }
  }*/

  strcpy(scriptname, "/tmp/besserver.XXXXXX");
  //Compose the script
  fd = mkstemp(scriptname);
  if (fd == -1) {
    perror("submitJob: mkstemp");
    fprintf(stderr, "Can not write SGE script file to disk\n");
    return BESE_SYS_ERR;
  }
  script = fdopen(fd, "w");
  if (script == NULL) {
    perror("submitJob: fdopen");
    fprintf(stderr, "Can not write SGE script file to disk\n");
    return BESE_SYS_ERR;
  }

  fprintf(script, "#!/bin/sh\n");

  if(jc->jobname!=NULL)
     fprintf(script,"#$ -N %s\n", jc->jobname);
  //List of nodes for the job
  fprintf(script,"#$ -pe * ");
  /*switch(jc->tcpuType)
  {
      case EXACT: fprintf(script,"%u",jc->tcpu.exact);break;
      case LOWERBOUND: fprintf(script, "%u", 
	jc->tcpu.lowerBound.exclusive?jc->tcpu.lowerBound.bound+1:
	jc->tcpu.lowerBound.bound);break;
      case UPPERBOUND: fprintf(script, "1-%u",
	jc->tcpu.upperBound.exclusive?jc->tcpu.upperBound.bound:
	jc->tcpu.upperBound.bound+1);break;
      case RANGE: fprintf(script, "%u-%u",
	jc->tcpu.range.exclusiveLower?jc->tcpu.range.lowerBound+1:
	jc->tcpu.range.lowerBound,
	jc->tcpu.range.exclusiveUpper?jc->tcpu.range.upperBound+1:
	jc->tcpu.range.upperBound);break;
      default:fprintf(script,"1");
  }*/
  fprintf(script,"%u",jc->icpu);

  fprintf(script, "\n");
  //List of resources for the job
  if(jc->ipmem!=0)
     fprintf(script,"#$ -l s_vmem=%u\n",jc->ipmem);
  if(jc->ivmem!=0)
     fprintf(script,"#$ -l h_vmem=%u\n",jc->ivmem);
  //End of list of resources
  if(jc->wd==NULL)
     jc->wd="~";
  fprintf(script,"#$ -j y\n");
  if(jc->jobproject!=NULL)
     fprintf(script,"#$ -%s %s\n","A",jc->jobproject);

  //Store the submitting info in environment variables for later use
  char list[MAXVAR_BUFFER];
  if(jc->wd!=NULL)
     sprintf(list,"SGE_O_HOME=/home/%s,SGE_O_LOGNAME=%s,SGE_O_SHELL=/bin/bash,SGE_O_WORKDIR=%s",jc->username,jc->username,jc->wd);
  else
     sprintf(list,"SGE_O_HOME=/home/%s,SGE_O_LOGNAME=%s,SGE_O_SHELL=/bin/bash,SGE_O_WORKDIR=/home/%s",jc->username,jc->username,jc->username);
  addRequestInfo(soap, list, jc);
  
  struct envvar* variableList = jc->environment;
  int envVariablesSize = 0;
  for(;variableList != NULL; variableList = variableList->next)
  {
      char *variable;
      envVariablesSize+= strlen(variableList->name) + 
	      strlen(variableList->val) + 5;
      if (envVariablesSize > MAXVAR_BUFFER)
	      break;
      variable = (char*) soap_malloc(soap, strlen(variableList->name)
		      + strlen(variableList->val) + 5);
      sprintf(variable, ",%s=%s", variableList->name, variableList->val);
      strcat(list, variable);
      free(variable);
  }
  fprintf(script, "#$ -v %s\n", list);

  fprintf(script, "cd %s\n", jc->wd);

  //Obtain myproxy credentials ans start proxy
  /*struct cred* credentials = jc->credentials;
  while(credentials != NULL)
  {
     if(!strncmp(credentials->endpoint, "myproxy://", 10))
     {
         fprintf(script, "%s %s %s %s\n", CREDSCRIPT, credentials->endpoint,
          credentials->username, credentials->password);
     }
     credentials = credentials->next;
  }

  //Stage in files
  struct fileStage* currentFile=jc->files;
  while(currentFile != NULL)
  {
     if(currentFile->source != NULL)
     {
        if(currentFile->userSource == NULL)
           fprintf(script,"%s in %s %s\n", STAGESCRIPT, currentFile->source,
                currentFile->filename, currentFile->userSource,
		currentFile->passwordSource);
        else if(currentFile->passwordSource==NULL)
           fprintf(script, "%s in %s %s %s\n", STAGESCRIPT, 
			   currentFile->source, currentFile->filename,
			   currentFile->userSource);
        else
           fprintf(script, "%s in %s %s %s %s\n", STAGESCRIPT,
			   currentFile->source, currentFile->filename,
			   currentFile->userSource, 
			   currentFile->passwordSource);
     }
     currentFile = currentFile->next;
  }*/
  
  //The executable
  if (jc->executable)
    fprintf(script, "%s", jc->executable);
  for (i = 0; i < jc->num_args; i++) 
    fprintf(script, " %s", jc->args[i]);
  if(jc->input!=NULL)
     fprintf(script," < %s ",jc->input);
  if(jc->output!=NULL)
     fprintf(script," > %s ",jc->output);
  if(jc->error!=NULL)
     fprintf(script," 2> %s ",jc->error);
  fprintf(script, "\n");
  
  //Stage out files
  /*currentFile = jc->files;
  while(currentFile!=NULL)
  {
     if(currentFile->target != NULL)
     {
        if(currentFile->userTarget == NULL)
           fprintf(script, "%s out %s %s\n", STAGESCRIPT, currentFile->target,
                currentFile->filename, currentFile->userTarget,
		currentFile->passwordTarget);
        else if(currentFile->passwordTarget==NULL)
           fprintf(script, "%s out %s %s %s\n", STAGESCRIPT,
		currentFile->target, currentFile->filename,
		currentFile->userTarget, currentFile->passwordTarget);
        else
           fprintf(script, "%s out %s %s %s %s\n", STAGESCRIPT,
		currentFile->target, currentFile->filename,
		currentFile->userTarget, currentFile->passwordTarget);
     }
     currentFile = currentFile->next;
  }

   //Remove files which DeleteOnTermination equals true
  currentFile=jc->files;
  while(currentFile!=NULL)
  {
     if(currentFile->deleteFile){
        fprintf(script,"rm %s\n",currentFile->filename);
     }
     currentFile=currentFile->next;
  }*/

  fclose(script);

  if (chmod(scriptname, S_IRWXU)) {
    perror("submitJob: chmod");
    fprintf(stderr, "Can not write SGE script file to disk\n");
    return BESE_SYS_ERR;
  }


  char outputFile[256];
  strcpy(outputFile, "/tmp/besserver.XXXXXX");

  int submission = executeScriptSameUser( script_sge, script_sge_exec, 
		  "submit", scriptname, outputFile);
  //Delete the temporal script file
  unlink(scriptname);
  
  if(submission!=0){
    fprintf(stderr, "SGE error code");
    unlink(outputFile);
    return BESE_SYS_ERR;
  }

  result = fopen(outputFile, "r");
  int pid=0;
  fscanf(result,"%d",&pid);
  fclose(result);
  unlink(outputFile);

  if(pid<=0){
    fprintf(stderr, "SGE error code");
    return BESE_SYS_ERR;
  }

  *return_jobid = (char*)soap_malloc(soap, sizeof(int)*8+1);
  sprintf(*return_jobid,"%d", jobid);

  return BESE_OK;
}

