#!/bin/bash

# Check running as root
if [ "$(whoami)" != "root" ]; then
        (echo "Run $0 as root.") 1>&2
        exit 1
fi

nconf=/usr/share/nginx/conf
# Generate the ssl cert, if necessary
if [ ! -d "$nconf" ]; then
	echo "ssl cert not found at $nconf, generating..."
	./gen_ssl_cert.sh $1
 	if [[ $? != 0 ]] ; then
		(echo "gen_ssl_cert failed, exiting.") 1>&2
		exit 1
	fi;
fi

nginx=/etc/nginx
# Check for nginx
if [ ! -d "$nginx" ]; then
	(echo "Install nginx.") 1>&2
	exit 1
fi

# Copy nginx configs
echo
echo "Copying nginx configs..."
cp -v nginx/fastcgi_params ./nginx/mime.types $nginx/
rm -fv $nginx/sites-enabled/*
cp -v nginx/sites-enabled/mctxconfig $nginx/sites-enabled

echo
echo "Restarting nginx..."
/etc/init.d/nginx restart

echo
echo "Note: Check the document root for the nginx config is set correctly."

# Copy syslog configs
rsyslog=/etc/rsyslog.d
lrotate=/etc/logrotate.d
if [ -d "$rsyslog" ]; then
	echo
	echo "Copying rsyslog configs..."
	cp -v rsyslog.d/30-mctxserv.conf $rsyslog/

	echo
	echo "Restarting rsyslog..."
	/etc/init.d/rsyslog restart
else
	echo
	(echo "Could not find rsyslog at $rsyslog. Skipping.") 1>&2
fi

if [ -d "$lrotate" ]; then
	echo
	echo "Copying logrotate configs..."
	cp -v logrotate.d/mctxserv.conf $lrotate/
else
	echo
	(echo "Could not find logrotate at $lrotate. Skipping.") 1>&2
fi


