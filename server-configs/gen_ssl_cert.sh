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

echo 'Making the conf dir /usr/share/nginx/conf...'
mkdir -p /usr/share/nginx/conf

echo Generating the server private key...
openssl genrsa -out /usr/share/nginx/conf/server.key 2048

echo Generating the CSR...
openssl req -new -key /usr/share/nginx/conf/server.key \
-out /usr/share/nginx/conf/server.csr \
 -subj "/C=AU/ST=WA/L=Perth/O=UWA/OU=Mechatronics/CN=$1"

echo Signing the certificate...
openssl x509 -req -days 3650 -in /usr/share/nginx/conf/server.csr \
-signkey /usr/share/nginx/conf/server.key \
-out /usr/share/nginx/conf/server.crt


