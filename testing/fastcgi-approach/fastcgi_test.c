#include "fcgi_stdio.h" /* fcgi library; put it first*/
#include <stdlib.h>

int main (int argc, char *argv[])
{
  int count = 0;

  //Spawn thread to get sensor data here?
  //Response loop
	while (FCGI_Accept() >= 0)   {
		printf("Content-type: text/html\r\n"
			   "\r\n"
			   "<title>FastCGI Hello! (C, fcgi_stdio library)</title>"
			   "<h1>FastCGI Hello! (C, fcgi_stdio library)</h1>"
			   "Request number %d running on host <i>%s</i>\n",
			   count++, getenv("SERVER_HOSTNAME"));

		char *data = getenv("QUERY_STRING");
		if (data) {
			printf("<br>Query string is: '%s'\n", data);
		}
	}
}
