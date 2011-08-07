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
    std::string capath   = "/Users/merzky/links/saga/adaptors/ogf/trunk/external/bes++/besserver/cert";
    std::string x509cert = "";
    std::string x509pass = "";
    std::string user     = "merzky";
    std::string pass     = "aaa";

    hpcbp::connector bp;
    bp.set_security      (x509cert, x509pass, capath, user, pass);
    bp.set_host_endpoint ("https://localhost:1235");

    hpcbp::job_description jd;

    jd.set_job_name          ("SleepName");
    jd.set_job_annotation    ("SleepAnnotation");
    jd.set_job_project       ("SleepProject");
    jd.set_total_cpu_count   ("1");
    jd.set_executable        ("/bin/sleep");
    jd.set_output            ("/dev/null");
    jd.set_working_directory ("/tmp/");

    std::vector <std::string> args;
    args.push_back ("10");
    jd.set_args (args); 

    hpcbp::job_handle job_epr = bp.run (jd);

    std::cout << job_epr->str << std::endl;

    while ( true ) 
    {
      hpcbp::combined_state cs = bp.get_state (job_epr);

      if ( cs.state == hpcbp::Canceled  ||
           cs.state == hpcbp::Failed    ||
           cs.state == hpcbp::Finished  )
      {
        break;
      }

      std::cout << "." << std::flush;
      ::sleep (1);
    }

    std::cout << std::endl;
  }
  catch ( const char * e )
  {
    std::cerr << "exception: " << e << std::endl;
    return -1;
  }

  return 0;
}

