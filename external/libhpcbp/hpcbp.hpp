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

namespace hpcbp
{
  enum state 
  {
    Pending   = BES_Pending  ,
    Running   = BES_Running  ,
    Canceled  = BES_Cancelled,
    Failed    = BES_Failed   ,
    Finished  = BES_Finished ,
    State_Num = BES_State_Num
  };

  typedef struct bes_epr * job_handle;

  class job_description
  {
    private:
      struct jsdl_job_definition * jsdl_;

    public:
      job_description  (void);
      ~job_description (void);

      struct jsdl_job_definition   * get_jsdl (void) const;
      struct jsdl_hpcp_application * get_app  (void);

      void set_job_name          (std::string s);
      void set_job_annotation    (std::string s);
      void set_job_project       (std::string s);
      void set_total_cpu_count   (std::string s);
      void set_executable        (std::string s);
      void set_input             (std::string s);
      void set_output            (std::string s);
      void set_error             (std::string s);
      void set_working_directory (std::string s);
      void set_args              (std::vector <std::string> args);
  };



  class connector
  {
    private: 
      struct bes_context * bes_context_;
      epr_t                host_epr_;
      char               * host_;
      std::string          x509cert_;
      std::string          x509pass_;
      std::string          capath_;
      std::string          user_;
      std::string          pass_;

      void init_security_ (void);


    public:
      connector (void);
      ~connector (void);

      void             set_security       (std::string x509cert, std::string x509pass, std::string capath, 
                                           std::string user,     std::string pass);
      void             set_host_epr       (const std::string epr);
      void             set_host_endpoint  (const std::string host);
      job_handle       run                (const job_description & jd);
      void             terminate          (job_handle & job_epr);
      state            get_state          (job_handle & job_epr);
      job_description  get_description    (job_handle & job_epr);
  };

} // namespace hpcbp

#endif // HPCBP_CONNECTOR_HPP

