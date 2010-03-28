//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

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
#include "ogf_bes_job_service.hpp"


////////////////////////////////////////////////////////////////////////
namespace ogf_bes_job
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
  {
    instance_data idata (this);

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

    // FIXME: check if host exists and can be used, otherwise throw
    // BadParameter.  Easiest would probably to run an invalid job 
    // request and see if we get a sensible error...
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
    // TODO: check how that is supported in BES
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

} // namespace ogf_bes_job
////////////////////////////////////////////////////////////////////////

