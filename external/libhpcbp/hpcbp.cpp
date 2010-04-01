
#include "hpcbp.hpp"


namespace hpcbp
{
  job_description::job_description (void)
    : jsdl_ (NULL)
  {
    jsdl_ = new struct jsdl_job_definition;

    if ( jsdl_newJobDefinition (JSDL_HPC_PROFILE_APPLICATION, &jsdl_) )
    {
      throw "Canot create jd";
    }
  }

  job_description::~job_description (void)
  {
  }

  struct jsdl_job_definition * job_description::get_jsdl (void) const
  {
    return jsdl_;
  }

  struct jsdl_hpcp_application * job_description::get_app (void)
  {
    return (struct jsdl_hpcp_application *) jsdl_->Application;
  }

  void job_description::set_job_name (std::string s)
  {
    // FIXME: leaking!
    jsdl_->JobName = ::strdup (s.c_str ());
  }

  void job_description::set_job_annotation (std::string s)
  {
    jsdl_->JobAnnotation = ::strdup (s.c_str ());
  }

  void job_description::set_job_project (std::string s)
  {
    jsdl_->JobProject = ::strdup (s.c_str ());
  }

  void job_description::set_total_cpu_count (std::string s)
  {
    int n = ::atoi (s.c_str ());

    struct jsdl_range_value * cpucount;

    if ( jsdl_newRangeValue (&cpucount) )
    {
      throw "Can't allocate RangeValue";
    }

    if (jsdl_addExact (cpucount, n, 0.0) )
    {
      throw "Can't add Exact to RangeValue";
    }

    jsdl_->TotalCPUCount = cpucount;  
  }

  void job_description::set_executable (std::string s)
  {
    get_app ()->Executable = ::strdup (s.c_str ());
  }

  void job_description::set_input (std::string s)
  {
    get_app ()->Input = ::strdup (s.c_str ());
  }

  void job_description::set_output (std::string s)
  {
    get_app ()->Output = ::strdup (s.c_str ());
  }

  void job_description::set_error (std::string s)
  {
    get_app ()->Error = ::strdup (s.c_str ());
  }

  void job_description::set_working_directory (std::string s)
  {
    get_app ()->WorkingDirectory = ::strdup (s.c_str ());
  }

  void job_description::set_args (std::vector <std::string> args)
  {
    char ** c_args = NULL;

    if ( args.size () > 0 )
    {
      c_args = (char**) calloc (args.size (), sizeof (char*));

      for ( unsigned int i = 0; i < args.size (); i++ )
      {
        c_args[i] = ::strdup (args[i].c_str ());
      }
    }

    get_app ()->num_args = args.size ();
    get_app ()->Argument = c_args;
  }

  void connector::init_security_ (void)
  {
    char * x509cert;
    char * x509pass;
    char * capath;
    char * user;
    char * pass;

    if ( x509cert_.empty () ) x509cert = NULL; else x509cert = ::strdup (x509cert_.c_str ());
    if ( x509pass_.empty () ) x509pass = NULL; else x509pass = ::strdup (x509pass_.c_str ());
    if ( capath_.empty   () ) capath   = NULL; else capath   = ::strdup (capath_.c_str   ());
    if ( user_.empty     () ) user     = NULL; else user     = ::strdup (user_.c_str     ());
    if ( pass_.empty     () ) pass     = NULL; else pass     = ::strdup (pass_.c_str     ());

    if ( bes_security (bes_context_, x509cert, x509pass, capath, user, pass) )
    {
      std::cout << bes_get_lasterror (bes_context_) << std::endl;
      throw bes_get_lasterror (bes_context_);
    }

    std::cout << "bes security initialized" << std::endl;
  }


  connector::connector (void)
    : bes_context_ (NULL)
    , host_epr_    (NULL)
  {
    if ( bes_init (&bes_context_) )
    {
      throw ("Cannot init bes context");
    }

    std::cout << "bes connector initialized" << std::endl;

  }

  connector::~connector (void)
  {
    bes_freeEPR  (&host_epr_);
    bes_finalize (&bes_context_);
    std::cout << "bes connector finalized" << std::endl;
  }


  void connector::set_security (std::string x509cert, std::string x509pass, std::string capath, 
                                std::string user,     std::string pass)
  {
    x509cert_ = x509cert;
    x509pass_ = x509pass;
    capath_   = capath;
    user_     = user;
    pass_     = pass;
  }


  void connector::set_host_endpoint (const std::string host)
  {
    host_ = strdup (host.c_str ());

    std::stringstream endpoint_ss;
    endpoint_ss << "<?xml version=\"1.0\"  encoding=\"UTF-8\"?>\n"
      << " <wsa:EndpointReference xmlns:wsa=\"http://www.w3.org/2005/08/addressing\">\n"
      << "  <wsa:Address>" << host_ << "</wsa:Address>\n"
      << " </wsa:EndpointReference>\n";

    char * endpoint_cs = ::strdup (endpoint_ss.str ().c_str ());

    if ( bes_initEPRFromString (bes_context_, endpoint_cs, &host_epr_) )
    {
      // Cannot initialize bes endpoint
      std::cout << bes_get_lasterror (bes_context_) << std::endl;
      throw (bes_get_lasterror (bes_context_));
    }

    std::cout << "host epr points to " << host_ << std::endl;
  }

  job_handle connector::run_job (const job_description & jd)
  {
    init_security_ ();

    epr_t epr;

    if ( bes_createActivity (bes_context_, host_epr_, jd.get_jsdl (), &epr) )
    {
      std::cout << bes_get_lasterror (bes_context_) << std::endl;
      throw bes_get_lasterror (bes_context_);
    }

    job_handle job_epr = (job_handle) epr;

    // std::cout << job_epr->str << std::endl;

    // FIXME: epr is leaking memory here... - should be wrapped in
    // separate class
    return job_epr;
  }

  state connector::get_state (job_handle job_epr)
  {
    init_security_ (); 

    struct bes_activity_status status;

    if ( bes_getActivityStatuses (bes_context_, host_epr_, job_epr, &status) )
    {
      std::cout << bes_get_lasterror (bes_context_) << std::endl;
      throw (bes_get_lasterror (bes_context_));
    }

    // FIXME: set state and substate attribs FIXME: run thread which
    // monitors job state asynchronously, and which sets attribs and
    // metrics
    return (state) status.state; 
  }

} // namespace hpcbp

