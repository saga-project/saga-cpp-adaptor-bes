/* ----------------------------------------------------------------
 *
 * This is a modified version of the bes++ besclient.c program, with changes
 * which reflect a SAGA adaptor structure.  If this client works against
 * a specific BES backend, the SAGA adaptor should work, too.
 *
 * bes_sagatest.c
 *   
 *      Client implementation of the OGSA Basic Execution Services
 *
 * Copyright (C) 2006-2009, Platform Computing Corporation. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "hpcbp.hpp"

int main (int argc, char * argv[])
{

  try 
  {
    char * cadir        = "/home/merzky/.saga/certificates/";
    char * cert         = "/tmp/x509up_u501";
    char * key          = "/tmp/x509up_u501";
    char * user         = "ogf";
    char * pass         = "ogf";
    char * jsdl         = "sleep.jsdl";

    hpcbp_connector bp (cert, key, cadir, user, pass);
    bp.set_host_endpoint ("https://zam1161v01.zam.kfa-juelich.de:8002/DEMO-SITE/services/BESFactory?res=default_bes_factory");

    // char * capath       = "../besserver/cert/";
    // char * x509cert     = "../besserver/cert/arc-user-cert.pem";
    // char * x509pass     = "../besserver/cert/arc-user-key.pem";
    // char * user         = NULL;
    // char * pass         = NULL;

    // hpcbp_connector bp (x509cert, x509pass, capath, user, pass);
    // bp.set_host_endpoint ("https://interop.grid.niif.hu:2010/arex-x509");

    hpcbp_job_description jd;

    jd.set_job_name ("Sleep Name");

    jd.set_job_name          ("SleepName");
    jd.set_job_annotation    ("SleepAnnotation");
    jd.set_job_project       ("SleepProject");
    jd.set_total_cpu_count   (1);
    jd.set_executable        ("/bin/sleep");
    jd.set_output            ("/dev/null");
    jd.set_working_directory ("/tmp/");

    std::vector <std::string> args;
    args.push_back ("10");
    jd.set_args (args); 

    hpcbp_job_handle job_epr = bp.run_job (jd);

    std::cout << job_epr->str << std::endl;

    while ( true ) 
    {
      hpcbp_state state = bp.get_state (job_epr);

      if ( state == HPCBP_Cancelled ||
           state == HPCBP_Failed    ||
           state == HPCBP_Finished  )
      {
        std::cout << "final state" << std::endl;
        break;
      }

      std::cout << "." << std::flush;
      ::sleep (1);
    }

    std::cout << "--" << std::endl;
  }
  catch ( const char * e )
  {
    std::cerr << "exception: " << e << std::endl;
    return -1;
  }

  return 0;
}

