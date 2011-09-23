/* ----------------------------------------------------------------
 * bescluster.c
 *
 *      BES++ server implementation of the OGSA Basic Execution Services
 *      Common functions for all types of clusters
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
#include "rm_util.h"
#include "rm.h"
#include "faults.h"
#include "job.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_OS 1
#define N_ARCH 2
char *OPERATING_SYSTEM_NAME[] = {"LINUX"};
char *OPERATING_SYSTEM_VERSION[] = {"2.6.9"};
char *CPU_ARCHITECTURE[] = {"x86", "x86_64"};


/**
 * Executes the python script by forking and calling execl. The python
 * script sge.py calls the commands qsub, qstat, qdel, etc. and writes
 * the result to a text file that is later readed by this C program.
 * Before this call the process changes its UID to the UID of the 
 * account specified.
 * @param user String with the user account to execute the script under
 * @param script Absolute route to the script
 * @param scriptExec name of the executable (arg[0])
 * @param action First parameter for sge.py, see sge.py for details
 * @param parameter Second parameter for sge.py, see sge.py for details
 * @param outputFile Third parameter for sge.py, this is a temporal file
 * created to pass the information between the python script and this
 * C program
 * @return 0 if everything is OK, NOTAUTHORIZEDFAULT otherwise
 */
int executeScript(char * user, char * script, char * scriptExec, 
		char * action, char * parameter, char * outputFile)
{
   int fd;
   pid_t pid;
   struct passwd *pw;
   char *arg0, buf[512];

   //Make a temporal file for the script output
   fd = mkstemp(outputFile);
   if (fd == -1) {
    perror("executeScript: mkstemp");
    return BESE_SYS_ERR;
  }
#ifdef ROOTACCESS   
   if (!user) {
      return BESE_SYS_ERR;
   }

   if ((pw = getpwnam(user)) == NULL) {
      fprintf(stderr, "executeScript: couldn't get user %s from passwd\n", user);
      return BESE_SYS_ERR;
   }
#endif

   if ((pid = fork()) < 0) {
      fprintf(stderr,"executeScript: fork");
      return BESE_SYS_ERR;
   }

   if (pid == 0) {
#ifdef ROOTACCESS
     // child process 
      if (seteuid(0)) {
         perror("executeScript (child): setuid 0");
         _exit(1);
      }
      if (setgid(pw->pw_gid)) {
         perror("executeScript (child): setgid");
         _exit(1);
      }
      if (setuid(pw->pw_uid)) {
         perror("executeScript (child): setuid");
         _exit(1);
      }
#endif
      execlp(script, scriptExec, action, parameter, outputFile, (char*) NULL);
      perror("executeScript (child): execl");
      _exit(1);
   }
   else{
      /* parent process */   
      if (waitpid(pid, NULL, 0) < 0) {
          perror("executeScript: waitpid");
          return BESE_SYS_ERR;
      }
   }

   close(fd);
   return BESE_OK;
}

/**
 * Executes the python script by forking and calling execl. The python
 * script sge.py calls the commands qsub, qstat, qdel, etc. and writes
 * the result to a text file that is later readed by this C program. It
 * does not change the UID, so this function should be used ONLY for 
 * obtaining information that is widely available (QSTAT)
 *
 * @param script Absolute route to the script
 * @param scriptExec name of the executable (arg[0])
 * @param action First parameter for sge.py, see sge.py for details
 * @param parameter Second parameter for sge.py, see sge.py for details
 * @param outputFile Third parameter for sge.py, this is a temporal file
 * created to pass the information between the python script and this
 * C program
 * @return 0 if everything is OK, NOTAUTHORIZEDFAULT otherwise
 */
int executeScriptSameUser(char * script, char * scriptExec, char * action, 
		char * parameter, char * outputFile)
{
   int fd;
   pid_t pid;
   struct passwd *pw;
   char *arg0, buf[512];

   //Make a temporal file for the script output
   fd = mkstemp(outputFile);
   if (fd == -1) {
    perror("executeScript: mkstemp");
    return BESE_SYS_ERR;
  }
   

   if ((pid = fork()) < 0) {
      fprintf(stderr,"executeScript: fork");
      return BESE_SYS_ERR;
   }

   if (pid == 0) {
      /* child process */
      execl(script, scriptExec, action, parameter, outputFile, (char*) NULL);
      perror("executeScript (child): execl");
      _exit(1);
   }
   else{
      /* parent process */   
      if (waitpid(pid, NULL, 0) < 0) {
          perror("executeScript: waitpid");
          return -1;
      }
   }

   close(fd);

   return 0;
}

