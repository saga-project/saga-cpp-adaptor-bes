/* ----------------------------------------------------------------
 * rm_saga.c
 *
 *      SAGA backend implementation of the OGSA Basic Execution Services
 *
 * Copyright (C) Andre Merzky <andre@merzky.net>
 *               CCT, Louisiana State University
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

#define CXX

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

#include <saga/saga.hpp>
#include <boost/lexical_cast.hpp>

//! Maximum length of the environment variables string
#define MAXVAR_BUFFER 4096

//GLOBAL VARIABLES

/**
 * SAGA does not need any special initialization
 */
extern "C" int rm_initialize (struct soap * soap, 
                              char        * serverName)
{
  return BESE_OK;
}

/**
 * Terminates a job. 
 * @param jobid is the PID assigned by the queue
 * @param user is a string with the account of the user who requested the 
 * job deletion
 * @return 0 if correct, non-zero if error
 */
extern "C" int rm_terminateJob (struct soap * soap, 
                                char        * jobid, 
                                char        * user)
{
  if ( ! jobid || std::string (jobid).empty () )
  {
    // empty jobid - not sure what to do
    return BESE_BAD_ARG;
  }

  // FIXME: dig js URL from jobid
  try
  {
    saga::job::service js;
    saga::job::job j = js.get_job (jobid);
    j.cancel ();
  }
  catch ( const saga::exception & e )
  {
    std::cout << "Could not kill job " 
              << jobid 
              << " : " 
              << e.what () 
              << std::endl;
    return BESE_BACKEND;
  }

  return BESE_OK;
}

/**
 * Gets the status of the job. 
 *
 * It maps the different states of SAGA jobs to BES states.
 * @param jobid is the saga job id
 * @param user is a string with the account of the user who requested the 
 * job status
 * @param jobStatus is an output parameter with the status of the job -NULL
 * if error-
 * @return 0 if correct, non-zero if error
 */
extern "C"
int rm_getJobStatus(struct soap * soap, 
                    char        * jobid, 
                    char        * user, 
                    struct bes__ActivityStatusType ** jobStatus)
{
  struct bes__ActivityStatusType *activityStatus;

  if ( ! jobid || std::string (jobid).empty () )
  {
    // empty jobid - not sure what to do
    return BESE_BAD_ARG;
  }

  if ( ! jobStatus )
  {
    return BESE_BAD_ARG;
  }

  activityStatus = (struct bes__ActivityStatusType*) soap_malloc (soap, sizeof (struct bes__ActivityStatusType));

  if ( activityStatus == NULL )
  {
    return BESE_MEM_ALLOC;
  }

  ::memset (activityStatus, 0, sizeof (struct bes__ActivityStatusType));

  bool ok = true;

  try
  {
    saga::job::service js;
    saga::job::job j = js.get_job (jobid);

    switch ( j.get_state () )
    {
      case saga::job::New:       activityStatus->state = Pending;   break;
      case saga::job::Running:   activityStatus->state = Running;   break;
      case saga::job::Suspended: activityStatus->state = Running;   break;
      case saga::job::Canceled:  activityStatus->state = Cancelled; break;
      case saga::job::Failed:    activityStatus->state = Failed;    break;
      case saga::job::Done:      activityStatus->state = Finished;  break;
      default:                   ok = false;                        break;
    }
  }
  catch ( const saga::exception & e )
  {
    std::cout << "Could not get job state for " 
      << jobid 
      << " : " 
      << e.what () 
      << std::endl;
    return BESE_BACKEND;
  }

  if ( ! ok )
  {
    std::cout << "invalid state state for " 
      << jobid 
      << std::endl;
    return BESE_BACKEND;
  }

  *jobStatus = activityStatus;

  return BESE_OK;
}

/**
 * Gets the information about a job. 
 *
 * @param jobid is the PID assigned by the queue
 * @param soap
 * @param job_info is an output parameter which contains the information
 * about the job submission request and the job state (NULL if error).
 * @return 0 if correct, non-zero if error
 */
extern "C"
int rm_getJobInfo (struct soap    *  soap, 
                   char           *  jobid, 
                   char           *  user, 
                   struct jobcard ** job_info )
{
  if ( ! jobid || std::string (jobid).empty () )
  {
    // empty jobid - not sure what to do
    return BESE_BAD_ARG;
  }

  if ( ! job_info )
  {
    return BESE_BAD_ARG;
  }

