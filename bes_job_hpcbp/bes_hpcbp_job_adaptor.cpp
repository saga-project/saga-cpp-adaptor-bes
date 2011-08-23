
//  Copyright (c) 2009-2011 Andre Merzky <andre@merzky.net>
//  Distributed under the GPLv.2 - see accompanying LICENSE file.

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga adaptor includes
#include <saga/saga/adaptors/config.hpp>
#include <saga/saga/adaptors/adaptor.hpp>

// adaptor includes
#include "bes_hpcbp_job_adaptor.hpp"
#include "bes_hpcbp_job_service.hpp"
#include "bes_hpcbp_job.hpp"

SAGA_ADAPTOR_REGISTER (bes_hpcbp_job::adaptor);


////////////////////////////////////////////////////////////////////////
namespace bes_hpcbp_job
{
  // register function for the SAGA engine
  saga::impl::adaptor_selector::adaptor_info_list_type
      adaptor::adaptor_register (saga::impl::session * s)
  {
    // list of implemented cpi's
    saga::impl::adaptor_selector::adaptor_info_list_type list;

    // create empty preference list
    // these list should be filled with properties of the adaptor, 
    // which can be used to select adaptors with specific preferences.
    // Example:
    //   'security' -> 'gsi'
    //   'logging'  -> 'yes'
    //   'auditing' -> 'no'
    preference_type prefs; 

    // create file adaptor infos (each adaptor instance gets its own uuid)
    // and add cpi_infos to list
    job_service_cpi_impl::register_cpi (list, prefs, adaptor_uuid_);
    job_cpi_impl::register_cpi         (list, prefs, adaptor_uuid_);

    // and return list
    return (list);
  }

} // namespace bes_hpcbp_job
////////////////////////////////////////////////////////////////////////

