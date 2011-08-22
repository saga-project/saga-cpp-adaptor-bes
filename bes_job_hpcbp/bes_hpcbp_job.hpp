
//  Copyright (c) 2009-2011 Andre Merzky <andre@merzky.net>
//  Distributed under the GPLv.2 - see accompanying LICENSE file.


#ifndef ADAPTORS_OGF_HPCBP_JOB_HPP
#define ADAPTORS_OGF_HPCBP_JOB_HPP

// stl includes
#include <string>

// saga includes
#include <saga/saga.hpp>
#include <saga/saga/adaptors/task.hpp>

// saga engine includes
#include <saga/impl/engine/proxy.hpp>

// saga adaptor includes
#include <saga/saga/adaptors/adaptor.hpp>
#include <saga/saga/adaptors/task.hpp>
#include <saga/saga/adaptors/adaptor_data.hpp>

// job package includes
#include <saga/impl/packages/job/job_cpi.hpp>

// adaptor includes
#include "bes_hpcbp_job_adaptor.hpp"

// hpcbp includes
#include <hpcbp.hpp>

////////////////////////////////////////////////////////////////////////
namespace bes_hpcbp_job
{
  class job_cpi_impl 
    : public saga::adaptors::v1_0::job_cpi <job_cpi_impl>
  {
    private:
      typedef saga::adaptors::v1_0::job_cpi <job_cpi_impl> base_cpi;

      // adaptor data
      typedef saga::adaptors::adaptor_data <adaptor> adaptor_data_type;

      saga::session           session_;
      saga::url               rm_;
      std::string             rm_s_;
      saga::job::description  jd_;
      saga::job::state        state_;
      std::string             jobid_;

      hpcbp::connector        bp_;
      hpcbp::job_description  jsdl_;
      hpcbp::job_handle       job_epr_;


    public:
      // constructor of the job adaptor
      job_cpi_impl  (proxy                           * p, 
                     cpi_info const                  & info,
                     saga::ini::ini const            & glob_ini, 
                     saga::ini::ini const            & adap_ini,
                     TR1::shared_ptr <saga::adaptor>   adaptor);

      // destructor of the job adaptor
      ~job_cpi_impl (void);

      // job functions
      void sync_get_state       (saga::job::state       & ret);
      void sync_get_description (saga::job::description & ret);
      void sync_get_job_id      (std::string            & ret);

      void sync_get_stdin       (saga::job::ostream     & ret);
      void sync_get_stdout      (saga::job::istream     & ret);
      void sync_get_stderr      (saga::job::istream     & ret);

      void sync_checkpoint      (saga::impl::void_t     & ret);
      void sync_migrate         (saga::impl::void_t     & ret,
                                 saga::job::description   jd);
      void sync_signal          (saga::impl::void_t     & ret, 
                                 int                      signal);

      // inherited from saga::task
      void sync_run             (saga::impl::void_t & ret);
      void sync_cancel          (saga::impl::void_t & ret, 
                                 double               timeout);
      void sync_suspend         (saga::impl::void_t & ret);
      void sync_resume          (saga::impl::void_t & ret);

      void sync_wait            (bool         & ret, 
                                 double         timeout);
  };  // class job_cpi_impl

} // namespace bes_hpcbp_job
////////////////////////////////////////////////////////////////////////

#endif // ADAPTORS_OGF_HPCBP_JOB_HPP

