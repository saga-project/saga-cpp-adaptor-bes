
#include <saga/saga.hpp>

// -------------------------------
//              sec  sleep   saga
// -------------------------------
// local.fork     x      x      x
// local.bes      x      x      x 
// ssh            x      x      x
// graml.loni     x      x      x
// graml.lrz      x      x      x
// graml.lrz      x      x      x
// unicore        -      -      - SF
// gridsam        -      -      -
// arc            x      x      x
// smoa.1         x      x      x
// smoa.2         x      x      x
// fg.sierra      x      x      x SF
// fg.india       x      x      x SF
// fg.alamo       x      x      x SF
// fg.sierra.uc   -      x      - 
// fg.india.uc    -      x      -
// ec2            -      -      -
// -------------------------------
// occi           -      -      -
// -------------------------------

namespace sja = saga::job::attributes;

#ifdef SAGA_APPLE
# define HOME "/Users/merzky/"
# define UID  "503"
#else
# define HOME "/home/merzky/"
# define UID  "1000"
#endif


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
      exe      = HOME "/saga/install/bin/saga-run.sh";
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
      cadir    = HOME "/.saga/certificates/";
      exe      = HOME "/saga/install/bin/saga-run.sh";
    }
};


class endpoint_ssh_cyder : public endpoint
{
  public:
    endpoint_ssh_cyder (void)
    {
      type     = "ssh";
      url      = "ssh://cyder.cct.lsu.edu/";
      user     = "amerzky";
      pass     = "";
      cert     = "";
      key      = "";
      cadir    = HOME ".saga/certificates/";
      exe      = "/home/amerzky/install/bin/saga-run.sh";
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
      cert     = "/tmp/x509up_u" UID;
      key      = "/tmp/x509up_u" UID;
      cadir    = HOME ".saga/certificates/";
      exe      = "/home/unicoreinterop/install/bin/saga-run.sh";
    }
};


class endpoint_gridsam : public endpoint
{
  public:
    endpoint_gridsam (void)
    {
      type     = "UserPass";
   // url      = "https://demo.oerc.ox.ac.uk:8443/gridsam/services/hpcbp";
   // url      = "https://gridsam-test.oerc.ox.ac.uk:18443/gridsam/services/hpcbp";
      url      = "https://localhost:10004";
      user     = "ogf";
      pass     = "ogf";
      cert     = "/tmp/x509up_u" UID;
      key      = "/tmp/x509up_u" UID;
      cadir    = HOME ".saga/certificates/";
      exe      = "/home/amerzky/install/bin/saga-run.sh";
    }
};


class endpoint_gram_loni : public endpoint
{
  public:
    endpoint_gram_loni (void)
    {
      type     = "x509";
      url      = "gram://qb1.loni.org/";
      user     = "";
      pass     = "";
      cert     = "/tmp/x509up_u" UID;
      key      = "/tmp/x509up_u" UID;
      cadir    = HOME ".globus/certificates/";
      exe      = "/home/merzky/install/bin/saga-run.sh";
    }
};


class endpoint_gram_lrz : public endpoint
{
  public:
    endpoint_gram_lrz (void)
    {
      type     = "x509";
      url      = "gram://lxgt2.lrz-muenchen.de/";
      user     = "";
      pass     = "";
      cert     = "/tmp/x509up_u" UID;
      key      = "/tmp/x509up_u" UID;
      cadir    = HOME ".globus/certificates/";
      exe      = "/home/cluster/pr95ho/lu26kov/install/bin/saga-run.sh";
    }
};


class endpoint_gram_unido : public endpoint
{
  public:
    endpoint_gram_unido (void)
    {
      type     = "x509";
      url      = "gram://udo-gt01.grid.tu-dortmund.de/";
      user     = "";
      pass     = "";
      cert     = "/tmp/x509up_u" UID;
      key      = "/tmp/x509up_u" UID;
      cadir    = HOME ".globus/certificates/";
      exe      = "/home/hp/hp0250/install/bin/saga-run.sh";
    }
};


