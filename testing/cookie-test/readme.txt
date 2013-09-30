compile with:
gcc test.c -lfcgi -o test

Run with:
spawn-fcgi -p9005 -n ./test

nginx must be configured to pass $http_cookie as an evironment variable named COOKIE