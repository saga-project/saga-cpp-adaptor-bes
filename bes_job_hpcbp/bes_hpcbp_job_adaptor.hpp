
//  Copyright (c) 2009-2011 Andre Merzky <andre@merzky.net>
//  Distributed under the GPLv.2 - see accompanying LICENSE file.


#ifndef ADAPTORS_OGF_HPCBP_JOB_ADAPTOR_HPP
#define ADAPTORS_OGF_HPCBP_JOB_ADAPTOR_HPP

// saga adaptor includes
#include <saga/saga/adaptors/adaptor.hpp>

// hpcbp includes
#include <hpcbp.hpp>

////////////////////////////////////////////////////////////////////////
namespace bes_hpcbp_job
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

    saga::job::state get_saga_state (const hpcbp::combined_state & cs) const
    {
      switch ( cs.state )
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

    std::string get_saga_substate (const hpcbp::combined_state & cs) const
    {
      if ( cs.substate.empty () )
      {
        return "";
      }

      std::string ret;
      std::string tmp = cs.substate;

      std::vector <std::string> matches;
      std::vector <std::string> names;
      matches.push_back ("<State xmlns=\"http://www.nordugrid.org/schemas/a-rex\">");
      matches.push_back ("<State xmlns=\"http://schemas.ogf.org/glue/2008/05/spec_2.0_d41_r01\">");

      names.push_back ("ARC");
      names.push_back ("GLUE2");

      for ( unsigned int i = 0; i < matches.size (); i++ )
      {
        size_t p1 = 0;
        size_t p2 = 0;

        p1 = tmp.find (matches[i]);

        if ( p1 != std::string::npos )
        {
          p2 = tmp.find ("</State>", p1);

          if ( p2 != std::string::npos )
          {
            std::string substate = tmp.substr (p1+ matches[i].length (), p2 - (p1 + matches[i].length ())) + "";
            tmp.erase (p1, matches[i].length () + substate.length () + 8);

            if ( ! ret.empty () ) { ret += ','; }

            ret += names[i] + ':' + substate;
          }
        }
      }

      if ( ! tmp.empty () )
      {
        if ( ! ret.empty () ) { ret += ','; }
        ret += "UNKNOWN:" + tmp;
      }

      return ret;
    }
  };

} // namespace bes_hpcbp_job
////////////////////////////////////////////////////////////////////////

#endif // ADAPTORS_OGF_HPCBP_JOB_ADAPTOR_HPP

