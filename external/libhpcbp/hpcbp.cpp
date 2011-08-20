
//  Copyright (c) 2009-2011 Andre Merzky <andre@merzky.net>
//  Distributed under the GPLv.2 - see accompanying LICENSE file.

#include "hpcbp.hpp"

#include <regex.h>


namespace hpcbp
{
  job_description::job_description (void)
    : jsdl_ (NULL)
  {
    jsdl_ = new struct jsdl_job_definition;

    if ( jsdl_newJobDefinition (JSDL_HPC_PROFILE_APPLICATION, &jsdl_) )
    {
      throw "Cannot create jd";
    }

    jsdl_->DataStaging = NULL;
  }

  job_description::~job_description (void)
  {
    if ( jsdl_ != NULL )
    {
      jsdl_freeJobDefinition (jsdl_);
      jsdl_ = NULL;
    }
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

  // struct jsdl_data_staging 
  // {
  //   struct jsdl_data_staging * next;
  //   char                     * name;
  //   char                     * FileName;
  //   char                     * FileSystemName;
  //   enum jsdl_creation_flags   CreationFlag;
  //   int                        DeleteOnTermination;
  //   char                     * Source;
  //   char                     * Target;
  //   struct hpcp_credential   * Credential;
  // };
  void job_description::set_file_transfers (std::vector <std::string> specs)
  {
    // "           http://host.one/data/one >> ftp://host.two/stage/two"
    // "UserPass @ http://host.one/data/one >  ftp://host.two/stage/two"
    std::string spec_pattern = "^(([^ ]+) +@ +)?([^ ><]+) *(>|>>|<|<<) *([^ ><]+)$";
    regex_t     regex;

    if ( 0 != regcomp (&regex, spec_pattern.c_str (), REG_EXTENDED) )
    {
      throw "regcomp() failed"; // FIXME errno
    }


    for ( unsigned int i = 0; i < specs.size (); i++ )
    {
      std::string spec   = specs[i];
      size_t      nmatch = 6;
      regmatch_t  pmatch[6];

      std::cout << "parsing '" << spec << "' against '" << spec_pattern << "'" << std::endl;

      if ( 0 != regexec (&regex, spec.c_str (), nmatch, pmatch, 0) )
      {
        throw "regexec() failed"; // FIXME errno
      }

      std::vector <std::string> matches;
      matches.resize (6, "");

      for ( unsigned int j = 0; j < nmatch; j++ )
      {
        if ( pmatch[j].rm_so != -1 )
        {
          for ( int k = pmatch[j].rm_so; k < pmatch[j].rm_eo; k++ )
          {
            matches[j] += spec[k];
          }
        }

        std::cout << " spec match " << j << " : " << matches[j] << std::endl;
      }

      std::string  fname ("");
      std::string  fsys  ("");
      std::string  src   ("");
      std::string  tgt   (""); 
      std::string  ctx   ("");
      staging_flag flag;
      bool         cleanup = false;

      std::cout << "matches: " << matches.size () << std::endl;

      // 0 1 2                   3              4              5
      // "^( ([^\\s]+)\\s+@\\s+)?([^\\><s]+)\\s*(>|>>|<|<<)\\s*([^\\><s]+)$";

      ctx = matches[2];

      if ( matches[4] == ">"  ||
           matches[4] == ">>" )
      {
        // stage in
        src   = matches[3];
        fname = matches[5];

        if ( matches[4] == ">"  ) { flag = Overwrite; }
        else                      { flag = Append;    }
      }
      else if ( matches[4] == "<"  || 
                matches[4] == "<<" )
      {
        // stage out
        tgt   = matches[3];
        fname = matches[5];

        if ( matches[4] == "<"  ) { flag = Overwrite; }
        else                      { flag = Append;    }
      }
      else
      {
        throw "invalid file transfer operation"; // FIXME: details
      }

      struct jsdl_data_staging * file = new struct jsdl_data_staging;

      file->next                = NULL;
      file->name                = NULL;
      file->FileName            = NULL;
      file->FileSystemName      = NULL;
      file->CreationFlag        = jsdl_nocreationflag;
      file->DeleteOnTermination = 0;
      file->Source              = NULL;
      file->Target              = NULL;
      file->Credential          = NULL;


      if ( ! fname.empty () )
      {
        file->FileName = ::strdup (fname.c_str ());
        if ( file->FileName == NULL )
        {	
          throw "strdup error";
        }
      }

      if ( ! fsys.empty () )
      {
        file->FileSystemName = ::strdup (fsys.c_str ());
        if ( file->FileSystemName == NULL )
        {	
          throw "strdup error";
        }
      }

      if      ( flag & Overwrite     ) { file->CreationFlag        = jsdl_overwrite;     }
      else if ( flag & Append        ) { file->CreationFlag        = jsdl_append;        }
      else if ( flag & DontOverwrite ) { file->CreationFlag        = jsdl_dontOverwrite; }

      if ( cleanup ) 
      { 
        file->DeleteOnTermination = 1;
      }

      if ( ! src.empty () )
      {
        file->Source = ::strdup (src.c_str ());
        if ( file->Source == NULL )
        {
          throw "strdup error";
        }
      }

      if ( ! tgt.empty () )
      {
        file->Target = ::strdup (tgt.c_str ());
        if ( file->Target == NULL )
        {
          throw "strdup error";
        }
      }

      if ( ! ctx.empty () )
      {
        std::cout <<  "no credential support for file staging, yet: " << ctx << std::endl;
        // throw "no credential support for file staging, yet";
        // struct hpcp_credential * cred = NULL;
        // if ( (rc = jsdl_processCredential(cur, &cred)) != BESE_OK )
        // {
        //   jsdl_freeDataStaging(file);
        //   return rc;
        // }
        // file->Credential = cred;
      }

      struct jsdl_data_staging * cur_file = jsdl_->DataStaging;
      if ( cur_file ) 
      {
        while ( cur_file->next ) 
        {
          cur_file = cur_file->next;
        }
        cur_file->next = file;
      }
      else 
      {
        jsdl_->DataStaging = file;
      }
    }

    regfree (&regex); // FIXME: we should re-use the compiled regex

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

  void job_description::dump (void)
  {
    jsdl_printJobDefinition (jsdl_);
  }


  //////////////////////////////////////////////////////////////////////
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
      std::cerr << bes_get_lasterror (bes_context_) << std::endl;
      throw bes_get_lasterror (bes_context_);
    }

    // std::cout << "bes security initialized:" << std::endl;
    // std::cout << "  x509cert : " << (x509cert ? x509cert : "NULL") << std::endl;
    // std::cout << "  x509pass : " << (x509pass ? x509pass : "NULL") << std::endl;
    // std::cout << "  capath   : " << (capath   ? capath   : "NULL") << std::endl;
    // std::cout << "  user     : " << (user     ? user     : "NULL") << std::endl;
    // std::cout << "  pass     : " << (pass     ? pass     : "NULL") << std::endl;

  }


