
//  Copyright (c) 2009-2011 Andre Merzky <andre@merzky.net>
//  Distributed under the GPLv.2 - see accompanying LICENSE file.


// stl includes
#include <vector>
#include <sstream>

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga engine includes
#include <saga/impl/config.hpp>

// saga adaptor includes
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/attribute.hpp>

// saga package includes
#include <saga/saga/packages/job/adaptors/job.hpp>
#include <saga/saga/packages/job/adaptors/job_self.hpp>

// adaptor includes
#include "bes_hpcbp_job_service.hpp"


////////////////////////////////////////////////////////////////////////
namespace bes_hpcbp_job
{
  // TODO:
  //
  //  - all call should be locked on the bes ctx
  //

  // constructor
  job_service_cpi_impl::job_service_cpi_impl (proxy                * p, 
                                              cpi_info const       & info,
                                              saga::ini::ini const & glob_ini, 
                                              saga::ini::ini const & adap_ini,
                                              TR1::shared_ptr <saga::adaptor> adaptor)
    : base_cpi (p, info, adaptor, cpi::Noflags)
    , session_ (p->get_session ())
  {
    instance_data idata (this);

    rm_ = idata->rm_;

    // check if URL is usable
    if ( ! rm_.get_scheme ().empty ()    &&
           rm_.get_scheme () != "bes"    && 
           rm_.get_scheme () != "http"   && 
           rm_.get_scheme () != "https"  && 
           rm_.get_scheme () != "epr"    && // read epr from url
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
      catch ( const char & m )
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
      catch ( const char & m )
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
        if ( c.get_attribute  (saga::attributes::context_type) == "UserPass" )
        {
          std::string user;
          std::string pass;

          if ( c.attribute_exists (saga::attributes::context_userid) )
          {
            user = c.get_attribute (saga::attributes::context_userid);
          }

          if ( c.attribute_exists (saga::attributes::context_userpass) )
          {
            pass = c.get_attribute (saga::attributes::context_userpass);
          }

          bp_.set_security ("", "", "", user, pass);

          context_found = true;
        }
        else if ( c.get_attribute (saga::attributes::context_type) == "UserPass" )
        {
          std::string cert;
          std::string pass;
          std::string cadir;

          if ( c.attribute_exists (saga::attributes::context_certrepository) )
          {
            cadir = c.get_attribute (saga::attributes::context_certrepository);
          }

          if ( c.attribute_exists (saga::attributes::context_usercert) )
          {
            cert = c.get_attribute (saga::attributes::context_usercert);
          }

          if ( c.attribute_exists (saga::attributes::context_userpass) )
          {
            pass = c.get_attribute (saga::attributes::context_userpass);
          }

          bp_.set_security (cert, pass, cadir, "", "");

          context_found = true;
        }

        if ( context_found )
        {
          // TODO?: test if context can be used to contact server.  
          // If not, set context_found to false again, and free 
          // the bes context
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
    // a sensible error...  But latency *sigh*
  }

  // destructor
  job_service_cpi_impl::~job_service_cpi_impl (void)
  {
  }

  //////////////////////////////////////////////////////////////////////
  // SAGA API functions
  void job_service_cpi_impl::sync_create_job (saga::job::job         & ret, 
                                              saga::job::description   jd)
  {
    ret = saga::adaptors::job (rm_, jd, 
                               proxy_->get_session ());
  }

  void job_service_cpi_impl::sync_run_job (saga::job::job     & ret, 
                                           std::string          cmd, 
                                           std::string          host, 
                                           saga::job::ostream & in, 
                                           saga::job::istream & out, 
                                           saga::job::istream & err)
  {
    // rely on fallback adaptor to kick in    
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_service_cpi_impl::sync_list (std::vector <std::string> & ret)
  {
    // TODO: no idea how that is supported in BES
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

  void job_service_cpi_impl::sync_get_job (saga::job::job & ret, 
                                           std::string      jobid)
  {
    instance_data idata (this);

    // create job from jobid
    ret = saga::adaptors::job (idata->rm_,
                               jobid, 
                               proxy_->get_session ());
  }

  void job_service_cpi_impl::sync_get_self (saga::job::self & ret)
  {
    // this will in general not possible with BES, unless the 
    // fallback kicks in
    SAGA_ADAPTOR_THROW ("Not Implemented", saga::NotImplemented);
  }

} // namespace bes_hpcbp_job
////////////////////////////////////////////////////////////////////////

