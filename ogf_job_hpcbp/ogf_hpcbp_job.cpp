//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

// system includes
#include <string.h>

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga adaptor includes
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/attribute.hpp>
#include <saga/saga/adaptors/file_transfer_spec.hpp>

// saga engine includes
#include <saga/impl/config.hpp>
#include <saga/impl/exception_list.hpp>

// saga package includes
#include <saga/saga/packages/job/adaptors/job_self.hpp>
#include <saga/saga/packages/job/job_description.hpp>

// adaptor includes
#include "ogf_hpcbp_job.hpp"

namespace sja = saga::job::attributes;

////////////////////////////////////////////////////////////////////////
namespace ogf_hpcbp_job
{
  // constructor
  job_cpi_impl::job_cpi_impl (proxy                           * p, 
                              cpi_info const                  & info,
                              saga::ini::ini const            & glob_ini, 
                              saga::ini::ini const            & adap_ini,
                              TR1::shared_ptr <saga::adaptor>   adaptor)
    : base_cpi (p, info, adaptor, cpi::Noflags)
    , session_ (p->get_session ())
    , state_   (saga::job::New)
  {
    instance_data     idata (this);
    adaptor_data_type adata (this);

    rm_   = idata->rm_;
    rm_s_ = rm_.get_string ();

    // check if URL is usable
    if ( ! rm_.get_scheme ().empty ()    &&
           rm_.get_scheme () != "bes"    && 
           rm_.get_scheme () != "http"   && 
           rm_.get_scheme () != "https"  && 
           rm_.get_scheme () != "epr"    && 
           rm_.get_scheme () != "any"    )
    {
      SAGA_OSSTREAM strm;
      strm << "Could not initialize job service for [" << rm_ << "]. " 
           << "Only these schemas are supported: any://, bes://, http(s)://, epr://, or none.";

      SAGA_ADAPTOR_THROW (SAGA_OSSTREAM_GETSTRING (strm), 
                          saga::adaptors::AdaptorDeclined);
    }
    
    if ( rm_.get_scheme () == "any" ||
         rm_.get_scheme () == "bes" )
    {
      rm_.set_scheme ("https");
    }

    if ( rm_.get_scheme () == "epr" )
    {
      // read epr from file, using saga::filesystem
      saga::url e (rm_);
      e.set_scheme ("any");

      saga::filesystem::file f (e);
      saga::size_t           s = f.get_size ();
      saga::mutable_buffer   b (s + 1);

      f.read (b);

      static_cast <char *> (b.get_data ())[s] = '\0';
      
      bp_.set_host_epr (static_cast <const char*> (b.get_data ()));
    }
    else
    {
      bp_.set_host_endpoint (rm_.get_string ());
    }

    // cycle over contexts and see which ones we can use.  
    // We accept x509 and UserPass

    bool context_found = false;
    std::vector <saga::context> contexts = session_.list_contexts ();

    for ( unsigned int i = 0; i < contexts.size (); i++ )
    {
      saga::context c = contexts[i];

      if ( c.attribute_exists (saga::attributes::context_type) )
      {
        if ( c.get_attribute  (saga::attributes::context_type) == "UserPass" ||
             c.get_attribute  (saga::attributes::context_type) == "X509" )
        {
          std::string user  ("");
          std::string pass  ("");
          std::string cert  ("");
          std::string key   ("");
          std::string cadir ("");

          if ( c.attribute_exists (saga::attributes::context_userid) )
          {
            user = c.get_attribute (saga::attributes::context_userid);
          }

          if ( c.attribute_exists (saga::attributes::context_userpass) )
          {
            pass = c.get_attribute (saga::attributes::context_userpass);
          }

          if ( c.attribute_exists (saga::attributes::context_certrepository) )
          {
            cadir = c.get_attribute (saga::attributes::context_certrepository);
          }

          if ( c.attribute_exists (saga::attributes::context_usercert) )
          {
            cert = c.get_attribute (saga::attributes::context_usercert);
          }

          if ( c.attribute_exists (saga::attributes::context_userkey) )
          {
            key = c.get_attribute (saga::attributes::context_userkey);
          }

          bp_.set_security (cert, key, cadir, user, pass);

          context_found = true;
        }

        if ( context_found )
        {
          // TODO: test if context can be used to contact server.  If not, set
          // context_found to false again, and free the bes context
        }
      }
    }

    if ( ! context_found )
    {
      // this is not really an error, maybe there is no security on the endpoint
      // whatsoever - but its actually unlikely that calls will succeed.  So, we
      // print a warning
      SAGA_ADAPTOR_THROW ("No suitable context found - use either X509 or UserPass context",
                          saga::AuthenticationFailed);
    }


    // TODO: check if host exists and can be used, otherwise throw BadParameter
    // easiest would probably to run an invalid job request and see if we get
    // a sensible error...


    if ( idata->init_from_jobid_ )
    {
      SAGA_ADAPTOR_THROW ("Job reconnect is not yet implemented",
                          saga::NotImplemented);

      jobid_ = idata->jobid_;

      // TODO: confirm that the job exists on the host (get state)
      // TODO: fill job description from the jobs jsdl

      // we successfully inited from job id -- store job description
      // idata->jd_ = jd_;

      state_ = saga::job::Running;
    }
    else
    {
      // init from job description
      jd_ = idata->jd_;
      
      if ( ! jd_.attribute_exists (sja::description_executable) )
      {
        SAGA_ADAPTOR_THROW ("job description misses executable",
                            saga::BadParameter);
      }

      jsdl_.set_executable (jd_.get_attribute (sja::description_executable));


      if ( jd_.attribute_exists (sja::description_arguments) )
      {
        jsdl_.set_args (jd_.get_vector_attribute (sja::description_arguments));
      }

      if ( jd_.attribute_exists (sja::description_job_project) )
      {
        jsdl_.set_job_project (jd_.get_attribute (sja::description_job_project));
      }

      if ( jd_.attribute_exists (sja::description_total_cpu_count) )
      {
        jsdl_.set_total_cpu_count  (jd_.get_attribute (sja::description_total_cpu_count));
      }

      if ( jd_.attribute_exists (sja::description_input) )
      {
        jsdl_.set_input (jd_.get_attribute (sja::description_input));
      }

      if ( jd_.attribute_exists (sja::description_output) )
      {
        jsdl_.set_output (jd_.get_attribute (sja::description_output));
      }

      if ( jd_.attribute_exists (sja::description_error) )
      {
        jsdl_.set_error (jd_.get_attribute (sja::description_error));
      }

      if ( jd_.attribute_exists (sja::description_working_directory) )
      {
        jsdl_.set_working_directory (jd_.get_attribute (sja::description_working_directory));
      }

      if ( jd_.attribute_exists (sja::description_file_transfer) )
      {
        jsdl_.set_file_transfers (jd_.get_vector_attribute (sja::description_file_transfer));
      }
    }


    // FIXME: register metrics etc.
  }


