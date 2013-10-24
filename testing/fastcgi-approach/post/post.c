#include <fcgi_stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
char *FCGI_URLDecode(char *buf);

int main() {
	while (FCGI_Accept() >= 0) {
		char buf[BUFSIZ];
		printf("Content-type: text/plain\r\n\r\n");


		while(fgets(buf, BUFSIZ, stdin)) {
			printf("POST (raw):\r\n");
			printf("%s", buf);
			printf("\r\nPOST (decoded):\r\n");
			printf("%s", FCGI_URLDecode(buf));
		}

		snprintf(buf, BUFSIZ, "%s", getenv("QUERY_STRING"));
		printf("\r\nGET (raw):\r\n");
		printf("%s", getenv("QUERY_STRING"));

		printf("\r\nGET (decoded):\r\n");
		printf("%s", FCGI_URLDecode(buf));
	}
	return 0;

}

char *FCGI_URLDecode(char *buf) {
	char *head = buf, *tail = buf;
	char hex[3] = {0};
	while (*tail) {
		if (*tail == '%') {
			tail++;
			if (isxdigit(*tail) && isxdigit(*(tail+1))) {
				hex[0] = *tail++;
				hex[1] = *tail++;
				*head++ = (char)strtol(hex, NULL, 16);
			} else {
				head++;
			}
		} else if (*tail == '+') {
			tail++;
			*head++ = ' ';
		} else {
			*head++ = *tail++;
		}
	}
	*head = 0;
	return buf;
}