  struct jobcard * jc = (struct jobcard *) soap_malloc (soap, sizeof (struct jobcard));

  try
  {  
    saga::job::service js;
    saga::job::job j = js.get_job (jobid);

    jc->jobname       = "";
    jc->jobproject    = "";
    jc->num_hostnames = 0;
    jc->hostnames     = NULL;
    jc->exclusive     = 0;
    jc->osname        = "";
    jc->osver         = "";
    jc->cpuarch       = "";
    jc->executable    = "";
    jc->num_args      = 0;
    jc->args          = NULL;
    jc->input         = "";
    jc->output        = "";
    jc->error         = "";
    jc->wd            = "$HOME";
    jc->environment   = NULL;
    jc->username      = "";

    // while ( env )
    // {
    //   struct envvar* newEnvVar= (struct envvar *) soap_malloc (soap,
    //                                                            sizeof (struct envvar));
    //   memset (newEnvVar, 0, sizeof (struct envvar));
    //   newEnvVar->name      = soap_strdup (soap, env.key);
    //   newEnvVar->val       = soap_strdup (soap, env.val);
    //   newEnvVar->next      = job_info->environment;
    //
    //   jc->environment = newEnvVar;
    // }
  }
  catch ( const saga::exception & e )
  {
    std::cout << "Could not get job info for " 
      << jobid 
      << " : " 
      << e.what () 
      << std::endl;
    return BESE_BACKEND;
  }

