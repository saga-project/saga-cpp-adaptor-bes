#!/usr/bin/perl -w

BEGIN {
  use strict;
}

chdir ("$ENV{HOME}/.globus/certificates/") 
  || die "Cannot change to $ENV{HOME}/.globus/certificates/: $!\n";

foreach my $file ( sort (glob ("*.crl_url")) )
{
  my $crl =  $file;
     $crl =~ s/\.crl_url$/\.r0/io;
  my $url =  qx{cat $file};
  chomp $url;

  printf ("%-60s \t -> $crl\n", $url);

  system ("wget -q $url -O $crl &");
}

