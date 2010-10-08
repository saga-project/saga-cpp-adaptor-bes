
#include <saga/saga.hpp>

namespace sja = saga::job::attributes;

struct endpoint
{
  std::string               url;
  std::string               user;
  std::string               pass;
  std::string               cert;
  std::string               key;
  std::string               cadir;
  std::string               exe;
  std::vector <std::string> args;
};


class endpoint_local : public endpoint
{
  public:
    endpoint_local (void)
    {
      url      = "https://localhost:1235/";
      user     = "merzky";
      pass     = "aaa";
      cert     = "";
      key      = "";
      cadir    = "/Users/merzky/links/saga/adaptors/ogf/trunk/external/bes++/besserver/cert/";
      exe      = "/bin/sleep";

      args.push_back ("10");
    }
};


class endpoint_unicore : public endpoint
{
  public:
    endpoint_unicore (void)
    {
      url      = "https://zam1161v01.zam.kfa-juelich.de:8002/DEMO-SITE/services/BESFactory?res=default_bes_factory";
      user     = "ogf30";
      pass     = "ogf30";
      cert     = "/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/bes_client_cert.pem";
      key      = "/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/bes_client_cert.pem";
      cadir    = "/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/certificates/";
      exe      = "/bin/sleep";

      args.push_back ("10");
    }
};


class endpoint_gridsam : public endpoint
{
  public:
    endpoint_gridsam (void)
    {
      url      = "https://gridsam-test.oerc.ox.ac.uk:18443/gridsam/services/hpcbp";
      user     = "ogf30";
      pass     = "ogf30";
      cert     = "/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/bes_client_cert.pem";
      key      = "/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/bes_client_cert.pem";
      cadir    = "/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/certificates/";
      exe      = "/bin/sleep";

      args.push_back ("10");
    }
};


class endpoint_arc : public endpoint
{
  public:
    endpoint_arc (void)
    {
      url      = "https://interop.grid.niif.hu:2010/arex-x509";
      user     = "";
      pass     = "";
      cert     = "/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/arc-user-cert+key.pem";
      key      = "z1nfandel";
      cadir    = "/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/certificates/";
      exe      = "/bin/sleep";

      args.push_back ("10");
    }
};

class endpoint_smoa : public endpoint
{
  public:
    endpoint_smoa (void)
    {
      url      = "https://grass1.man.poznan.pl:19001";
      user     = "ogf30";
      pass     = "ogf30";
      cert     = "/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/bes_client_cert.pem";
      key      = "/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/bes_client_cert.pem";
      cadir    = "/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/certificates/";
      exe      = "/bin/sleep";

      args.push_back ("10");
    }
};

class endpoint_fg_sierra : public endpoint
{
  public:
    endpoint_fg_sierra (void)
    {
      url      = "epr://localhost/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/fg_sierra.epr";
      user     = "ogf30";
      pass     = "ogf30";
      cert     = "/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/bes_client_cert.pem";
      key      = "/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/bes_client_cert.pem";
      cadir    = "/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/certificates/";
      exe      = "/bin/sleep";

      args.push_back ("10");
    }
};


int run_test (std::string       name, 
              struct endpoint & ep)
{
  int err = 0;

  try
  {
    std::cout <<  " ==================================================================" << std::endl;
    std::cout <<  " ----- " << ep.url << " ----- " << std::endl;
    std::cout <<  " ------------------------------------------------------------------" << std::endl;

    saga::session s;
    saga::context c ("UserPass");

    c.set_attribute (saga::attributes::context_usercert,       ep.cert);
    c.set_attribute (saga::attributes::context_userkey,        ep.key);
    c.set_attribute (saga::attributes::context_userid,         ep.user);
    c.set_attribute (saga::attributes::context_userpass,       ep.pass);
    c.set_attribute (saga::attributes::context_certrepository, ep.cadir);

    s.add_context (c);

    saga::job::service     js (s, ep.url);
    saga::job::description jd;

    jd.set_attribute        (sja::description_executable, ep.exe);
    jd.set_vector_attribute (sja::description_arguments,  ep.args);

    std::string output ("file://localhost/tmp/output_");
    output += ep.url;
    jd.set_attribute (sja::description_output,     "output");

    // not supported, yet
    std::vector <std::string> transfers;
    transfers.push_back ("file://localhost/tmp/output < output");
    jd.set_vector_attribute (sja::description_file_transfer, transfers);


    saga::job::job j = js.create_job (jd);

    j.run ();

    std::cout << name << ": Submitted" << std::endl;
    
    // j.wait (-1.0);

    saga::job::state state = j.get_state ();

    while ( state == saga::job::New )
    {
      std::cout << name << ": New" << std::endl;
      ::sleep (1);
      state = j.get_state ();
    }

    while ( state == saga::job::Running )
    {
      std::cout << name << ": Running" << std::endl;
      ::sleep (1);
      state = j.get_state ();
    }

    if ( state == saga::job::Done )
    {
      std::cout << name << ": Done" << std::endl;
    }
    else
    {
      std::cout << name << ": Failed?" << std::endl;
      err++;
    }
  }
  catch ( const saga::exception & e )
  {
    std::cout << name << ": Exception: " << e.what () << std::endl;
    err++;
  }
  catch ( const char * m )
  {
    std::cout << name << ": exception: " << m << std::endl;
    err++;
  }

  std::cout <<  " ----- " << err << " ---------------------------------------------------------- " << std::endl;

  return err;
}

int main (int argc, char** argv)
{
  int err   = 0;
  int total = 0; 

  // struct endpoint_local     ep_l;   run_test ("local    ", ep_l)  && err++;  total++;
  // struct endpoint_unicore   ep_u;   run_test ("unicore  ", ep_u)  && err++;  total++;
  // struct endpoint_gridsam   ep_g;   run_test ("gridsam  ", ep_g)  && err++;  total++;
  // struct endpoint_arc       ep_a;   run_test ("arc      ", ep_a)  && err++;  total++;
  // struct endpoint_smoa      ep_s;   run_test ("smoa     ", ep_s)  && err++;  total++;
     struct endpoint_fg_sierra ep_f1;  run_test ("fg sierra", ep_f1) && err++;  total++;

  std::cout << " ==================================================================" << std::endl;
  std::cout << " tests succeeded: " << total - err << std::endl;
  std::cout << " tests failed   : " <<         err << std::endl;
  std::cout << " ==================================================================" << std::endl;

  return -err;
}

