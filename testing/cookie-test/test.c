#include <fcgi_stdio.h>
#include <stdlib.h>
int main() {
	while (FCGI_Accept() >= 0) {
		printf("Content-type: text\r\n");
		printf("Set-Cookie: name=value with spaces; and a semicolon\r\n");
		printf("Set-Cookie: name2=value2\r\n\r\n");
		printf("Cookie:%s\n", getenv("COOKIE_STRING"));
	}
}
