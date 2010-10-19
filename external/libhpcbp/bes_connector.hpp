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

#ifndef BES_CONNECTOR_HPP
#define BES_CONNECTOR_HPP

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#include "bes.hpp"

enum hpcbp_activity_state 
{
  HPCBP_Pending   = BES_Pending  ,
  HPCBP_Running   = BES_Running  ,
  HPCBP_Cancelled = BES_Cancelled,
  HPCBP_Failed    = BES_Failed   ,
  HPCBP_Finished  = BES_Finished ,
  HPCBP_State_Num = BES_State_Num
};

class hpcbp_job_handle
{
};

class hpcbp_connector
{
  // why the heck do we need to set security before every BES call?
  // That makes thread safety tricky...

  private: 
    struct bes_context * hpcbp_context_;
    epr_t                host_epr_;
    std::string          host_;
    std::string          x509cert_;
    std::string          x509pass_;
    std::string          capath_;
    std::string          user_;
    std::string          pass_;

    void init_security_ (void)
    {
      if ( bes_security (hpcbp_context_, 
                         x509cert_.c_str (), 
                         x509pass_.c_str (), 
                         capath_.c_str (), 
                         user_.c_str (), 
                         pass_.c_str ()) )
      {
        throw bes_get_lasterror (hpcbp_context_);
      }

      // std::cout << "bes security initialized" << std::endl;
    }


  public:
    bes_connector (std::string x509cert, 
                   std::string x509pass, 
                   std::string capath, 
                   std::string user,     
                   std::string pass)
      : hpcbp_context_ (NULL)
      , host_epr_    (NULL)
      , x509cert_    (x509cert)
      , x509pass_    (x509pass)
      , capath_      (capath)
      , user_        (user)
      , pass_        (pass)
    {
      if ( bes_init (&hpcbp_context_) )
      {
        throw ("Cannot init bes context");
      }

      // std::cout << "bes connector initialized" << std::endl;

    }

    ~bes_connector (void)
    {
      bes_freeEPR  (&host_epr_);
      bes_finalize (&hpcbp_context_);
      // std::cout << "bes connector finalized" << std::endl;
    }

    void set_host_endpoint (const std::string host)
    {
      host_ = host;

      std::stringstream endpoint_ss;
      endpoint_ss << "<?xml version=\"1.0\"  encoding=\"UTF-8\"?>\n"
                  << " <wsa:EndpointReference xmlns:wsa=\"http://www.w3.org/2005/08/addressing\">\n"
                  << "  <wsa:Address>" << host_ << "</wsa:Address>\n"
                  << " </wsa:EndpointReference>\n";

      std::string endpoint_cs = endpoint_ss.str ();

      if ( bes_initEPRFromString (hpcbp_context_, endpoint_cs.c_str (), &host_epr_) )
      {
        // Cannot initialize bes endpoint
        throw (bes_get_lasterror (hpcbp_context_));
      }

      // std::cout << "host epr points to " << host_ << std::endl;
    }

    struct bes_epr * run_job (struct jsdl_job_definition * jd)
    {
      init_security_ ();

      epr_t epr;

      if ( bes_createActivity (hpcbp_context_, host_epr_, jd, &epr) )
      {
        throw bes_get_lasterror (hpcbp_context_);
      }

      return (struct bes_epr *) (epr);

      // FIXME: epr is leaking memory here... - should be wrapped in
      // separate class
    }

    struct bes_activity_status get_state (struct bes_epr * job_epr)
    {
      init_security_ (); 

      struct bes_activity_status status;

      if ( bes_getActivityStatuses (hpcbp_context_, host_epr_, job_epr, &status) )
      {
        throw (bes_get_lasterror (hpcbp_context_));
      }

      return status;
    }
};

#endif // BES_CONNECTOR_HPP

