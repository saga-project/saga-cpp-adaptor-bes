#!/bin/sh

have_pem=false
cadir=$HOME/.saga/certificates
test -e $cadir/server.pem && have_pem=true

test "$have_pem" = "true" && hash=`openssl x509 -noout -hash -in $cadir/server.pem`
test "$have_pem" = "true" && rm -f $cadir/server.* $cadir/$hash.0
test -d $HOME/.saga/certificates/ && rm -f $cadir/server.*
test -d $cadir || mkdir $cadir
cd $cadir/
openssl genrsa 1024                                  > $cadir/server.key
chmod   0400                                           $cadir/server.key
openssl req  -new -x509 -nodes -sha1 -days 365 -key    $cadir/server.key \
             -subj '/C=US/O=SAGA/CN=localhost'       > $cadir/server.cert
openssl x509 -noout -fingerprint -text               < $cadir/server.cert \
                                                     > $cadir/server.info
cat         $cadir/server.cert $cadir/server.key     > $cadir/server.pem
chmod 0400  $cadir/server.pem
rm -f                   `openssl x509 -noout -hash -in $cadir/server.pem`.0
ln -s $cadir/server.pem `openssl x509 -noout -hash -in $cadir/server.pem`.0

# from thijs
#
# /usr/lib/ssl/misc/CA.pl -newca 
# /usr/lib/ssl/misc/CA.pl -newreq
# /usr/lib/ssl/misc/CA.pl -sign  
# 

