#!/bin/sh

have_pem=false
test -e ../cert/server.pem && have_pem=true

test "$have_pem" = "true" && hash=`openssl x509 -noout -hash -in ../cert/server.pem`
test "$have_pem" = "true" && rm -f ../cert/server.* ../cert/$hash.0
test -d ../cert && rm -f ../cert/server.*
test -d ../cert || mkdir ../cert
cd ../cert/
openssl genrsa 1024                                  > ../cert/server.key
chmod   0400                                           ../cert/server.key
openssl req  -new -x509 -nodes -sha1 -days 365 -key    ../cert/server.key \
             -subj '/C=US/O=SAGA/CN=localhost'       > ../cert/server.cert
openssl x509 -noout -fingerprint -text               < ../cert/server.cert \
                                                     > ../cert/server.info
cat         ../cert/server.cert ../cert/server.key   > ../cert/server.pem
chmod 0400  ../cert/server.pem
rm -f                    `openssl x509 -noout -hash -in ../cert/server.pem`.0
ln -s ../cert/server.pem `openssl x509 -noout -hash -in ../cert/server.pem`.0

# from thijs
#
# /usr/lib/ssl/misc/CA.pl -newca 
# /usr/lib/ssl/misc/CA.pl -newreq
# /usr/lib/ssl/misc/CA.pl -sign  
# 

