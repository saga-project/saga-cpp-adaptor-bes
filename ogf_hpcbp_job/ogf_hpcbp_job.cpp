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
#include "ogf_bes_job.hpp"

////////////////////////////////////////////////////////////////////////
namespace ogf_bes_job
{
  // TODO: should be exposed from bes.h, really
  struct bes_epr 
  {
    char *str;
    struct soap_dom_element *dom;
    int domCreateFlag;
  };
  

  // constructor
  job_cpi_impl::job_cpi_impl (proxy                           * p, 
                              cpi_info const                  & info,
                              saga::ini::ini const            & glob_ini, 
                              saga::ini::ini const            & adap_ini,
                              TR1::shared_ptr <saga::adaptor>   adaptor)
    : base_cpi  (p, info, adaptor, cpi::Noflags)
    , session_  (p->get_session ())
  {
    instance_data     idata (this);
    adaptor_data_type adata (this);

    rm_ = idata->rm_;

    // check if URL is usable
    if ( ! rm_.get_scheme ().empty ()    &&
           rm_.get_scheme () != "bes"    && 
           rm_.get_scheme () != "http"   && 
           rm_.get_scheme () != "https"  && 
           rm_.get_scheme () != "any"    )
    {
      SAGA_OSSTREAM strm;
      strm << "Could not initialize job service for [" << rm_ << "]. " 
           << "Only these schemas are supported: any://, bes://, http(s)://, or none.";

      SAGA_ADAPTOR_THROW (SAGA_OSSTREAM_GETSTRING (strm), 
                          saga::adaptors::AdaptorDeclined);
    }
    
    if ( rm_.get_scheme () == "any" ||
         rm_.get_scheme () == "bes" )
    {
      rm_.set_scheme ("https");
    }

    std::stringstream ss;
    ss << "<?xml version=\"1.0\"  encoding=\"UTF-8\"?>\n"
       << " <wsa:EndpointReference xmlns:wsa=\"http://www.w3.org/2005/08/addressing\">\n"
       << "  <wsa:Address>" << rm_.get_string () << "</wsa:Address>\n"
       << " </wsa:EndpointReference>\n";

    host_epr_s_ = ss.str ();


    // init bes context
    if ( bes_init (&bes_context_) )
    {
      SAGA_ADAPTOR_THROW ("Cannot init BES context",
                          saga::NoSuccess);
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
        if ( c.get_attribute  (saga::attributes::context_type) == "X509" )
        {
          std::string user_s;
          std::string pass_s;

          if ( c.attribute_exists (saga::attributes::context_userid) )
          {
            user_s = c.get_attribute (saga::attributes::context_userid);
          }

          if ( c.attribute_exists (saga::attributes::context_userpass) )
          {
            pass_s = c.get_attribute (saga::attributes::context_userpass);
          }

          char * user_cs = ::strdup (user_s.c_str ());
          char * pass_cs = ::strdup (pass_s.c_str ());

          if ( bes_security (bes_context_, NULL, NULL, NULL, user_cs, pass_cs) ) 
          {
            ::free (user_cs);
            ::free (pass_cs);

            SAGA_ADAPTOR_THROW (bes_get_lasterror (bes_context_),
                                saga::NoSuccess);
          }

          ::free (user_cs);
          ::free (pass_cs);

          context_found = true;
        }
        else if ( c.get_attribute (saga::attributes::context_type) == "UserPass" )
        {
          std::string cert_s;
          std::string pass_s;
          std::string cadir_s;

          if ( c.attribute_exists (saga::attributes::context_certrepository) )
          {
            cadir_s = c.get_attribute (saga::attributes::context_certrepository);
          }

          if ( c.attribute_exists (saga::attributes::context_usercert) )
          {
            cert_s = c.get_attribute (saga::attributes::context_usercert);
          }

          if ( c.attribute_exists (saga::attributes::context_userpass) )
          {
            pass_s = c.get_attribute (saga::attributes::context_userpass);
          }

          char * cert_cs  = ::strdup (cert_s.c_str ());
          char * pass_cs  = ::strdup (pass_s.c_str ());
          char * cadir_cs = ::strdup (cadir_s.c_str ());


          if ( bes_security (bes_context_, cert_cs, pass_cs, cadir_cs, NULL, NULL) )
          {
            ::free (cert_cs);
            ::free (pass_cs);
            ::free (cadir_cs);

            SAGA_ADAPTOR_THROW (bes_get_lasterror (bes_context_),
                                saga::NoSuccess);
          }

          ::free (cert_cs);
          ::free (pass_cs);
          ::free (cadir_cs);

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
      SAGA_LOG_WARN ("No suitable security context found");

      // in anyway, we need to initialize the bes_context anyway
      if ( bes_security (bes_context_, NULL, NULL, NULL, NULL, NULL) )
      {
        SAGA_ADAPTOR_THROW (bes_get_lasterror (bes_context_),
                            saga::NoSuccess);
      }
    }


    // create epr from epr string
    char * host_epr_cs = ::strdup (host_epr_s_.c_str ());

    if ( bes_readEPRFromString (bes_context_, host_epr_cs, &host_epr_) ) 
    {
      SAGA_ADAPTOR_THROW ("Cannot convert rm URL into endpoint EPR", 
                          saga::BadParameter);
    }

    ::free (host_epr_cs);

    // TODO: check if host exists and can be used, otherwise throw BadParameter
    // easiest would probably to run an invalid job request and see if we get
    // a sensible error...


    if ( idata->init_from_jobid_ )
    {
      jobid_ = idata->jobid_;

      // TODO: confirm that the job exists on the host (get state)
      // TODO: fill job description from the jobs jsdl

      // we successfully inited from job id -- store job description
      idata->jd_ = jd_;
    }
    else
    {
      // init from job description
      jd_ = idata->jd_;
      
      if ( ! jd_.attribute_exists (saga::job::attributes::description_executable) )
      {
        SAGA_ADAPTOR_THROW ("job description misses executable",
                            saga::BadParameter);
      }

      std::string exe = jd_.get_attribute (saga::job::attributes::description_executable);


      std::vector <std::string> args;
      if ( jd_.attribute_exists (saga::job::attributes::description_arguments) )
      {
        args = jd_.get_vector_attribute (saga::job::attributes::description_arguments);
      }


      // TODO: build jsdl from job description
      std::stringstream ss;

      ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
         << "<JobDefinition xmlns=\"http://schemas.ggf.org/jsdl/2005/11/jsdl\">\n"
         << "    <JobDescription>\n"
         << "        <JobIdentification>\n"
         << "            <JobName>Sleep</JobName>\n"
         << "        </JobIdentification>\n"
         << "        <Application>\n"
         << "            <HPCProfileApplication xmlns=\"http://schemas.ggf.org/jsdl/2006/07/jsdl-hpcpa\">\n"
         << "                <Executable>" << exe << "</Executable>\n";

      for ( unsigned int i = 0; i < args.size ();  i++ )
      {
         ss << "                <Argument>" << args[i] << "</Argument>\n";
      }
      ss << "                <Output>/dev/null</Output>\n"
         << "                <WorkingDirectory>/tmp</WorkingDirectory>\n"
         << "            </HPCProfileApplication>\n"
         << "        </Application>\n"
         << "        <Resources>\n"
         << "            <TotalCPUCount>\n"
         << "                <Exact>1</Exact>\n"
         << "            </TotalCPUCount>\n"
         << "        </Resources>\n"
         << "    </JobDescription>\n"
         << "</JobDefinition>\n";

      jsdl_ = ss.str ();
    }


    // FIXME: register metrics etc.
  }


  // destructor
  job_cpi_impl::~job_cpi_impl (void)
  {
    bes_freeEPR (&activity_epr_);
    bes_freeEPR (&host_epr_);
  }


  //  SAGA API functions
  void job_cpi_impl::sync_get_state (saga::job::state & ret)
  {
    // TODO
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_cpi_impl::sync_get_description (saga::job::description & ret)
  {
    ret = jd_;
  }

  void job_cpi_impl::sync_get_job_id (std::string & ret)
  {
    if ( ! job_epr_s_.empty () )
    {
      std::string s ("");
      s += "[" + rm_.get_string () + "]-[" + job_epr_s_ + "]";
      ret = s;
    }
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
    //  FIXME: need a valid jsdl here!
    char* jsdl_cs = ::strdup (jsdl_.c_str ());

    if ( bes_createActivityFromString (bes_context_, host_epr_, jsdl_cs, &activity_epr_ )) 
    {
      SAGA_ADAPTOR_THROW (bes_get_lasterror (bes_context_), saga::NoSuccess);
    }

    ::free (jsdl_cs);

    struct bes_epr * tmp = (struct bes_epr *) activity_epr_;
    job_epr_s_ = ::strdup (tmp->str);

    std::cout << "Successfully submitted activity: " << job_epr_s_ << std::endl;
  }

  void job_cpi_impl::sync_cancel (saga::impl::void_t & ret, 
                                  double timeout)
  {
    // TODO
  }

  //  wait for the child process to terminate
  void job_cpi_impl::sync_wait (bool   & ret, 
                                double   timeout)
  {
    // TODO
  }

  // TODO: add state polling and metrics support

} // namespace ogf_bes_job
////////////////////////////////////////////////////////////////////////

