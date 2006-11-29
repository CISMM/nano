#!/bin/sh
# This script should be run from MSDEV. It relies on the $WkspDir
# variable being set.

#echo proj: $ProjDir
#cd '$(WkspDir)\app\nano\lib\nmMScope';

VRPNPATH="vrpn"
for i in 0 1 2 3 4 5 6 7 8 9 10 11 12; do
	VRPNPATH="../$VRPNPATH"
	if [ -x $VRPNPATH/util/gen_rpc/gen_vrpn_rpc.pl ] ; then
		break
	fi
done


"$VRPNPATH/util/gen_rpc/gen_vrpn_rpc.pl" "$@"
