
#include <saga/saga.hpp>

// -------------------------------
//              sec  sleep   saga
// -------------------------------
// local.fork     x      x      x
// local.bes      x      x      x 
// ssh            x      x      x
// gram           x      x      x
// unicore        x      x      -
// gridsam        -      -      -
// arc            x      x      x
// smoa.1         x      x      x
// smoa.2         x      x      x
// fg.sierra      x      x      x
// fg.india       x      x      x
// fg.sierra.uc   x      x      -
// fg.india.uc    x      x      -
// -------------------------------
// ec2            -      -      -
// occi           -      -      -
// -------------------------------

namespace sja = saga::job::attributes;

struct endpoint
{
  std::string               type;
  std::string               url;
  std::string               user;
  std::string               pass;
  std::string               cert;
  std::string               key;
  std::string               cadir;
  std::string               exe;
  std::string               pwd;
  std::vector <std::string> args;
};

#ifdef SAGA_APPLE
# define HOME "/Users/merzky/"
#else
# define HOME "/home/merzky/"
#endif


class endpoint_localfork : public endpoint
{
  public:
    endpoint_localfork (void)
    {
      type     = "";
      url      = "fork://localhost/";
      user     = "";
      pass     = "";
      cert     = "";
      key      = "";
      cadir    = "";
      exe      = "/Users/merzky/install/bin/saga-run.sh";

      args.push_back ("saga-advert");
      args.push_back ("list");
      args.push_back ("/applications/");
    }
};


class endpoint_localbes : public endpoint
{
  public:
    endpoint_localbes (void)
    {
      type     = "UserPass";
      url      = "https://localhost:1235/";
      user     = "merzky";
      pass     = "aaa";
      cert     = "";
      key      = "";
      cadir    = HOME ".saga/certificates/";
      exe      = "/Users/merzky/install/bin/saga-run.sh";

      args.push_back ("saga-advert");
      args.push_back ("list");
      args.push_back ("/applications/");
    }
};


class endpoint_unicore : public endpoint
{
  public:
    endpoint_unicore (void)
    {
      type     = "UserPass";
      url      = "https://zam1161v01.zam.kfa-juelich.de:8002/DEMO-SITE/services/BESFactory?res=default_bes_factory";
   // url      = "https://localhost:10002";
      user     = "ogf";
      pass     = "ogf";
      cert     = "/tmp/x509up_u503";
      key      = "/tmp/x509up_u503";
      cadir    = HOME ".saga/certificates/";
      exe      = "/home/unicoreinterop/install/bin/saga-run.sh";

      args.push_back ("sleep");
      args.push_back ("10");
    }
};


class endpoint_gridsam : public endpoint
{
  public:
    endpoint_gridsam (void)
    {
      type     = "UserPass";
      url      = "https://gridsam-test.oerc.ox.ac.uk:18443/gridsam/services/hpcbp";
  //  url      = "https://localhost:10003";
      user     = "ogf";
      pass     = "ogf";
      cert     = "/tmp/x509up_u503";
      key      = "/tmp/x509up_u503";
      cadir    = HOME ".saga/certificates/";
      exe      = "/home/amerzky/install/bin/saga-run.sh";

      args.push_back ("saga-advert");
      args.push_back ("list");
      args.push_back ("/applications/");
    }
};


class endpoint_gram : public endpoint
{
  public:
    endpoint_gram (void)
    {
      type     = "x509";
      url      = "gram://qb1.loni.org/";
      user     = "";
      pass     = "";
      cert     = "/tmp/x509up_u503";
      key      = "/tmp/x509up_u503";
      cadir    = HOME ".globus/certificates/";
      exe      = "/home/merzky/install/bin/saga-run.sh";

      args.push_back ("saga-advert");
      args.push_back ("list");
      args.push_back ("/applications/");
    }
};


class endpoint_arc : public endpoint
{
  public:
    endpoint_arc (void)
    {
      type     = "UserPass";
   // url      = "https://interop.grid.niif.hu:60000/arex-ut";
      url      = "https://localhost:10001/arex-ut";
      user     = "ogf30";
      pass     = "ogf30";
      cert     = "/tmp/x509up_u503";
      key      = "/tmp/x509up_u503";
      cadir    = HOME ".saga/certificates/";
      exe      = "/usr/local/saga/bin/saga-run.sh";

      args.push_back ("saga-advert");
      args.push_back ("list");
      args.push_back ("/applications/");
    }
};