  *job_info = jc;

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
extern "C"
int rm_getJobList(struct soap      *  soap, 
                  struct rm_filter *  filters,
                  struct rm_job    ** joblist, 
                  int              *  numjobs)
{
  struct rm_job * jobList;
  char   filename[128];
  char   command[256];
  struct rm_filter* filter;
  int    jobid;
  int    scan;
  char   ignore[128];
  int    numJobs = 0;

  // set up filter maps.  
  // NOTE: we do not support time and id filters, as ids are not numerical in
  // SAGA, and time 
  std::map <std::string, bool> user_map;
  std::map <std::string, bool> state_map;

  // we do not su
  std::vector <std::pair <long, long> > id_range_map;
  std::vector <std::pair <std::string, std::string> > time_range_map;

  filter = filters;
  while ( (filter != NULL) )
  {
    if ( filter->user != NULL ) 
    {
      user_map [filter->user] = true;
    }

    if ( filter->state != NULL ) 
    {
      state_map [filter->state] = true;
    }

    filter = filter->next;
  }

  try
  {
    saga::job::service js;
    std::vector <std::string> ids = js.list ();

    for ( unsigned int i = 0; i < ids.size (); i++ )
    {
      saga::job::job         j  = js.get_job (ids[i]);
      saga::job::description jd = j.get_description ();

      // FIXME: this gives the user ID for the first session context, not the
      // user ID for whoever actually owns this job.  Not sure this is possible
      // in SAGA...
      std::string user  = j.get_session ().list_contexts ()[0].get_attribute ("UserID");
      std::string state = boost::lexical_cast <std::string> (j.get_state ());

      if ( user_map .count (user ) == 0 &&
           state_map.count (state) == 0 )
      {
        struct rm_job * newJob = (struct rm_job *) soap_malloc (soap, 
                                                                sizeof (struct rm_job));
        if (  ! newJob )
        {
          return BESE_MEM_ALLOC;
        }

        newJob->jobid = soap_strdup (soap, ids[i].c_str ());
        newJob->next  = jobList;
        jobList = newJob;

        numJobs++;
      }
    }
  }
  catch ( const saga::exception & e )
  {
    std::cout << "Could not get job list : " 
      << e.what () 
      << std::endl;
    return BESE_BACKEND;
  }

  *joblist = jobList;
  *numjobs = numJobs;

  return BESE_OK;
}


extern "C"
int rm_getResourceList (struct soap        *  soap, 
                        struct rm_filter   *  filter, 
                        struct rm_resource ** resourcelist, 
                        int                *  num_resources)
{
  // SAGA does not have resource discovery :-(
  return BESE_SYS_ERR;
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
extern "C"
int rm_getClusterInfo (struct soap             *  soap, 
                       struct rm_clusterInfo   ** clusterInf
                       /* , int    compactResources */
                      )
{
  // SAGA does not have resource discovery :-(
  return BESE_SYS_ERR;
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
extern "C"
int rm_submitJob ( struct soap    *  soap, 
                   struct jobcard *  jc, 
                   char           *  user, 
                   char           ** return_jobid)
{
  try
  {
    using namespace saga::job::attributes;

    saga::url              rm;  // job service contact
    saga::job::description jd;

    if ( ! jc )
    {
      return BESE_BAD_ARG;
    }


    if ( jc->executable )
    {
      jd.set_attribute (description_executable, jc->executable);
    }
    else
    {
      std::cerr << "Error: Need to have the executable name" << std::endl;
      return BESE_BAD_ARG;
    }


    if ( jc->input )
    {
      jd.set_attribute (description_input, jc->input);
    }


    if ( jc->output )
    {
      jd.set_attribute (description_output, jc->output);
    }


    if ( jc->error )
    {
      jd.set_attribute (description_error, jc->error);
    }


    if ( jc->osname )
    {
      jd.set_attribute (description_operating_system_type, jc->osname);
    }


    if ( jc->cpuarch )
    {
      jd.set_attribute (description_cpu_architecture, jc->cpuarch);
    }


    if ( jc->wd )
    {
      jd.set_attribute (description_working_directory, jc->wd);
    }


    if ( jc->jobproject )
    {
      // FIXME: saga has it wrong, project is scalar!
      std::vector <std::string> tmp;
      tmp.push_back (jc->jobproject);
      jd.set_vector_attribute (description_job_project, tmp);
    }


    // unsupported
    // if ( jc->jobname != NULL )
    // {
    //   jd.set_attribute (description_jobname, jc->jobname);
    // }
    //

    if ( jc->icpu != 0 )
    {
      jd.set_attribute (description_total_cpu_count, 
                        boost::lexical_cast <std::string> (jc->icpu));
    }


    if ( jc->ipmem != 0 )
    {
      jd.set_attribute (description_total_physical_memory, 
                        boost::lexical_cast <std::string> (jc->ipmem));
    }


    // unsupported by SAGA
    // if ( jc->ivmem != 0 )
    // {
    //   jd.set_attribute (description_vmem, 
    //                     boost::lexical_cast <std::string> (jc->ivmem));
    // }


    std::vector <std::string> args;
    for ( int i = 0; i < jc->num_args; i++ ) 
    {
      args.push_back (jc->args[i]);
    }
    jd.set_vector_attribute (description_arguments, args);


    std::vector <std::string> chosts;
    for ( int i = 0; i < jc->num_hostnames; i++ )
    {
      if ( jc->hostnames[i] )
      {
        chosts.push_back (jc->hostnames[i]);
      }
    }
    jd.set_vector_attribute (description_candidate_hosts, chosts);


    // Store the submitting info in environment variables for later use
    std::vector <std::string> env;
    struct envvar* variableList = jc->environment;
    for ( ; variableList != NULL; variableList = variableList->next )
    {
      std::string key = variableList->name;
      std::string val = variableList->val;

      env.push_back (key + "=" + val);

    }
    jd.set_vector_attribute (description_environment, env);


    std::vector <std::string> transfers;
    bool   cleanup   = true; // clean all or none
    struct fileStage * stage = jc->files;
    while ( stage != NULL )
    {
      if ( stage->credential )
      {
        std::cerr << "Warning: user/pass unsupported for file staging" 
          << std::endl;
      }

      if ( stage->source != NULL )
      {
        std::string tmp (stage->source);
        tmp += " > ";
        tmp += stage->filename;
      }
      else if ( stage->target != NULL )
      {
        std::string tmp (stage->filename);
        tmp += " < ";
        tmp += stage->target;
      }

      // if any file needs to be kept, do *not* set the delete flag
      if ( ! stage->del )
      {
        cleanup = false;
      }

      stage = stage->next;
    }

    if ( ! transfers.empty () )
    {
      jd.set_vector_attribute (description_file_transfer, transfers);
    }

    jd.set_attribute (description_cleanup, boost::lexical_cast <std::string> (cleanup));


    // create and run job
    std::cout << "job starting" << std::endl;
    saga::job::service js;
    saga::job::job     j = js.create_job (jd);
    j.run ();

    std::cout << "job started:" << j.get_job_id () << std::endl;

    * return_jobid = soap_strdup (soap, j.get_job_id ().c_str ());
  }
  catch ( const saga::exception & e )
  {
    std::cerr << "Error in job submission: "
      << std::endl
      << e.what ()
      << std::endl;

    return BESE_SYS_ERR;
  }

  return BESE_OK;
}

