#!/bin/sh

root=/Users/merzky/links/saga/trunk/adaptors/ogf/external/bes++/
besserver=$root/besserver/besserver
service_user=merzky
host=localhost
endpoint="https://${host}"
certfile=$root/besserver/cert/host.pem
port=1236

# needed for SSL mutual authentication
capath=$root/besserver/cert/

# user to run jobs from users authenticated using SSL
generic_user=merzky

# if [ -f /ego/lsf/conf/profile.lsf ]; then
# 	. /ego/lsf/conf/profile.lsf
# else
# 	echo "$0: can't find profile.lsf"
# 	exit 1
# fi

echo $besserver -u $service_user -h $host -p $port -e $endpoint -s $certfile -c $capath -g $generic_user -r saga
$besserver -u $service_user -h $host -p $port -e $endpoint -s $certfile -c $capath -g $generic_user -r saga

exit 0

