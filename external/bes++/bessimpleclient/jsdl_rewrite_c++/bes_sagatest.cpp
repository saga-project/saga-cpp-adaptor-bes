/* ----------------------------------------------------------------
 *
 * This is a modified version of the bes++ besclient.c program, with changes
 * which reflect a SAGA adaptor structure.  If this client works against
 * a specific BES backend, the SAGA adaptor should work, too.
 *
 * bes_sagatest.c
 *   
 *      Client implementation of the OGSA Basic Execution Services
 *
 * Copyright (C) 2006-2009, Platform Computing Corporation. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
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
// #include "../config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#include "bes.hpp"

char * bes_activity_state[BES_State_Num] = 
{
  "Pending", 
  "Running", 
  "Cancelled", 
  "Failed", 
  "Finished"
};

int main (int argc, char * argv[])
{
  struct bes_context         * bes_context_ = NULL;
  struct bes_activity_status   status;
  epr_t                        host_epr_    = NULL;
  epr_t                        job_epr_     = NULL;

  char                       * capath       = "../besserver/cert/";
  char                       * x509cert     = NULL;
  char                       * x509pass     = NULL;
  char                       * user         = "merzky";
  char                       * pass         = "aaa";
  char                       * jsdl         = "sleep.jsdl";


  if ( bes_init (&bes_context_) )
  {
    exit (1);
  }

  if ( bes_security (bes_context_, x509cert, x509pass, capath, user, pass) )
  {
    std::cout << bes_get_lasterror (bes_context_) << std::endl;
    bes_finalize (&bes_context_);
    exit (2);
  }

  std::stringstream endpoint_ss;
  endpoint_ss << "<?xml version=\"1.0\"  encoding=\"UTF-8\"?>\n"
              << " <wsa:EndpointReference xmlns:wsa=\"http://www.w3.org/2005/08/addressing\">\n"
              << "  <wsa:Address>https://localhost:1236</wsa:Address>\n"
              << " </wsa:EndpointReference>\n";

  char * endpoint_cs = ::strdup (endpoint_ss.str ().c_str ());

  if ( bes_initEPRFromString (bes_context_, endpoint_cs, &host_epr_) )
  {
    std::cout << bes_get_lasterror (bes_context_) << std::endl;
    bes_finalize (&bes_context_);
    exit (3);
  }

  struct jsdl_job_definition * jd = new struct jsdl_job_definition;

  if ( jsdl_newJobDefinition (JSDL_HPC_PROFILE_APPLICATION, &jd) )
  {
    std::cout << bes_get_lasterror (bes_context_) << std::endl;
    bes_finalize (&bes_context_);
    exit (3);
  }

  jd->JobName       = "Sleep";
  jd->JobAnnotation = "SleepAnnotation";
  jd->JobProject    = "SleepProject";

  struct new_jsdl_range_value cpucount;

  cpucount.exact.value   = 1.0;
  cpucount.exact.epsilon = 0.0;

  jd->TotalCPUCount = cpucount;  

  struct jsdl_hpcp_application * app = (struct jsdl_hpcp_application *) jd->Application;

  char * args[]         = { "5" };

  app->Executable       = "/bin/sleep";
  app->num_args         = 1;
  app->Argument         = args;
  app->Output           = "/dev/null";
  app->WorkingDirectory = "/tmp";
  

  if ( bes_createActivity (bes_context_, host_epr_, jd, &job_epr_) )
  {
    std::cout << "create activity: " << bes_get_lasterror (bes_context_) << std::endl;
    bes_finalize (&bes_context_);
    exit (5);
  }

  struct bes_epr * epr = (struct bes_epr *) (job_epr_);
  std::cout << epr->str << std::endl;

  while ( true ) 
  {
    if ( bes_security (bes_context_, x509cert, x509pass, capath, user, pass) )
    {
      std::cout << "add_usertoken: " << bes_get_lasterror (bes_context_) << std::endl;
      exit (6);
    }

    if ( bes_getActivityStatuses (bes_context_, host_epr_, job_epr_, &status) )
    {
      std::cout << "get status: " << bes_get_lasterror (bes_context_) << std::endl;
      bes_finalize (&bes_context_);
      exit (7);
    }

    if ( status.state == BES_Cancelled ||
         status.state == BES_Failed    ||
         status.state == BES_Finished  )
    {
      break;
    }

    std::cout << "." << std::flush;
    ::sleep (1);
  }

  std::cout << std::endl;

  bes_freeEPR  (&host_epr_);
  bes_freeEPR  (&job_epr_);
  bes_finalize (&bes_context_);

  return 0;
}