/**
 * Assigns an environment variable to a field in the jobcard struct. 
 *
 * The queue stores the original job parameters of the request in 
 * environment variables like HPCP_XXXXX. This function gets a list of 
 * environment variables and fills the struct jobcard.
 * @param list is a list of environment variables "VAR1="var1",VAR2="var2""
 * @param jobInfo is the struct which is filled with the information
 * @param soap is used to allocate memory deallocatable by the gSOAP library
 * about the job
 */
void getRequestInfo(struct soap* soap, char* list, struct jobcard* jobInfo)
{
     char *variable;
     jobInfo->args = (char**) soap_malloc(soap, sizeof(char**)*50);
     for ( variable = strtok(list,","); variable != NULL;
           variable = strtok(NULL, ",") )
     {
         char name[200];
         char *equal = strchr(variable, '=');
         char varValue[200];
         memset(name, 0, 200);
         memset(varValue, 0, 200);
         strncpy(name, variable, equal-variable);
         strcpy(varValue, equal + 1);
         
         if(!strcmp(name, "HPCP_JOB_NAME"))
         {
	    jobInfo->jobname = soap_strdup(soap, varValue);
         }
         else if(!strcmp(name, "HPCP_JOB_PROJECT"))
         {
            jobInfo->jobproject = soap_strdup(soap, varValue);
         }
         else if(!strcmp(name, "HPCP_EXCLUSIVE"))
         {
            jobInfo->exclusive = atoi(varValue);
         }
         else if(!strcmp(name, "HPCP_OSNAME"))
         {
	    jobInfo->osname = soap_strdup(soap, varValue);
         }
         else if(!strcmp(name, "HPCP_CPU_ARCH"))
         {
	    jobInfo->cpuarch = soap_strdup(soap, varValue);
         }
         else if(!strcmp(name, "HPCP_TOTAL_CPU_COUNT"))
         {
            jobInfo->icpu = atoi(varValue);
         }
         else if(!strcmp(name, "HPCP_APP_EXECUTABLE"))
         {
	    jobInfo->executable = soap_strdup(soap, varValue);
         }
         else if(!strncmp(name, "HPCP_APP_ARG",12))
         {
            jobInfo->args[jobInfo->num_args] = soap_strdup(soap, varValue);
	    jobInfo->num_args += 1;
         }
         else if(!strcmp(name, "HPCP_APP_INPUT"))
         {
	    jobInfo->input = soap_strdup(soap, varValue);
         }
         else if(!strcmp(name, "HPCP_APP_OUTPUT"))
         {
	    jobInfo->output = soap_strdup(soap, varValue);
         }
         else if(!strcmp(name, "HPCP_APP_ERROR"))
         {
	    jobInfo->error = soap_strdup(soap, varValue);
	 }
         else if(!strcmp(name, "HPCP_APP_WD"))
         {
	    jobInfo->wd = soap_strdup(soap, varValue);
         }
         else if(!strcmp(name, "HPCP_APP_USERNAME"))
         {
	    jobInfo->username = soap_strdup(soap, varValue);
         }
         else
         {
            struct envvar* newEnvVar=(struct envvar*) soap_malloc(soap, 
			    sizeof(struct envvar));
            memset(newEnvVar, 0, sizeof(struct envvar));
            newEnvVar->name = soap_strdup(soap,name);
            newEnvVar->val = soap_strdup(soap,varValue);
            newEnvVar->next = jobInfo->environment;
            jobInfo->environment = newEnvVar;
         }
     }
}

/**
 * Loads the cluster resource information file into the module variable 
 * clusterInfo.
 *
 * 
 * @return 0 is correct, 1 otherwise
 */
