//  Copyright (c) 2005-2007 Hartmut Kaiser 
//  Copyright (c) 2005-2007 Andre Merzky   (andre@merzky.net)
// 
//  Distributed under the Boost Software License, Version 1.0. 
//  (See accompanying file LICENSE or copy at 
//   http://www.boost.org/LICENSE_1_0.txt)

#ifndef ADAPTORS_OGF_HPCBP_JOB_ADAPTOR_HPP
#define ADAPTORS_OGF_HPCBP_JOB_ADAPTOR_HPP

// saga adaptor includes
#include <saga/saga/adaptors/adaptor.hpp>

// hpcbp includes
#include <hpcbp.hpp>

////////////////////////////////////////////////////////////////////////
namespace ogf_hpcbp_job
{
  struct adaptor : public saga::adaptor
  {
    typedef saga::impl::v1_0::op_info         op_info;  
    typedef saga::impl::v1_0::cpi_info        cpi_info;
    typedef saga::impl::v1_0::preference_type preference_type;

    // This function registers the adaptor with the factory
    // @param factory the factory where the adaptor registers
    //        its maker function and description table
    saga::impl::adaptor_selector::adaptor_info_list_type 
      adaptor_register (saga::impl::session * s);

    std::string get_name (void) const
    { 
      return BOOST_PP_STRINGIZE (SAGA_ADAPTOR_NAME);
    }

    saga::job::state get_saga_state (const hpcbp::state & s) const
    {
      switch ( s )
      {
        case hpcbp::Pending:
          return saga::job::New;
          break;

        case hpcbp::Running:
          return saga::job::Running;
          break;

        case hpcbp::Canceled:
          return saga::job::Canceled;
          break;

        case hpcbp::Failed:
          return saga::job::Failed;
          break;

        case hpcbp::Finished:
          return saga::job::Done;
          break;

        default:
          return saga::job::Unknown;
          break;
      }
      
      return saga::job::Unknown;
    }
  };

} // namespace ogf_hpcbp_job
////////////////////////////////////////////////////////////////////////

#endif // ADAPTORS_OGF_HPCBP_JOB_ADAPTOR_HPP

