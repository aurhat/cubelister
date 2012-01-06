#!/bin/bash

WGET=`which wget`
GUNZIP=`which gunzip`

test -z "$WGET" && { echo "$0: wget missing"; exit 1; }
test -z "$GUNZIP" && { echo "$0: gunzip missing"; exit 1; }

DPATH=${0%/*}

cd "$DPATH"

"$WGET" http://geolite.maxmind.com/download/geoip/database/GeoLiteCountry/GeoIP.dat.gz || exit 1;
test -f GeoIP.dat && rm GeoIP.dat && "$GUNZIP" GeoIP.dat.gz

test "$1" = "all" || exit

"$WGET" http://geolite.maxmind.com/download/geoip/database/GeoLiteCity.dat.gz || exit 1;
test -f GeoLiteCity.dat && rm GeoLiteCity.dat && "$GUNZIP" GeoLiteCity.dat.gz