int loadResourceFile(struct soap * soap,  
		struct rm_resource** resources, 
		char* resourceFilename,
		int* num_resources)
{
   char resourceName[128];
   char state[128];
   unsigned int CPUCount;
   unsigned long CPUSpeed;
   unsigned long PhysicalMemory;
   unsigned long VirtualMemory;
   int eof;
   int count;
   struct rm_resource* rm_resource;
   struct rm_resource* new_resource;
   FILE* resourceFile;

   rm_resource = new_resource = NULL;
   count = 0;
   resourceFile = fopen(resourceFilename, "r");

   if (resourceFile == NULL){
      fprintf(stderr,"Error with cluster info file:%s\n", resourceFilename);
      return BESE_OTHER;
   }

   do{
     eof = fscanf(resourceFile, "%s %lu %lu %s", resourceName,
                &PhysicalMemory, &CPUCount, state);
     if (eof != EOF)
     {
	 new_resource = (struct rm_resource*) soap_malloc(soap, 
			      sizeof (struct rm_resource));
	 if (new_resource == NULL)
		 return BESE_MEM_ALLOC;
	 memset(new_resource, 0, sizeof(rm_resource));

	 new_resource->CPUCount = (double*) soap_malloc(soap, 
			 sizeof(double));
         new_resource->CPUSpeed = (double*) soap_malloc(soap, 
			 sizeof(double));
         new_resource->PhysicalMemory = (double*) soap_malloc(soap, 
			 sizeof(double));
	 new_resource->VirtualMemory = (double*) soap_malloc(soap, 
			 sizeof(double));
         new_resource->ResourceName = (char*) soap_malloc(soap, 
			 strlen(resourceName) + 1);
	 if (!new_resource->CPUCount || !new_resource->PhysicalMemory ||
			 !new_resource->ResourceName)
		 return BESE_MEM_ALLOC;
	 *(new_resource->CPUCount) = CPUCount;
	 *(new_resource->CPUSpeed) = 0;
         *(new_resource->VirtualMemory) = PhysicalMemory;
	 *(new_resource->PhysicalMemory) = PhysicalMemory;
         new_resource->ResourceName = soap_strdup(soap, resourceName);
	 new_resource->next = rm_resource;
	 rm_resource = new_resource;
	 count ++;
     }
   }while (eof != EOF);
   fclose(resourceFile);

   *resources = rm_resource;
   *num_resources = count;

   return BESE_OK;
}

/**
 * Fills a jobcard struct with 0s, 1s and empty strings.
 *
 * This function is used to avoid segmentation faults if we access a field in 
 * the struct that has not been filled by @see getRequestInfo.
 * @param jobInfo is the -allocated- struct to fill
 */
void fillJobStatusDefaults(struct jobcard*jobInfo)
{
   jobInfo->jobname="";
   jobInfo->jobproject="";
   jobInfo->num_hostnames=0;
   jobInfo->hostnames=NULL;
   jobInfo->exclusive=0;
   jobInfo->osname="";
   jobInfo->osver="";
   jobInfo->cpuarch="";
   jobInfo->executable="";
   jobInfo->num_args=0;
   jobInfo->args=NULL;
   jobInfo->input="";
   jobInfo->output="";
   jobInfo->error="";
   jobInfo->wd="$HOME";
   jobInfo->environment=NULL;

   jobInfo->username="";
  
}

/**
 * Adds an environment variable for every field in the struct jobcard.
 *
 * The queue stores the original job parameters of the request in 
 * environment variables like HPCP_XXXXX. This function gets every property
 * used in the request and adds the corresponding environment variable to
 * the job submission request.
 * @param list is the environment variable list generated, separated by commas
 * @param jobInfo is the struct with the request info
 * @param soap is used to allocate memory deallocatable by the gsoap library
 */
