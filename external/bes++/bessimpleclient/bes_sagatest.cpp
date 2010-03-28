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
    char * capath       = "../besserver/cert/";
    char * x509cert     = NULL;
    char * x509pass     = NULL;
    char * user         = "merzky";
    char * pass         = "aaa";
    char * jsdl         = "sleep.jsdl";

    hpcbp_connector bp (x509cert, x509pass, capath, user, pass);
    bp.set_host_endpoint ("https://localhost:1236");

    hpcbp_jd jd;


    struct bes_epr * job_epr = bp.run_job (jd);

    std::cout << job_epr->str << std::endl;

    while ( true ) 
    {
      hpcbp_state state = bp.get_state (job_epr);

      if ( state == HPCBP_Cancelled ||
           state == HPCBP_Failed    ||
           state == HPCBP_Finished  )
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