  connector::connector (void)
    : bes_context_ (NULL)
    , host_epr_    (NULL)
  {
    if ( bes_init (&bes_context_) )
    {
      throw ("Cannot init bes context");
    }

    // std::cout << "bes connector initialized" << std::endl;

  }

  connector::~connector (void)
  {
    bes_freeEPR  (&host_epr_);
    bes_finalize (&bes_context_);
    // std::cout << "bes connector finalized" << std::endl;
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


  void connector::set_host_epr (const std::string epr)
  {
    // std::cout << " ---------------- host epr ------------- " << std::endl;
    // std::cout << epr << std::endl;
    // std::cout << " --------------------------------------- " << std::endl;

    char * endpoint_cs = ::strdup (epr.c_str ());

    if ( bes_initEPRFromString (bes_context_, endpoint_cs, &host_epr_) )
    {
      // Cannot initialize bes endpoint
      std::cerr << bes_get_lasterror (bes_context_) << std::endl;
      throw (bes_get_lasterror (bes_context_));
    }

    // std::cout << "epr init ok" << std::endl;
  }

  void connector::set_host_endpoint (const std::string host)
  {
    host_ = strdup (host.c_str ());

    std::stringstream endpoint_ss;
    endpoint_ss << "<?xml version=\"1.0\"  encoding=\"UTF-8\"?>\n"
      << " <wsa:EndpointReference xmlns:wsa=\"http://www.w3.org/2005/08/addressing\">\n"
      << "  <wsa:Address>" << host_ << "</wsa:Address>\n"
      << " </wsa:EndpointReference>\n";

    // std::cout << " ---------------- host epr ------------- " << std::endl;
    // std::cout << endpoint_ss.str () << std::endl;
    // std::cout << " --------------------------------------- " << std::endl;

    char * endpoint_cs = ::strdup (endpoint_ss.str ().c_str ());

    if ( bes_initEPRFromString (bes_context_, endpoint_cs, &host_epr_) )
    {
      // Cannot initialize bes endpoint
      std::cerr << bes_get_lasterror (bes_context_) << std::endl;
      throw (bes_get_lasterror (bes_context_));
    }

    // std::cout << "host epr points to " << host_ << std::endl;
  }

  job_handle connector::run (const job_description & jd)
  {
    init_security_ ();

    epr_t epr;

    if ( bes_createActivity (bes_context_, host_epr_, jd.get_jsdl (), &epr) )
    {
      std::cerr << bes_get_lasterror (bes_context_) << std::endl;
      throw bes_get_lasterror (bes_context_);
    }

    job_handle job_epr = (job_handle) epr;

    // std::cout << "job epr: " << job_epr->str << std::endl;

    // FIXME: epr is leaking memory here... - should be wrapped in
    // separate class
    return job_epr;
  }

  void connector::terminate (job_handle & job_epr)
  {
    init_security_ ();

    if ( bes_terminateActivities (bes_context_, host_epr_, job_epr) )
    {
      std::cerr << bes_get_lasterror (bes_context_) << std::endl;
      throw bes_get_lasterror (bes_context_);
    }

    return;
  }

  combined_state connector::get_state (job_handle & job_epr)
  {
    init_security_ (); 

    struct bes_activity_status status;

    if ( bes_getActivityStatuses (bes_context_, host_epr_, job_epr, &status) )
    {
      std::cerr << bes_get_lasterror (bes_context_) << std::endl;
      throw (bes_get_lasterror (bes_context_));
    }

    combined_state cs;

    cs.state    = static_cast <state> (status.state);

    if ( NULL != status.substate )
    {
      cs.substate = status.substate;
    }
    else
    {
      cs.substate = "";
    }

    // FIXME: run thread which monitors job state asynchronously, 
    // and which sets attribs and metrics

    return cs;
  }

} // namespace hpcbp

