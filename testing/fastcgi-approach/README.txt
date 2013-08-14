The application could be quite easily made to use FastCGI. Unlike normal CGI,
with FastCGI (fcgi), the process is executed once, and continues to run. The
process will receive responses from the browser in the response loop. Hence,
sensor data can be read in another thread while the response loop runs.

Setup:
Compile fastcgi_test.c with:
gcc fastcgi_test.c -lfcgi -o fastcgi_test

Configure nginx to pass all requests to the address /cgi/ to the application:
Edit /etc/nginx/sites-enabled/default by adding the contents of nginx_server_config.txt

Restart nginx:
/etc/init.d/nginx restart

Run the application:
spawn-fcgi -p9005 -n ./fastcgi_test

You can see the results at:
http://your.domain/cgi