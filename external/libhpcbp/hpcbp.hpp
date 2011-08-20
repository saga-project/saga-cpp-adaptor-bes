
//  Copyright (c) 2009-2011 Andre Merzky <andre@merzky.net>
//  Distributed under the GPLv.2 - see accompanying LICENSE file.


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

  struct combined_state
  {
    enum state   state;
    std::string  substate;
  };

  typedef struct bes_epr * job_handle;

  class job_description
  {
    private:
      struct jsdl_job_definition * jsdl_;

    public:

      enum staging_flag
      {
        Overwrite     = 1,
        Append        = 2,
        DontOverwrite = 4
      };

      job_description  (void);
      ~job_description (void);

      struct jsdl_job_definition   * get_jsdl (void) const;
      struct jsdl_hpcp_application * get_app  (void);

      void set_job_name          (std::string  s);
      void set_job_annotation    (std::string  s);
      void set_job_project       (std::string  s);
      void set_total_cpu_count   (std::string  s);
      void set_executable        (std::string  s);
      void set_input             (std::string  s);
      void set_output            (std::string  s);
      void set_environment       (std::string  s);
      void set_error             (std::string  s);
      void set_working_directory (std::string  s);
      void set_args              (std::vector  <std::string> args);
      void set_file_transfers    (std::vector  <std::string> specs);

      void dump                  (void);
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
      combined_state   get_state          (job_handle & job_epr);
      job_description  get_description    (job_handle & job_epr);
  };

} // namespace hpcbp

#endif // HPCBP_CONNECTOR_HPP

