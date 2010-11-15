#!/bin/sh

# kill old tunnels
echo "close  stale        tunnels"
ps -Eefw | grep TUNNEL | grep TUNNEL | grep -v grep | grep ssh | cut -c 6-12 | xargs kill -9

# echo "create advert       tunnel : advert.cct.lsu.edu:8080"
# env TAG=SSHTUNNEL ssh -fNn -L 10000:advert.cct.lsu.edu:8080 amerzky@cyder.cct.lsu.edu
# 
# echo "create arc          tunnel : interop.grid.niif.hu:60000"
# env TAG=SSHTUNNEL ssh -fNn -L 10001:interop.grid.niif.hu:60000 amerzky@cyder.cct.lsu.edu
# 
# echo "create fg.uc.sierra tunnel : s79r.idp.sdsc.futuregrid.org:8080"
# env TAG=SSHTUNNEL ssh -fNn -L 10002:s79r.idp.sdsc.futuregrid.org:8080 amerzky@cyder.cct.lsu.edu
# 
# echo "create fg.uc.india  tunnel : i134r.idp.iu.futuregrid.org:8080"
# env TAG=SSHTUNNEL ssh -fNn -L 10003:i134r.idp.iu.futuregrid.org:8080 amerzky@cyder.cct.lsu.edu

echo "create gridsam      tunnel : gridsam-test.oerc.ox.ac.uk:18443"
env TAG=SSHTUNNEL ssh -fNn -L 10004:gridsam-test.oerc.ox.ac.uk:18443 amerzky@cyder.cct.lsu.edu


