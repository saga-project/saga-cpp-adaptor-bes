
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <hpcbp.hpp>

#define USER  ""
#define PASS  ""
#define CERT  ""
#define KEY   ""
#define CADIR ""

int main (int argc, char** argv)
{

  if ( argc < 2 )
  {
    std::cerr << "\n\tUsage: test_bes <epr> <command>\n\n";
    exit (-1);
  }

  hpcbp::connector        bes;
  hpcbp::job_description  jsdl;
  hpcbp::job_handle       job_epr;


  try 
  {
    bes.initialize ();
  }
  catch ( const char * m )
  {
    std::cerr << "init oops: " << m << std::endl;
    ::exit (-1);
  }

  std::string epr = argv[1];

  int fd = ::open (epr.c_str (), O_RDONLY);
  if ( fd < 0 )
  {
    std::cerr << "epr oops: " << ::strerror (errno) << std::endl;
    exit (-1);
  }

  struct stat s;
  if ( 0 != ::fstat (fd, &s) )
  {
    std::cerr << "stat oops: " << ::strerror (errno) << std::endl;
    exit (-1);
  }

  char * epr_buf = new char (s.st_size + 1);
  if ( s.st_size != ::read (fd, epr_buf, s.st_size) )
  {
    std::cerr << "read oops: " << ::strerror (errno) << std::endl;
    exit (-1);
  }

  epr_buf[s.st_size] = '\0';

  ::close (fd);

  try 
  {
    bes.set_host_epr (static_cast <const char*> (epr_buf));
  }
  catch ( const char * m )
  {
    std::cerr << "bes epr oops: " << m << std::endl;
    exit (-1);
  }

  try 
  {
    bes.set_security (CERT, KEY, CADIR, USER, PASS);
  }
  catch ( const char * m )
  {
    std::cerr << "bes sec oops: " << m << std::endl;
    exit (-1);
  }

  try 
  {
    jsdl.set_executable (argv[2]);
  }
  catch ( const char * m )
  {
    std::cerr << "jsdl exe oops: " << m << std::endl;
    exit (-1);
  }

  try
  {
    jsdl.dump ();

    job_epr = bes.run (jsdl);

    std::string jobid (::strdup (job_epr->str));

    std::cout << "Successfully submitted activity: " << jobid << std::endl;
  }
  catch ( const char * m )
  {
    std::cerr << "bes run oops: " << m << std::endl;
    exit (-1);
  }

  return 0;
}