class endpoint_smoa_1 : public endpoint
{
  public:
    endpoint_smoa_1 (void)
    {
      type     = "UserPass";
      url      = "https://grass1.man.poznan.pl:19021";
   // url      = "https://localhost:10004";
      user     = "ogf";
      pass     = "smoa-project.org";
      cert     = "";
      key      = "";
      cadir    = HOME ".saga/certificates/";
      exe      = "/home/ogf/install/bin/saga-run.sh";
      pwd      = "/home/ogf/";

      args.push_back ("saga-advert");
      args.push_back ("list");
      args.push_back ("/applications/");
    }
};

class endpoint_smoa_k : public endpoint
{
  public:
    endpoint_smoa_k (void)
    {
      type     = "UserPass";
      url      = "https://grass1.man.poznan.pl:19021";
   // url      = "https://localhost:10004";
      user     = "ogf";
      pass     = "smoa-project.org";
      cert     = "";
      key      = "";
      cadir    = HOME ".saga/certificates/";
      exe      = "/usr/bin/killall";
      pwd      = "/home/ogf/";

      args.push_back ("-9");
      args.push_back ("mandelbrot_client");
    }
};

class endpoint_smoa_2 : public endpoint
{
  public:
    endpoint_smoa_2 (void)
    {
      type     = "UserPass";
      url      = "https://grass1.man.poznan.pl:19022";
   // url      = "https://localhost:10005";
      user     = "";
      pass     = "";
      cert     = "/tmp/x509up_u503";
      key      = "/tmp/x509up_u503";
      cadir    = HOME ".saga/certificates/";
      exe      = "/home/ogf/install/bin/saga-run.sh";
      pwd      = "/home/ogf/";

      args.push_back ("saga-advert");
      args.push_back ("list");
      args.push_back ("/applications/");
    }
};

class endpoint_fg_sierra : public endpoint
{
  public:
    endpoint_fg_sierra (void)
    {
      type     = "UserPass";
      url      = "epr://localhost/" HOME ".saga/fg.sierra.short.epr";
      user     = "ogf30";
      pass     = "ogf30";
      cert     = "/tmp/x509up_u503";
      key      = "/tmp/x509up_u503";
      cadir    = HOME ".saga/certificates/";
      exe      = "/N/u/merzky/install/bin/saga-run.sh";

      args.push_back ("saga-advert");
      args.push_back ("list");
      args.push_back ("/applications/");
    }
};


class endpoint_fg_ucsierra : public endpoint
{
  public:
    endpoint_fg_ucsierra (void)
    {
      type     = "UserPass";
   // url      = "https://198.202.120.85:8080/DEMO-SITE/services/BESFactory?res=default_bes_factory";
      url      = "https://localhost:10002/DEMO-SITE/services/BESFactory?res=default_bes_factory";
      user     = "ogf30";
      pass     = "ogf30";
      cert     = "/tmp/x509up_u503";
      key      = "/tmp/x509up_u503";
      cadir    = HOME ".saga/certificates/";
      exe      = "/N/u/merzky/install/bin/saga-run.sh";

      args.push_back ("saga-advert");
      args.push_back ("list");
      args.push_back ("/applications/");
    }
};


class endpoint_fg_india : public endpoint
{
  public:
    endpoint_fg_india (void)
    {
      type     = "UserPass";
      url      = "epr://localhost/" HOME ".saga/fg.india.short.epr";
      user     = "ogf30";
      pass     = "ogf30";
      cert     = "/tmp/x509up_u503";
      key      = "/tmp/x509up_u503";
      cadir    = HOME ".saga/certificates/";
      exe      = "/N/u/merzky/install/bin/saga-run.sh";

      args.push_back ("saga-advert");
      args.push_back ("list");
      args.push_back ("/applications/");
    }
};


class endpoint_fg_ucindia : public endpoint
{
  public:
    endpoint_fg_ucindia (void)
    {
      type     = "UserPass";
   // url      = "https://198.202.120.85:8080/DEMO-SITE/services/BESFactory?res=default_bes_factory";
      url      = "https://localhost:10003/DEMO-SITE/services/BESFactory?res=default_bes_factory";
      user     = "ogf30";
      pass     = "ogf30";
      cert     = "/tmp/x509up_u503";
      key      = "/tmp/x509up_u503";
      cadir    = HOME ".saga/certificates/";
      exe      = "/N/u/merzky/install/bin/saga-run.sh";

      args.push_back ("saga-advert");
      args.push_back ("list");
      args.push_back ("/applications/");
    }
};


