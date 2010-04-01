
#include <saga/saga.hpp>

namespace sja = saga::job::attributes;

struct test
{
  std::string endpoint;
  std::string user;
  std::string pass;
  std::string cert;
  std::string key;
  std::string cadir;
  std::string exe;
};


class test_local : public test
{
  public:
    test_local (void)
    {
      endpoint = "bes://localhost:1235";
      user     = "merzky";
      pass     = "aaa";
      cert     = "";
      key      = "";
      cadir    = "./cert.local";
      exe      = "/usr/bin/uname";
    }
};


class test_gridsam : public test
{
  public:
    test_gridsam (void)
    {
      endpoint = "INVALID";
      user     = "";
      pass     = "";
      cert     = "";
      key      = "";
      cadir    = "./cert.gridsam";
      exe      = "/bin/uname";
    }
};


void run_test (std::string name, struct test & t)
{
  try
  {

    saga::session s;
    saga::context c ("UserPass");

    c.set_attribute (saga::attributes::context_userid,         t.user);
    c.set_attribute (saga::attributes::context_userpass,       t.pass);
    c.set_attribute (saga::attributes::context_certrepository, t.cadir);

    s.add_context (c);

    saga::job::service     js (s, t.endpoint);
    saga::job::description jd;

    std::string output ("output_");
    output += t.endpoint;
    jd.set_attribute (sja::description_executable, t.exe);
    jd.set_attribute (sja::description_output,     "output");

    
    std::vector <std::string> args;
    args.push_back ("-a");
    jd.set_vector_attribute (sja::description_arguments, args);


    // not supported, yet
    std::vector <std::string> transfers;
    args.push_back ("file://localhost/tmp/output < output");
    jd.set_vector_attribute (sja::description_file_transfer, transfers);


    saga::job::job j = js.create_job (jd);

    j.run ();
    
    // j.wait (-1.0);

    saga::job::state state = j.get_state ();

    while ( state == saga::job::Running )
    {
      std::cout << name << ": Running" << std::endl;
      ::sleep (1);
      state = j.get_state ();
    }

    std::cout << name << ": Done" << std::endl;
  }
  catch ( const saga::exception & e )
  {
    std::cout << name << ": Exception: " << e.what () << std::endl;
  }
  catch ( const char * m )
  {
    std::cout << name << ": exception: " << m << std::endl;
  }

  std::cout <<  "===========================================" << std::endl;
  return;
}

int main (int argc, char** argv)
{
  struct test_local   t_l;    run_test ("local",   t_l);
  struct test_gridsam t_gs;   run_test ("gridsam", t_gs);

  return 0;
}

