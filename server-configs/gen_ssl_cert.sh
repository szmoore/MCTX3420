#!/bin/bash

# Check input params
if [ $# -ne 1 ]; then
	(echo "Usage: $0 common-name") 1>&2
	exit 1
fi

# Check running as root
if [ "$(whoami)" != "root" ]; then
        (echo "Run $0 as root.") 1>&2
        exit 1
fi

# Check for nginx dir
nginx=/usr/share/nginx
if [ ! -d "$nginx" ]; then
	(echo "nginx folder not found at $nginx.") 1>&2
	exit 1
fi;

echo 'Making the conf dir $nginx/conf...'
mkdir -p $nginx/conf

echo Generating the server private key...
openssl genrsa -out $nginx/conf/server.key 2048

echo Generating the CSR...
openssl req -new -key $nginx/conf/server.key -out $nginx/conf/server.csr \
 -subj "/C=AU/ST=WA/L=Perth/O=UWA/OU=Mechatronics/CN=$1"

echo Signing the certificate...
openssl x509 -req -days 3650 -in $nginx/conf/server.csr \
 -signkey $nginx/conf/server.key -out $nginx/conf/server.crt