class endpoint_arc : public endpoint
{
  public:
    endpoint_arc (void)
    {
      type     = "UserPass";
      url      = "https://interop.grid.niif.hu:60000/arex-ut";
   // url      = "https://localhost:10001/arex-ut";
      user     = "ogf30";
      pass     = "ogf30";
      cert     = "/tmp/x509up_u" UID;
      key      = "/tmp/x509up_u" UID;
      cadir    = HOME ".saga/certificates/";
      exe      = "/usr/local/saga/bin/saga-run.sh";
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
    }
};

class endpoint_smoa_2 : public endpoint
{
  public:
    endpoint_smoa_2 (void)
    {
      type     = "x509";
      url      = "https://grass1.man.poznan.pl:19022";
   // url      = "https://localhost:10005";
      user     = "";
      pass     = "";
      cert     = "/tmp/x509up_u" UID;
      key      = "/tmp/x509up_u" UID;
      cadir    = HOME ".saga/certificates/";
      exe      = "/home/ogf/install/bin/saga-run.sh";
      pwd      = "/home/ogf/";
    }
};

class endpoint_fg_sierra_u : public endpoint
{
  public:
    endpoint_fg_sierra_u (void)
    {
      type     = "UserPass";
      url      = "epr://localhost/" HOME ".saga/eprs/fg.sierra.short.epr";
      user     = "ogf30";
      pass     = "ogf30";
      cert     = "/tmp/x509up_u" UID;
      key      = "/tmp/x509up_u" UID;
      cadir    = HOME ".saga/certificates/";
      exe      = "/N/u/merzky/install/bin/saga-run.sh";
    }
};


class endpoint_fg_sierra_c : public endpoint
{
  public:
    endpoint_fg_sierra_c (void)
    {
      type     = "x509";
      url      = "epr://localhost/" HOME ".saga/eprs/fg.sierra.short.epr";
      user     = "";
      pass     = "";
      cert     = "/tmp/x509up_u" UID;
      key      = "/tmp/x509up_u" UID;
      cadir    = HOME ".saga/certificates/";
      exe      = "/N/u/merzky/install/bin/saga-run.sh";
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
      cert     = "/tmp/x509up_u" UID;
      key      = "/tmp/x509up_u" UID;
      cadir    = HOME ".saga/certificates/";
      exe      = "/N/u/merzky/install/bin/saga-run.sh";
    }
};


class endpoint_fg_india_u : public endpoint
{
  public:
    endpoint_fg_india_u (void)
    {
      type     = "UserPass";
      url      = "epr://localhost/" HOME ".saga/eprs/fg.india.short.epr";
      user     = "ogf30";
      pass     = "ogf30";
      cert     = "/tmp/x509up_u" UID;
      key      = "/tmp/x509up_u" UID;
      cadir    = HOME ".saga/certificates/";
      exe      = "/N/u/merzky/install/bin/saga-run.sh";
    }
};


class endpoint_fg_india_c : public endpoint
{
  public:
    endpoint_fg_india_c (void)
    {
      type     = "x509";
      url      = "epr://localhost/" HOME ".saga/eprs/fg.india.short.epr";
      user     = "";
      pass     = "";
      cert     = "/tmp/x509up_u" UID;
      key      = "/tmp/x509up_u" UID;
      cadir    = HOME ".saga/certificates/";
      exe      = "/N/u/merzky/install/bin/saga-run.sh";
    }
};


class endpoint_fg_alamo_u : public endpoint
{
  public:
    endpoint_fg_alamo_u (void)
    {
      type     = "UserPass";
      url      = "epr://localhost/" HOME ".saga/eprs/fg.alamo.short.epr";
      user     = "ogf30";
      pass     = "ogf30";
      cert     = "/tmp/x509up_u" UID;
      key      = "/tmp/x509up_u" UID;
      cadir    = HOME ".saga/certificates/";
      exe      = "/N/soft/SAGA/saga/1.6/gcc-4.1.2/bin/saga-run.sh";
      exe      = "/does/not/exist";
    }
};


class endpoint_fg_alamo_c : public endpoint
{
  public:
    endpoint_fg_alamo_c (void)
    {
      type     = "x509";
      url      = "epr://localhost/" HOME ".saga/eprs/fg.alamo.short.epr";
      user     = "";
      pass     = "";
      cert     = "/tmp/x509up_u" UID;
      key      = "/tmp/x509up_u" UID;
      cadir    = HOME ".saga/certificates/";
      exe      = "/N/soft/SAGA/saga/1.6/gcc-4.1.2/bin/saga-run.sh";
    }
};



