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

#ifndef HPCBP_CONNECTOR_HPP
#define HPCBP_CONNECTOR_HPP

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <vector>

#include "bes.hpp"

enum hpcbp_state 
{
  HPCBP_Pending   = BES_Pending  ,
  HPCBP_Running   = BES_Running  ,
  HPCBP_Cancelled = BES_Cancelled,
  HPCBP_Failed    = BES_Failed   ,
  HPCBP_Finished  = BES_Finished ,
  HPCBP_State_Num = BES_State_Num
};

typedef struct bes_epr * hpcbp_job_handle;

class hpcbp_job_description
{
  private:
    struct jsdl_job_definition * jsdl_;

  public:
    hpcbp_job_description (void)
      : jsdl_ (NULL)
    {
      jsdl_ = new struct jsdl_job_definition;
      
      if ( jsdl_newJobDefinition (JSDL_HPC_PROFILE_APPLICATION, &jsdl_) )
      {
        throw "Canot create jd";
      }
    }

    ~hpcbp_job_description (void)
    {
    }

    struct jsdl_job_definition * get_jsdl (void) const
    {
      return jsdl_;
    }

    struct jsdl_hpcp_application * get_app (void)
    {
      return (struct jsdl_hpcp_application *) jsdl_->Application;
    }

    void set_job_name (std::string s)
    {
      // FIXME: leaking!
      jsdl_->JobName = ::strdup (s.c_str ());
    }

    void set_job_annotation (std::string s)
    {
      jsdl_->JobAnnotation = ::strdup (s.c_str ());
    }

    void set_job_project (std::string s)
    {
      jsdl_->JobProject = ::strdup (s.c_str ());
    }

    void set_total_cpu_count (unsigned int n)
    {
      struct jsdl_range_value * cpucount;

      if ( jsdl_newRangeValue (&cpucount) )
      {
        throw "Can't allocate RangeValue";
      }
      
      if (jsdl_addExact (cpucount, n, 0.0) )
      {
        throw "Can't add Exact to RangeValue";
      }
      
      jsdl_->TotalCPUCount = cpucount;  
    }

    void set_executable (std::string s)
    {
      get_app ()->Executable = ::strdup (s.c_str ());
    }

    void set_input (std::string s)
    {
      get_app ()->Input = ::strdup (s.c_str ());
    }

    void set_output (std::string s)
    {
      get_app ()->Output = ::strdup (s.c_str ());
    }

    void set_error (std::string s)
    {
      get_app ()->Error = ::strdup (s.c_str ());
    }

    void set_working_directory (std::string s)
    {
      get_app ()->WorkingDirectory = ::strdup (s.c_str ());
    }

    void set_args (std::vector <std::string> args)
    {
      char ** c_args = NULL;

      if ( args.size () > 0 )
      {
        c_args = (char**) calloc (args.size (), sizeof (char*));

        for ( unsigned int i = 0; i < args.size (); i++ )
        {
          c_args[i] = ::strdup (args[i].c_str ());
        }
      }
      
      get_app ()->num_args = args.size ();
      get_app ()->Argument = c_args;
    }
};



class hpcbp_connector
{
  // why the heck do we need to set security before every BES call?
  // That makes thread safety tricky...

  private: 
    struct bes_context * bes_context_;
    epr_t                host_epr_;
    char               * host_;
    char               * x509cert_;
    char               * x509pass_;
    char               * capath_;
    char               * user_;
    char               * pass_;

    void init_security_ (void)
    {
      if ( bes_security (bes_context_, x509cert_, x509pass_, capath_, user_, pass_) )
      {
        throw bes_get_lasterror (bes_context_);
      }

      std::cout << "bes security initialized" << std::endl;
    }


  public:
    hpcbp_connector (char * x509cert, 
                     char * x509pass, 
                     char * capath, 
                     char * user,     
                     char * pass)
      : bes_context_ (NULL)
      , host_epr_    (NULL)
      , x509cert_    (x509cert)
      , x509pass_    (x509pass)
      , capath_      (capath)
      , user_        (user)
      , pass_        (pass)
    {
      if ( bes_init (&bes_context_) )
      {
        throw ("Cannot init bes context");
      }

      std::cout << "bes connector initialized" << std::endl;

    }

    ~hpcbp_connector (void)
    {
      bes_freeEPR  (&host_epr_);
      bes_finalize (&bes_context_);
      std::cout << "bes connector finalized" << std::endl;
    }

    void set_host_endpoint (const std::string host)
    {
      host_ = strdup (host.c_str ());

      std::stringstream endpoint_ss;
      endpoint_ss << "<?xml version=\"1.0\"  encoding=\"UTF-8\"?>\n"
                  << " <wsa:EndpointReference xmlns:wsa=\"http://www.w3.org/2005/08/addressing\">\n"
                  << "  <wsa:Address>" << host_ << "</wsa:Address>\n"
                  << " </wsa:EndpointReference>\n";

      char * endpoint_cs = ::strdup (endpoint_ss.str ().c_str ());

      if ( bes_initEPRFromString (bes_context_, endpoint_cs, &host_epr_) )
      {
        // Cannot initialize bes endpoint
        throw (bes_get_lasterror (bes_context_));
      }

      std::cout << "host epr points to " << host_ << std::endl;
    }

    hpcbp_job_handle run_job (const hpcbp_job_description & jd)
    {
      init_security_ ();

      epr_t epr;

      if ( bes_createActivity (bes_context_, host_epr_, jd.get_jsdl (), &epr) )
      {
        throw bes_get_lasterror (bes_context_);
      }

      hpcbp_job_handle job_epr = (hpcbp_job_handle) epr;
      
      // std::cout << job_epr->str << std::endl;

      // FIXME: epr is leaking memory here... - should be wrapped in
      // separate class
      return job_epr;
    }

    hpcbp_state get_state (hpcbp_job_handle job_epr)
    {
      init_security_ (); 

      struct bes_activity_status status;

      if ( bes_getActivityStatuses (bes_context_, host_epr_, job_epr, &status) )
      {
        throw (bes_get_lasterror (bes_context_));
      }

      // FIXME: set state and substate attribs FIXME: run thread which
      // monitors job state asynchronously, and which sets attribs and
      // metrics
      return (hpcbp_state) status.state; 
    }
};

#endif // HPCBP_CONNECTOR_HPP

