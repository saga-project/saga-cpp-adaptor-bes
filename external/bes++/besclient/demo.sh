#!/bin/sh


##################################################################################
#
function run_test() {

  echo " === $1 ==="
  rm -f job-epr.xml

  export ARGS=""

  if test "$USER"        != ""; then ARGS="$ARGS -u $USER"       ; fi
  if test "$PASS"        != ""; then ARGS="$ARGS -p $PASS"       ; fi
  if test "$CERT"        != ""; then ARGS="$ARGS -x $CERT"       ; fi
  if test "$KEY"         != ""; then ARGS="$ARGS -k $KEY"        ; fi
  if test "$CADIR"       != ""; then ARGS="$ARGS -c $CADIR"      ; fi
  if test "$ENDPOINT"    != ""; then ARGS="$ARGS -e $ENDPOINT"   ; fi

  ok=1
  
  echo ./besclient $ARGS create sleep-hpc.jsdl job-epr.xml || ok=0
  ./besclient $ARGS create sleep-hpc.jsdl job-epr.xml || ok=0

  if test "$ok" = "1"; then

    res=`./besclient $ARGS status job-epr.xml | head -1`
    state=`echo $res | cut -f 2 -d ':' | sed -e "s/ //g"`
    echo "state: $state"
  
    while test  "$state" = "Pending" -o "$state" = "Running"; do
  
      sleep 1
  
      res=`./besclient $ARGS status job-epr.xml | head -1`
      state=`echo $res | cut -f 2 -d ':' | sed -e "s/ //g"`
      echo "state: $state"
  
    done

  fi # ok?

  echo " === $1 ==="
}
#
##################################################################################



##################################################################################
#
  unset  USER PASS CERT KEY CADIR ENDPOINT
  export USER=""
  export PASS=""
  export CERT=/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/arc-user-cert+key.pem
  export KEY=z1nfandel
  export CADIR=/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/certificates
  export ENDPOINT=arc.xml
  
  run_test arc
#
##################################################################################

exit 1


##################################################################################
#
  unset  USER PASS CERT KEY CADIR ENDPOINT
  export USER=ogf
  export PASS=ogf
  export CERT=/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/bes_client_cert.pem
  export KEY=""
  export CADIR=/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/certificates
  export ENDPOINT=gridsam.xml
  
  run_test gridsam
#
##################################################################################



##################################################################################
# 
  unset  USER PASS CERT KEY CADIR ENDPOINT
  export USER=ogf
  export PASS=ogf
  export CERT=/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/bes_client_cert.pem
  export KEY=/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/bes_client_cert.pem
  export CADIR=/Users/merzky/links/saga/adaptors/ogf/trunk/ogf_hpcbp_job/certs/certificates/
  export ENDPOINT=unicore.xml
 
  run_test unicore 
#
##################################################################################



##################################################################################
#
  unset  USER PASS CERT KEY CADIR ENDPOINT
  export USER=merzky
  export PASS=aaa
  export CERT=""
  export KEY=""
  export CADIR=/Users/merzky/links/saga/adaptors/ogf/trunk/external/bes++/besserver/cert/
  export ENDPOINT=local.xml

  run_test local 
#
##################################################################################