int run_test (std::string       name, 
              struct endpoint & ep)
{
  int err = 0;

  try
  {
    std::cout <<  " ==================================================================" << std::endl;
    std::cout <<  " ----- " << name << " ----- " << ep.url << " ----- " << std::endl;
    std::cout <<  " ------------------------------------------------------------------" << std::endl;

    saga::session s;
    saga::context c (ep.type);

    // std::cout << " contexttype   : " << ep.type    << std::endl;
    // std::cout << " usercert      : " << ep.cert    << std::endl;
    // std::cout << " userkey       : " << ep.key     << std::endl;
    // std::cout << " userid        : " << ep.user    << std::endl;
    // std::cout << " userpass      : " << ep.pass    << std::endl;
    // std::cout << " certrepository: " << ep.cadir   << std::endl;

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

    if ( ep.pwd != "" )
    {
      jd.set_attribute (sja::description_working_directory, ep.pwd);
    }

    saga::job::job j = js.create_job (jd);

    j.run ();

    std::cout << name << ": Submitted" << std::endl;


    saga::job::state state = j.get_state ();

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
  int err = 0;
  int tot = 0; 

  std::string test = "list";

  if ( argc > 1 )
  {
    test = argv[1];
  }

  if ( test == "list" )
  {
    std::cout << "            " << std::endl;
    std::cout << " local.fork " << std::endl;
    std::cout << " local.bes  " << std::endl;
    std::cout << " unicore    " << std::endl;
    std::cout << " gridsam    " << std::endl;
    std::cout << " gram       " << std::endl;
    std::cout << " arc        " << std::endl;
    std::cout << " smoa.1     " << std::endl;
    std::cout << " smoa.k     " << std::endl;
    std::cout << " smoa.2     " << std::endl;
    std::cout << " fg.sierra  " << std::endl;
    std::cout << " fg.india   " << std::endl;
    std::cout << " fg.ucsierra" << std::endl;
    std::cout << " fg.ucindia " << std::endl;
    std::cout << "            " << std::endl;

    return 0;
  }
  
  
  bool all = ( test == "all" ) ? true : false ;


  if ( all || test == "local.fork" ) { struct endpoint_localfork   ep; run_test ("local fork   ", ep) && err++; tot++; }
  if ( all || test == "local.bes"  ) { struct endpoint_localbes    ep; run_test ("local bes    ", ep) && err++; tot++; }
  if ( all || test == "unicore"    ) { struct endpoint_unicore     ep; run_test ("unicore      ", ep) && err++; tot++; } 
  if ( all || test == "gridsam"    ) { struct endpoint_gridsam     ep; run_test ("gridsam      ", ep) && err++; tot++; } 
  if ( all || test == "gram"       ) { struct endpoint_gram        ep; run_test ("gram         ", ep) && err++; tot++; } 
  if ( all || test == "arc"        ) { struct endpoint_arc         ep; run_test ("arc          ", ep) && err++; tot++; } 
  if ( all || test == "smoa.1"     ) { struct endpoint_smoa_1      ep; run_test ("smoa 1       ", ep) && err++; tot++; } 
  if ( all || test == "smoa.k"     ) { struct endpoint_smoa_k      ep; run_test ("smoa k       ", ep) && err++; tot++; } 
  if ( all || test == "smoa.2"     ) { struct endpoint_smoa_2      ep; run_test ("smoa 2       ", ep) && err++; tot++; } 
  if ( all || test == "fg.sierra"  ) { struct endpoint_fg_sierra   ep; run_test ("fg sierra    ", ep) && err++; tot++; }
  if ( all || test == "fg.india"   ) { struct endpoint_fg_india    ep; run_test ("fg india     ", ep) && err++; tot++; }
  if ( all || test == "fg.ucsierra") { struct endpoint_fg_ucsierra ep; run_test ("fg uc sierra ", ep) && err++; tot++; }
  if ( all || test == "fg.ucindia" ) { struct endpoint_fg_ucindia  ep; run_test ("fg uc india  ", ep) && err++; tot++; }


  std::cout << " ==================================================================" << std::endl;
  std::cout << " tests succeeded: " << tot - err << std::endl;
  std::cout << " tests failed   : " <<       err << std::endl;
  std::cout << " ==================================================================" << std::endl;

  return -err;
}