class endpoint_fg_ucindia : public endpoint
{
  public:
    endpoint_fg_ucindia (void)
    {
      type     = "UserPass";
      url      = "https://198.202.120.85:8080/DEMO-SITE/services/BESFactory?res=default_bes_factory";
   // url      = "https://localhost:10003/DEMO-SITE/services/BESFactory?res=default_bes_factory";
      user     = "ogf30";
      pass     = "ogf30";
      cert     = "/tmp/x509up_u" UID;
      key      = "/tmp/x509up_u" UID;
      cadir    = HOME ".saga/certificates/";
      exe      = "/N/u/merzky/install/bin/saga-run.sh";
    }
};


class endpoint_ec2host : public endpoint
{
  public:
    endpoint_ec2host (void)
    {
      type     = "ec2";
      url      = "ec2://ec2-50-16-45-213.compute-1.amazonaws.com";
      user     = "root";
      pass     = "";
      cert     = "";
      key      = "";
      cadir    = HOME ".saga/certificates/";
      exe      = "/usr/local/saga/bin/saga-run.sh";
    }
};


int run_test (std::string       name, 
              struct endpoint & ep)
{
  time_t      t   = ::time      (NULL);
  struct tm * tmp = ::localtime (&t);

  char tstr[256];
  strftime (tstr, sizeof (tstr), "%F.%T(%Z)", tmp);

  std::string ad  = "advert://SAGA:SAGA_client@cyder.cct.lsu.edu:8080/test/bes_test";
  std::string key = "endpoint";
  std::string val = name + "-" + tstr;
  
  std::vector <std::string> args;
  args.push_back ("saga-advert");
  args.push_back ("set_attribute");
  args.push_back (ad);
  args.push_back (key);
  args.push_back (val);

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
    jd.set_vector_attribute (sja::description_arguments,  args);

    std::vector <std::string> file_transfers;

    // file_transfers.push_back ("http://www.google.com/index.html >  /tmp/test.dat");
    // file_transfers.push_back ("doh://www.google.com/index.html >  /tmp/test.dat");
    // file_transfers.push_back ("file:///bin/sleep > test.dat");
    // file_transfers.push_back ("http://host.1/data/1.dat >  1.dat");
    // file_transfers.push_back ("http://host.2/data/2.dat >> 2.dat");
    // file_transfers.push_back ("http://host.3/data/3.dat <  3.dat");
    // file_transfers.push_back ("http://host.4/data/4.dat << 4.dat");
    // file_transfers.push_back ("userpass @ http://host.1/data/1.dat >  1.dat");
    // file_transfers.push_back ("userpass @ http://host.2/data/2.dat >> 2.dat");
    // file_transfers.push_back ("userpass @ http://host.3/data/3.dat <  3.dat");
    // file_transfers.push_back ("userpass @ http://host.4/data/4.dat << 4.dat");

    jd.set_vector_attribute (sja::description_file_transfer,  file_transfers);

    // std::cout << " command       : " << ep.exe;
    // for ( unsigned int i = 0; i < ep.args.size (); i++ )
    // {
    //   std::cout << " " << ep.args[i];
    // }
    // std::cout << std::endl;

    if ( ep.pwd != "" )
    {
      jd.set_attribute (sja::description_working_directory, ep.pwd);
    }

    saga::job::job j = js.create_job (jd);

    j.run ();

    std::cout << j.get_job_id () << std::endl;

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

      saga::advert::entry a (ad);
      std::string result = a.get_attribute (key);