  // destructor
  job_cpi_impl::~job_cpi_impl (void)
  {
  }


  //  SAGA API functions
  void job_cpi_impl::sync_get_state (saga::job::state & ret)
  {
    adaptor_data_type adata (this);

    state_ = adata->get_saga_state (bp_.get_state (job_epr_));

    ret = state_;
  }

  void job_cpi_impl::sync_get_description (saga::job::description & ret)
  {
    ret = jd_;
  }

  void job_cpi_impl::sync_get_job_id (std::string & ret)
  {
    ret = jobid_;
  }

  void job_cpi_impl::sync_get_stdin (saga::job::ostream & ret)
  {
    // not available for BES/JSDL
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_get_stdout (saga::job::istream & ret)
  {
    // not available for BES/JSDL
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_get_stderr (saga::job::istream & ret)
  {
    // not available for BES/JSDL
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_checkpoint (saga::impl::void_t & ret)
  {
    // not available for BES/JSDL
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_migrate (saga::impl::void_t     & ret, 
                                   saga::job::description   jd)
  {
    // not available for BES/JSDL
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_signal (saga::impl::void_t & ret, 
                                  int            signal)
  {
    // not available for BES/JSDL
    // TODO: check
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }


  //  suspend the child process 
  void job_cpi_impl::sync_suspend (saga::impl::void_t & ret)
  {
    // not available for BES/JSDL
    // TODO: check
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  //  suspend the child process 
  void job_cpi_impl::sync_resume (saga::impl::void_t & ret)
  {
    // not available for BES/JSDL
    // TODO: check
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }


  //////////////////////////////////////////////////////////////////////
  // inherited from the task interface
  void job_cpi_impl::sync_run (saga::impl::void_t & ret)
  {
    if ( state_ != saga::job::New )
    {
      SAGA_ADAPTOR_THROW ("can run only 'New' jobs", saga::IncorrectState);
    }

    job_epr_ = bp_.run (jsdl_);

    std::string s1 (rm_s_);
    std::string s2 (::strdup (job_epr_->str));

    jobid_ = std::string ("[") + s1 + "]-[" + s2 + "]";

    {
      adaptor_data_type adata (this); // scoped lock
      state_ = adata->get_saga_state (bp_.get_state (job_epr_));
    }

    while ( state_ == saga::job::New )
    {
      // std::cout << "waiting for job state to change" << std::endl;
      ::sleep (1);

      adaptor_data_type adata (this); // scoped lock
      state_ = adata->get_saga_state (bp_.get_state (job_epr_));
    }

    // std::cout << "Successfully submitted activity: " << jobid_ << std::endl;
  }

  void job_cpi_impl::sync_cancel (saga::impl::void_t & ret, 
                                  double timeout)
  {
    try
    {
      bp_.terminate (job_epr_);
    }
    catch ( const char * msg )
    {
      SAGA_ADAPTOR_THROW (msg, saga::NoSuccess);
    }
  }

  //  wait for the child process to terminate
  void job_cpi_impl::sync_wait (bool   & ret, 
                                double   timeout)
  {
    adaptor_data_type adata (this);
    double time = 0.0;

    while ( time < timeout ) 
    {
      state_ = adata->get_saga_state (bp_.get_state (job_epr_));

      if ( state_ == saga::job::Canceled ||
           state_ == saga::job::Failed   ||
           state_ == saga::job::Done     )
      {
        break;
      }

      ::sleep (1);
      time += 1.0;
    }
  }

  // TODO: add state polling and metrics support

} // namespace ogf_hpcbp_job
////////////////////////////////////////////////////////////////////////

