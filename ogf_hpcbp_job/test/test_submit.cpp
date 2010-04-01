
#include <saga/saga.hpp>

int main (int argc, char** argv)
{
  try
  {
    saga::session s;
    saga::context c ("UserPass");

    c.set_attribute (saga::attributes::context_userid,   "merzky");
    c.set_attribute (saga::attributes::context_userpass, "aaa");
    c.set_attribute (saga::attributes::context_certrepository, 
                     "/Users/merzky/links/saga/adaptors/ogf/trunk/external/bes++/besserver/cert");

    s.add_context (c);

    saga::job::service js (s, "bes://localhost:1235");
    saga::job::job j = js.run_job ("/bin/sleep 10");
    
    // j.wait (-1.0);

    saga::job::state state = j.get_state ();

    while ( state == saga::job::Running )
    {
      std::cout << "Running" << std::endl;
      ::sleep (1);
      state = j.get_state ();
    }

    std::cout << "Done" << std::endl;
  }
  catch ( const saga::exception & e )
  {
    std::cout << "Exception: " << e.what () << std::endl;
  }
  catch ( const char * m )
  {
    std::cout << "Exception: " << m << std::endl;
  }
}

