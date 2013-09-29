#define _BSD_SOURCE
#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

// Compile with: gcc -std=c99 -lcrypt

/** Deliberately make this smaller so we can test for buffer overflow problems **/
#define BUFSIZ 50


bool EnterTheShadowRealm(const char * shadow, const char * salt, const char * username, const char * passwd)
{

	if (strlen(username) + strlen(passwd) >= BUFSIZ-1)
	{
		fprintf(stderr, "User/Password too long!\n");
		return false;
	}

	FILE * f = fopen(shadow, "r");
	if (f == NULL)
	{
		fprintf(stderr, "Can't open %s - %s\n", shadow, strerror(errno));
		return false;
	}

	char buffer[BUFSIZ];
	int passwd_index = -1;
	int garbage_index = -1;
	while (fgets(buffer, BUFSIZ, f) != NULL) // NOTE: Restrict username+password strings to BUFSIZ... what could possibly go wrong?
	{

		printf("Scanning %d: %s", strlen(buffer), buffer);
	
		for (int i = 0; i < BUFSIZ-1; ++i)
		{
			if (buffer[i] == ':')
			{
				buffer[i] = '\0';
				passwd_index = i+1;
				break;
			}
		}

		if (strcmp(username,buffer) == 0)
		{
			printf("User matches! %s\n", buffer);
			break;
		} 
		passwd_index = -1;
	}

	if (passwd_index <= 0)
	{
		fprintf(stderr, "No user found matching %s\n", username);
		return false;
	}

	for (int i = passwd_index; i < BUFSIZ-1; ++i)
	{
		if (buffer[i] == ':' || buffer[i] == '\n')
		{
			buffer[i] = '\0';
			garbage_index = i+1;
		}
	}

	printf("Salted Entry: %s\n", buffer+passwd_index);
	printf("Salted Attempt: %s\n", crypt(passwd, salt));
	
	return (strcmp(crypt(passwd, salt), passwd) == 0);
	
}


int main(int argc, char ** argv)
{
	char * shadow = "shadow";
	if (argc > 1)
	{
		shadow = argv[1];
	}


	
	
	// Get the username and password
	// Need to get these passed through HTTPS at some point
	printf("Username: ");
	char username[BUFSIZ];
	if (fgets(username, BUFSIZ, stdin) != username)
	{
		fprintf(stderr, "Username too long!\n");
		exit(EXIT_FAILURE);
	}

	username[strlen(username)-1] = '\0';

	char * password = getpass("Password: "); //NOTE: getpass is deprecated. Just here for testing.
	password[strlen(password)-1] = '\0';
	
	printf("Could we enter the shadow realm? %d\n", EnterTheShadowRealm(shadow, "A9", username, password));
	
	
	
	return 0;
}