      if ( result == val )
      {
        std::cout << name << ": Done with Success (" << val << ")" << std::endl;
      }
      else
      {
        std::cout << name << ": Done with Failure (" << result << " != " << val << ")" << std::endl;
      }
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
    std::cout << " ssh.cyder  " << std::endl;
    std::cout << " unicore    " << std::endl;
    std::cout << " gridsam    " << std::endl;
    std::cout << " gram.loni  " << std::endl;
    std::cout << " gram.lrz   " << std::endl;
    std::cout << " gram.unido " << std::endl;
    std::cout << " arc        " << std::endl;
    std::cout << " smoa.1     " << std::endl;
    std::cout << " smoa.2     " << std::endl;
    std::cout << " fg.sierra.u" << std::endl;
    std::cout << " fg.sierra.c" << std::endl;
    std::cout << " fg.india.u " << std::endl;
    std::cout << " fg.india.c " << std::endl;
    std::cout << " fg.alamo.u " << std::endl;
    std::cout << " fg.alamo.c " << std::endl;
    std::cout << " fg.ucsierra" << std::endl;
    std::cout << " fg.ucindia " << std::endl;
    std::cout << " ec2host    " << std::endl;
    std::cout << "            " << std::endl;

    return 0;
  }
  
  
  bool all = ( test == "all" ) ? true : false ;


  if ( all || test == "local.fork" ) { struct endpoint_localfork      ep; run_test ("local.fork" , ep) && err++; tot++; }
  if ( all || test == "local.bes"  ) { struct endpoint_localbes       ep; run_test ("local.bes"  , ep) && err++; tot++; }
  if ( all || test == "ssh.cyder"  ) { struct endpoint_ssh_cyder      ep; run_test ("ssh.cyder"  , ep) && err++; tot++; }
  if ( all || test == "unicore"    ) { struct endpoint_unicore        ep; run_test ("unicore"    , ep) && err++; tot++; } 
  if ( all || test == "gridsam"    ) { struct endpoint_gridsam        ep; run_test ("gridsam"    , ep) && err++; tot++; } 
  if ( all || test == "gram.loni"  ) { struct endpoint_gram_loni      ep; run_test ("gram.loni"  , ep) && err++; tot++; } 
  if ( all || test == "gram.lrz"   ) { struct endpoint_gram_lrz       ep; run_test ("gram.lrz"   , ep) && err++; tot++; } 
  if ( all || test == "gram.unido" ) { struct endpoint_gram_unido     ep; run_test ("gram.unido" , ep) && err++; tot++; } 
  if ( all || test == "arc"        ) { struct endpoint_arc            ep; run_test ("arc"        , ep) && err++; tot++; } 
  if ( all || test == "smoa.1"     ) { struct endpoint_smoa_1         ep; run_test ("smoa.1"     , ep) && err++; tot++; } 
  if ( all || test == "smoa.2"     ) { struct endpoint_smoa_2         ep; run_test ("smoa.2"     , ep) && err++; tot++; } 
  if ( all || test == "fg.sierra.u") { struct endpoint_fg_sierra_u    ep; run_test ("fg.sierra.u", ep) && err++; tot++; }
  if ( all || test == "fg.sierra.c") { struct endpoint_fg_sierra_c    ep; run_test ("fg.sierra.c", ep) && err++; tot++; }
  if ( all || test == "fg.india.u" ) { struct endpoint_fg_india_u     ep; run_test ("fg.india.u" , ep) && err++; tot++; }
  if ( all || test == "fg.india.c" ) { struct endpoint_fg_india_c     ep; run_test ("fg.india.c" , ep) && err++; tot++; }
  if ( all || test == "fg.alamo.u" ) { struct endpoint_fg_alamo_u     ep; run_test ("fg.alamo.u" , ep) && err++; tot++; }
  if ( all || test == "fg.alamo.c" ) { struct endpoint_fg_alamo_c     ep; run_test ("fg.alamo.c" , ep) && err++; tot++; }
  if ( all || test == "fg.ucsierra") { struct endpoint_fg_ucsierra    ep; run_test ("fg.ucsierra", ep) && err++; tot++; }
  if ( all || test == "fg.ucindia" ) { struct endpoint_fg_ucindia     ep; run_test ("fg.ucindia" , ep) && err++; tot++; }
  if ( all || test == "ec2host"    ) { struct endpoint_ec2host        ep; run_test ("ec2host"    , ep) && err++; tot++; }


  std::cout << " ==================================================================" << std::endl;
  std::cout << " tests succeeded: " << tot - err << std::endl;
  std::cout << " tests failed   : " <<       err << std::endl;
  std::cout << " ==================================================================" << std::endl;

  return -err;
}