void addRequestInfo(struct soap* soap, char* list, struct jobcard* jobInfo)
{
   char variable[200];
   int i;

   if(jobInfo->jobname!=NULL)
   {
      sprintf(variable,",HPCP_JOB_NAME=%s",jobInfo->jobname);
      strcat(list,variable);
   }
   if(jobInfo->jobproject!=NULL)
   {
      sprintf(variable,",HPCP_JOB_PROJECT=%s",jobInfo->jobproject) ;
      strcat(list,variable);
   }
   for (i=0;i<jobInfo->num_hostnames;i++)
   {
      sprintf(variable,",HPCP_HOSTNAME%u=%s",i,jobInfo->hostnames[i]) ;
      strcat(list,variable);
   }
   if(jobInfo->exclusive)
   {
      sprintf(variable,",HPCP_EXCLUSIVE=%u",jobInfo->exclusive) ;
      strcat(list,variable);
   }
   if(jobInfo->osname!=NULL)
   {
      sprintf(variable,",HPCP_OSNAME=%s",jobInfo->osname) ;
      strcat(list,variable);
   }
   if(jobInfo->cpuarch!=NULL)
   {
      sprintf(variable,",HPCP_CPU_ARCH=%s",jobInfo->cpuarch) ;
      strcat(list,variable);
   }
   if(jobInfo->icpu > 0)
   {
      sprintf(variable,",HPCP_TOTAL_CPU_COUNT=%u",jobInfo->icpu) ;
      strcat(list,variable);
   }
   /*if(jobInfo->tcpuType==LOWERBOUND)
   {
      if(jobInfo->tcpu.lowerBound.exclusive)
         sprintf(variable,",HPCP_TOTAL_CPU_COUNT_LB=%u",jobInfo->tcpu.lowerBound.bound+1);
      else
         sprintf(variable,",HPCP_TOTAL_CPU_COUNT_LB=%u",jobInfo->tcpu.lowerBound.bound);
      strcat(list,variable);
   }
   if(jobInfo->tcpuType==UPPERBOUND)
   {
      if(jobInfo->tcpu.upperBound.exclusive)
         sprintf(variable,",HPCP_TOTAL_CPU_COUNT_UB=%u",jobInfo->tcpu.upperBound.bound);
      else
         sprintf(variable,",HPCP_TOTAL_CPU_COUNT_UB=%u",jobInfo->tcpu.upperBound.bound+1);
      strcat(list,variable);
   }
   if(jobInfo->tcpuType==RANGE)
   {
      if(jobInfo->tcpu.range.exclusiveUpper)
         sprintf(variable,",HPCP_TOTAL_CPU_COUNT_UB=%u",jobInfo->tcpu.range.upperBound);
      else
         sprintf(variable,",HPCP_TOTAL_CPU_COUNT_UB=%u",jobInfo->tcpu.range.upperBound+1);
      if(jobInfo->tcpu.range.exclusiveLower)
         sprintf(variable,",HPCP_TOTAL_CPU_COUNT_LB=%u",jobInfo->tcpu.range.lowerBound+1);
      else
         sprintf(variable,",HPCP_TOTAL_CPU_COUNT_LB=%u",jobInfo->tcpu.range.lowerBound);

      strcat(list,variable);
   }*/

   if(jobInfo->executable!=NULL)
   {
      sprintf(variable,",HPCP_APP_EXECUTABLE=%s",jobInfo->executable) ;
      strcat(list,variable);
   }
   for (i=0;i<jobInfo->num_args;i++)
   {
      sprintf(variable,",HPCP_APP_ARG%u=%s",i,jobInfo->args[i]) ;
      strcat(list,variable);
   }
   if(jobInfo->input!=NULL)
   {
      sprintf(variable,",HPCP_APP_INPUT=%s",jobInfo->input) ;
      strcat(list,variable);
   }
   if(jobInfo->output!=NULL)
   {
      sprintf(variable,",HPCP_APP_OUTPUT=%s",jobInfo->output) ;
      strcat(list,variable);
   }
   if(jobInfo->error!=NULL)
   {
      sprintf(variable,",HPCP_APP_ERROR=%s",jobInfo->error) ;
      strcat(list,variable);
   }
   if(jobInfo->wd!=NULL)
   {
      sprintf(variable,",HPCP_APP_WD=%s",jobInfo->wd) ;
      strcat(list,variable);
   }
   if(jobInfo->username!=NULL)
   {
      sprintf(variable,",HPCP_APP_USERNAME=%s",jobInfo->username) ;
      strcat(list,variable);
   }
}

char* trim(struct soap* soap, char* string)
{
   char * output = (char*) soap_malloc(soap, sizeof(char) * strlen(string) + 1);
   memset(output, 0, sizeof(char) * strlen(string) + 1);
   int j = 0, i;
   for(i = 0; i < strlen(string); i++){
      if ((string[i]!= ' ') && (string[i] != '\n')){
         output[j] = string[i];
	 j++;
      }
   }
   output[j] = '\0';
   return output;
}


/**
 * Gets a month string and returns the month number
 * @param month is a 3 letter string with the name of the month
 * @return integer number of month
*/
int monthToInt(char*month)
{
   if(!strcmp(month,"Jan"))
      return 1;
   if(!strcmp(month,"Feb"))
      return 2;
   if(!strcmp(month,"Mar"))
      return 3;
   if(!strcmp(month,"Apr"))
      return 4;
   if(!strcmp(month,"May"))
      return 5;
   if(!strcmp(month,"Jun"))
      return 6;
   if(!strcmp(month,"Jul"))
      return 7;
   if(!strcmp(month,"Aug"))
      return 8;
   if(!strcmp(month,"Sep"))
      return 9;
   if(!strcmp(month,"Oct"))
      return 10;
   if(!strcmp(month,"Nov"))
      return 11;
   if(!strcmp(month,"Dec"))
      return 12;
   return 0;
}

