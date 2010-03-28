Introduction
============

BES++ is an implementation of the OGSA Basic Execution Service (BES)
which is a standardized (http://www.ogf.org/documents/GFD.108.pdf)
SOAP-based Web services interface for submitting and managing jobs
running under the control of a resource manager. The BES++ code
was contributed by Platform Computing (http://www.platform.com) after
some HPC Basic Profile interoperability testing that occurred
during the SuperComputing conferences in 2006 and 2007.

The goals of the BES++ project are:

- Provide a reference implementation of OGSA-BES that is compliant
  with the HPC Basic Profile (http://www.ogf.org/documents/GFD.114.pdf).

- Provide this implementation as Open Source Software, in order to
  encourage the adoption of the standards.

- Become a vehicle for the development of extensions to the current
  specifications according to the needs of the HPC and Grid computing
  communities.

You can always access the latest release of BES++ from 
http://sourceforge.net/projects/bespp.


Building and Installing
=======================

In order to build the BES++ software you need to use the gSOAP Toolkit
(http://sourceforge.net/projects/gsoap2) to generate SOAP proxy code.
The version currently used for development is gsoap 2.7.10. Your mileage
may vary with other versions. Before building BES++, make sure and 
compile the gSOAP distribution (you don't actually need to install it).

Currently BES++ supports the Platform LSF resource manager, the Portable 
Batch System and the Sun Grid Engine. Make sure that LSF or PBS are 
installed on the machine on which you are building the BES++ code, as 
you will need access to the header files and libraries. This is not
necessary for SGE.

Building the software
---------------------

1. Edit config.h in the top level bespp directory to adjust any needed 
   parameters. See comments in the file for guidance on what to change.

2. Edit Make.config in the top level bespp directory and set:

   a) GSOAP_TOP to the top-level directory of the gSOAP distribution
      that you have built. 

   b) Adjust the LSF parameters to reflect the location of the LSF 
      distribution and to indicate the architecture that your are 
      building on. If you are not sure which architecture to use, and
      your environment is already setup to use LSF, you can run a 
      command like 'which lsid' to see which architecture specific 
      directory is appropriate for your machine.
      For PBS, the process is similar. For SGE, there are no variables
      to adjust except for RM. 

    c) If you need to set any extra paths to headers files or 
       libraries, use the EXTRA_* macros.  

3. Type 'make'. If all goes well, there will be a besclient binary
   in the besclient sub-directory, and a besserver binary in the 
   besserver sub-directory.

4. In order to interact with PBS and SGE, a Python installation is 
   required with the pexpect module (http://sourceforge.net/projects/pexpect/).
   This module can be downloaded and copied to the besserver/scripts 
   subdirectory.

Installing the software
-----------------------

If used with an LSF resource manager, you can copy the besclient and 
besserver binaries wherever you wish as they have no components which 
are required to be in any particular directory. For PBS and SGE, however
it is necessary that besserver can execute the Python scripts which are
located under the besserver/scripts subdirectory. The location for these 
scripts is defined in config.h.  

If you would like to support the use of user name and password authentication
using PAM, you will need to create a PAM configuration file and install
it into the appropriate place on your system ('man pam.conf' will usually
provide some information on the location ... usually /etc/pam.d). Two
sample pam.conf files are provided in the besserver/samples sub-directory.

You will need an X.509 certificate for the BES++ service. This can be
a self-signed certificate (see documentation from OpenSSL for information
about this topic) or a certificate from a valid certificate authority. Note
that the PEM formatted file used to contain the server certificate must
include both the certificate and the corresponding private key.


Running the besserver
=====================

Note: in order to run the besserver, it must be able to connect to
your LSF/PBS/SGE environment. The besserver can be run on any node in the
cluster (including a client-only machine). Make sure and have the 
environment variables properly initialized before starting the besserver.
This can be easily accomplished with a shell script wrapper. There is 
a sample wrapper in the besserver/samples sub-directory.

In most cases, the besserver must be started as the root user. This allows
the service to authenticate users using user name and password using 
PAM, and then to impersonate the user when submitting and controlling
jobs. The besserver can be run as a regular user, but this is only 
supported when: a) SSL mutual authentication is used, and b) all jobs
are submitted and managed as the user running the service.

besserver takes the following options:

-u service_user
    specifies under which user account the service will run.
    Required argument.

-h hostname
    the hostname of the service endpoint. This will also determine which
    interfaces the gSOAP stack will bind to for listening for requests.
    Default is 'localhost'.

-p port
    the port of the service endpoint.
    Default is 443.

-s server.pem
    the file containing the X.509 certificate and corresponding private
    key in PEM format for the service. 
    Default is './server.pem'.

-e service_endpoint
    a URL that indicates to clients how to connect to this service. By 
    default the endpoint is determined from the hostname and port, but
    in some cases (e.g. service is addressed on the internet using a
    CNAME alias) it is useful to override the default.
    Default is 'https://<hostname>:<port>'.

-c capath
    the directory in which CA certificates are kept for the purpose
    of authenticating clients via SSL. Each certificate should be 
    in a separate file, and should be named '<hash>.0', where <hash>
    is determined from the 'openssl x509 -hash' command.
    Default is './certs'.

-g generic_user
    specifies the user id under which jobs are run when authenticating
    clients via SSL

-V 
    the besserver prints out version information and then exits.

-? 
    the besserver prints out usage information and then exits.



Using the besclient
===================

The besclient program is used to make client requests to a BES service
(any BES compliant service, not just a BES++ service) in order to 
submit, control and monitor jobs, and to query the BES service 
attributes. 

Note that in BES an "Activity" is the same as a "Job" in LSF or other 
resource managers. 

A BES client will use SSL/TLS to connect to a BES service, and will use
SSL/TLS server authentication using X.509 certificates. This requires
the besclient user to set up a directory of "trusted" certificate files
(either actual server certificates, or trusted CAs). See the -capath 
option for more details on the contents of this directory.

The besclient program will support using either WS-Security's UsernameToken
for client authentication (i.e. username and password), or can authenticate
using X.509 certificates. The mechanism used will depend on the BES service 
being used.

There is a sample JSDL document in besclient/sleep.jsdl that should run 
on pretty much any UNIX type system through LSF if an example job is needed.

The basic command syntax of besclient is:

    besclient <options> <command> <command args>

The besclient supports the following commands:

create <JSDL document> <activity epr file>
    calls CreateActivity operation using the provided JSDL document
    as the input to the operation. On success, the returned activity
    EPR is placed in the file indicated by <activity epr file> for use
    in other operations.

status <activity epr file>
    calls GetActivityStatuses operation for the activity EPR that is
    contained in the provided file. The returned activity status is
    printed on stdout.

terminate <activity epr file>
    calls TerminateActivities operation for the activity EPR that is
    contained in the provided file. Success or failure of the operation
    is indicated on stdout.

activity <activity epr file>
    calls GetActivityDocuments operation for the activity EPR that is
    contained in the provided file. Information about the activity is
    printed to stdout.

factory
    calls GetFactoryAttributesDocument operation and prints information
    about the BES service on stdout.


besclient takes the following options:

-c capath
    the directory in which CA certificates are kept for the purpose
    of authenticating the BES service via SSL. Each certificate should 
    be in a separate file, and should be named '<hash>.0', where <hash>
    is determined from the 'openssl x509 -hash' command.
    Default is './certs'.

-e endpoint.xml
    the name of a file containing the EPR for the BES service to contact.
    Default is './endpoint.xml'. EPRs for different BES services will 
    vary slightly, so you will need to get this EPR from the provider
    of the BES service. The format of a BES++ BES service looks like
    the following:

    <?xml version="1.0"  encoding="UTF-8"?>
    <wsa:EndpointReference xmlns:wsa="http://www.w3.org/2005/08/addressing">
        <wsa:Address>https://myhost.mydomain.com:443</wsa:Address>
    </wsa:EndpointReference>

-u username
    the username to use when authenticating to the server using username
    and password authentication.
    One of -u or -x is required (not both).

-p password
    the password to use when authenticating to the server using username
    and password authentication.
    Default is to prompt for the password from stdin.

-x client.pem
    a file in PEM format containing the X.509 certificate and corresponding 
    private key that identifies the client to the BES service when 
    authenticating using SSL mutual authentication.
    One of -x or -u is required (not both).

-k keypassphrase
    the passphrase used to encrypt the client certificate.
    Default is to prompt for the key's passphrase from stdin.

-V 
    the besserver prints out version information and then exits.

-? 
    the besserver prints out usage information and then exits.


License
=======

BES++ is distributed under the terms of the GPL version 2 license.
See the file gpl.txt for a description of the terms and conditions
for copying, distribution and modification.


Copyright
=========

BES++ is copyrighted by Platform Computing Corp.
Copyright (C) 2007-2009 Platform Computing Corp.
All Rights Reserved

