
//  Copyright (c) 2009-2011 Andre Merzky <andre@merzky.net>
//  Distributed under the GPLv.2 - see accompanying LICENSE file.


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
#include "bes_hpcbp_job.hpp"

namespace sja = saga::job::attributes;

////////////////////////////////////////////////////////////////////////
namespace bes_hpcbp_job
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
    try 
    {
      bp_.initialize ();
    }
    catch ( const char * m )
    {
      SAGA_ADAPTOR_THROW ((std::string ("Could not initialize backend library: ") + m).c_str (), 
                          saga::NoSuccess);
    }
    catch ( const saga::exception & e )
    {
      SAGA_ADAPTOR_THROW ((std::string ("Could not initialize backend library: ") + e.what ()).c_str (), 
                          saga::NoSuccess);
    }

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

      try 
      {
        saga::filesystem::file f (e);
        saga::size_t           s = f.get_size ();
        saga::mutable_buffer   b (s + 1);

        f.read (b);

        static_cast <char *> (b.get_data ())[s] = '\0';

        bp_.set_host_epr (static_cast <const char*> (b.get_data ()));
      }
      catch ( const char * m )
      {
        SAGA_ADAPTOR_THROW ((std::string ("Could not handle EPR: ") + m).c_str (), 
                            saga::BadParameter);
      }
      catch ( const saga::exception & e )
      {
        SAGA_ADAPTOR_THROW ((std::string ("Could not handle EPR: ") + e.what ()).c_str (), 
                            saga::BadParameter);
      }
    }
    else
    {
      try 
      {
        bp_.set_host_endpoint (rm_.get_string ());
      }
      catch ( const char * m )
      {
        SAGA_ADAPTOR_THROW ((std::string ("Could not handle endpoint url: ") + m).c_str (), 
                            saga::BadParameter);
      }
      catch ( const saga::exception & e )
      {
        SAGA_ADAPTOR_THROW ((std::string ("Could not handle endpoint url: ") + e.what ()).c_str (), 
                            saga::BadParameter);
      }
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
             c.get_attribute  (saga::attributes::context_type) == "X509"     ||
             c.get_attribute  (saga::attributes::context_type) == "x509" )
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
          // TODO? test if context can be used to contact server.  
          // If not, set context_found to false again, and free 
          // the BES context
        }
      }
    }

    // TODO: check if host exists and can be used, otherwise throw BadParameter
    // easiest would probably to run an invalid job request and see if we get
    // a sensible error...  But latency *sigh*


    if ( idata->init_from_jobid_ )
    {
      jobid_ = idata->jobid_;
      size_t start_pos = jobid_.find("]-[");

      if ( start_pos == std::string::npos )
        SAGA_ADAPTOR_THROW("Invalid JobId", saga::BadParameter);

      std::string epr_str = jobid_.substr(start_pos + 3, jobid_.size() - start_pos - 4 );

      std::cout << "EPR = " << epr_str << std::endl;
      job_epr_ = bp_.get_job_handle(epr_str);
      
      // TODO: fill job description from the jobs jsdl
      // 
      // we successfully inited from job id -- store job description
      // idata->jd_ = jd_;
    }
    else
    {
      try
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
      catch ( const char * m )
      {
        SAGA_ADAPTOR_THROW ((std::string ("Could not create jsdl: ") + m).c_str (), 
                            saga::BadParameter);
      }
      catch ( const saga::exception & e )
      {
        SAGA_ADAPTOR_THROW ((std::string ("Could not create jsdl: ") + e.what ()).c_str (), 
                            saga::BadParameter);
      }
    }


    // FIXME: register metrics etc.
  }


  // destructor
  job_cpi_impl::~job_cpi_impl (void)
  {
    try 
    {
      bp_.finalize ();
    }
    catch ( const char * m )
    {
      SAGA_ADAPTOR_THROW ((std::string ("Could not finalize backend library: ") + m).c_str (), 
                          saga::NoSuccess);
    }
    catch ( const saga::exception & e )
    {
      SAGA_ADAPTOR_THROW ((std::string ("Could not finalize backend library: ") + e.what ()).c_str (), 
                          saga::NoSuccess);
    }
  }


  //  SAGA API functions
  void job_cpi_impl::sync_get_state (saga::job::state & ret)
  {
    try 
    {
      adaptor_data_type adata (this);

      hpcbp::combined_state cs = bp_.get_state (job_epr_);

      state_ = adata->get_saga_state (cs);

      saga::adaptors::attribute jobattr (this);

      // FIXME: need to set metric, not attribute.  How?
      // jobattr.set_attribute (saga::job::attributes::substate, adata->get_saga_substate (cs));

      // std::cout << "  substate   : " << adata->get_saga_substate (cs) << std::endl;
    }
    catch ( const char * m )
    {
      SAGA_ADAPTOR_THROW ((std::string ("Could not get state: ") + m).c_str (), 
                          saga::NoSuccess);
    }
    catch ( const saga::exception & e )
    {
      SAGA_ADAPTOR_THROW ((std::string ("Could not get state: ") + e.what ()).c_str (), 
                          saga::NoSuccess);
    }

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

    try
    {
      // jsdl_.dump ();

      job_epr_ = bp_.run (jsdl_);

      std::string s1 (rm_s_);
      std::string s2 (::strdup (job_epr_->str));

      // we filter <Metadata>...</Metadata> out of s1 and s2 (Hi Genesis-II :-)
      size_t pos1 = s1.find ("<Metadata",   0);
      size_t pos2 = s1.find ("</Metadata>", 0);
      size_t pos3 = s2.find ("<Metadata",   0);
      size_t pos4 = s2.find ("</Metadata>", 0);

      if ( pos1 != std::string::npos && 
           pos2 != std::string::npos &&
           pos2 > pos1 )
      {
        s1.erase (pos1, pos2 - pos1 + 10);
      }

      if ( pos3 != std::string::npos && 
           pos4 != std::string::npos &&
           pos4 > pos3 )
      {
        s2.erase (pos3, pos4 - pos3 + 10);
      }

      // construct te job od according to GFD.90
      jobid_ = std::string ("[") + s1 + "]-[" + s2 + "]";


      // job accepted by the system - assume running
      state_ = saga::job::Running; 

      // std::cout << "Successfully submitted activity: " << jobid_ << std::endl;
    }
    catch ( const char * m )
    {
      SAGA_ADAPTOR_THROW ((std::string ("Could not run job: ") + m).c_str (), 
                          saga::NoSuccess);
    }
    catch ( const saga::exception & e )
    {
      SAGA_ADAPTOR_THROW ((std::string ("Could not run job: ") + e.what ()).c_str (), 
                          saga::NoSuccess);
    }
  }

  void job_cpi_impl::sync_cancel (saga::impl::void_t & ret, 
                                  double timeout)
  {
    try
    {
      bp_.terminate (job_epr_);
    }
    catch ( const char * m )
    {
      SAGA_ADAPTOR_THROW ((std::string ("Could not cancel job: ") + m).c_str (), 
                          saga::NoSuccess);
    }
    catch ( const saga::exception & e )
    {
      SAGA_ADAPTOR_THROW ((std::string ("Could not cancel job: ") + e.what ()).c_str (), 
                          saga::NoSuccess);
    }
  }

  //  wait for the child process to terminate
  void job_cpi_impl::sync_wait (bool   & ret, 
                                double   timeout)
  {
    try
    {
      adaptor_data_type adata (this);
      double time = 0.0;

      while ( time < timeout ) 
      {
        adata->get_saga_state (bp_.get_state (job_epr_));

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
    catch ( const char * m )
    {
      SAGA_ADAPTOR_THROW ((std::string ("Could not wait for job: ") + m).c_str (), 
                          saga::NoSuccess);
    }
    catch ( const saga::exception & e )
    {
      SAGA_ADAPTOR_THROW ((std::string ("Could not wait for job: ") + e.what ()).c_str (), 
                          saga::NoSuccess);
    }
  }

  // TODO: add state polling and metrics support

} // namespace bes_hpcbp_job
////////////////////////////////////////////////////////////////////////

