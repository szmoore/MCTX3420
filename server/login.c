/**
 * @file login.c
 * @brief Implementation of Login related functionality
 */




#include "login.h"
#include "options.h"
#include <ctype.h>
#include <unistd.h>

#define LDAP_DEPRECATED 1 // Required to use ldap_simple_bind_s
#include <ldap.h>

/**
 * Attempt to login using a file formatted like /etc/shadow
 * This is here for horrible hack purposes
 * @param user - The username
 * @param pass - The password
 * @returns True if the login was successful, false otherwise
 */
bool Login_Shadow(const char * user, const char * pass, const char * shadow)
{
	if (strlen(user) + strlen(pass) >= BUFSIZ-1)
	{
		Log(LOGERR, "User/Password too long!\n");
		return false;
	}

	FILE * f = fopen(shadow, "r");
	if (f == NULL)
	{
		Log(LOGERR,"Can't open %s - %s\n", shadow, strerror(errno));
		return false;
	}

	char buffer[BUFSIZ];
	int passwd_index = -1;

	while (fgets(buffer, BUFSIZ, f) != NULL) // NOTE: Restrict username+password strings to BUFSIZ... what could possibly go wrong?
	{

		Log(LOGDEBUG,"Scanning %d: %s", strlen(buffer), buffer);
	
		for (int i = 0; i < BUFSIZ-1; ++i)
		{
			if (buffer[i] == ':')
			{
				buffer[i] = '\0';
				passwd_index = i+1;
				break;
			}
		}

		if (strcmp(user,buffer) == 0)
		{
			Log(LOGDEBUG,"User matches! %s\n", buffer);
			break;
		} 
		passwd_index = -1;
	}

	if (passwd_index <= 0)
	{
		Log(LOGDEBUG,"No user found matching %s\n", user);
		return false;
	}

	for (int i = passwd_index; i < BUFSIZ-1; ++i)
	{
		if (buffer[i] == ':' || buffer[i] == '\n')
		{
			buffer[i] = '\0';
			
		}
	}
	
	// Determine the salt
	char salt[BUFSIZ];
	int s = 0; int count = 0;
	for (int i = passwd_index; i < BUFSIZ-1; ++i)
	{
		salt[s++] = buffer[i];
		if (salt[s] == '$' && ++count >= 3)
			break;
	}

	Log(LOGDEBUG,"Salted Entry: %s\n", buffer+passwd_index);
	Log(LOGDEBUG,"Salted Attempt: %s\n", crypt(pass, salt));
	
	return (strcmp(crypt(pass, salt), buffer+passwd_index) == 0);
}

/**
 * Attempt to bind to a LDAP uri
 * @param uri - The uri
 * @param dn - The DN
 * @param pass - The password
 * @returns An error code according to libldap; LDAP_SUCCESS if everything worked
 */
int Login_LDAP_Bind(const char * uri, const char * dn, const char * pass)
{
	Log(LOGDEBUG, "Bind to %s with dn %s and pass %s", uri, dn, pass);

	// Initialise LDAP; prepares to connect to the server
	LDAP * ld = NULL;
	int err = ldap_initialize(&ld, uri);
	if (err != LDAP_SUCCESS || ld == NULL)
	{
		Log(LOGERR,"ldap_initialize failed - %s (ld = %p)", ldap_err2string(err), ld);
		return err;
	}

	// Set the LDAP version...
	int version = LDAP_VERSION3;
	err = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version); // specify the version
	if (err != LDAP_SUCCESS)
	{
		Log(LOGERR,"ldap_set_option failed - %s", ldap_err2string(err));
		return err;
	}

	// Attempt to bind using the supplied credentials.
	// NOTE: ldap_simple_bind_s is "deprecated" in <ldap.h>, but not listed as such in the man pages :S
	err = ldap_simple_bind_s(ld, dn, pass);
	if (err != LDAP_SUCCESS)
	{
		Log(LOGERR, "ldap_simple_bind_s failed - %s", ldap_err2string(err));
	}
	else
	{
		Log(LOGDEBUG, "Successfully bound to %s with dn %s", uri, dn);
	}

	int err2 = ldap_unbind_s(ld);
	if (err2 != LDAP_SUCCESS)
	{
		Log(LOGERR, "ldap_unbind_s failed - %s", ldap_err2string(err2));
		err = err2;
	}
	return err;
}

/**
 * Logout
 * @param context - The context. The key will be cleared.
 * @param params - Parameter string, UNUSED
 */
void Logout_Handler(FCGIContext * context, char * params)
{		
	FCGI_ReleaseControl(context);
}


/**
 * Handle a Login Request
 * @param context - The context
 * @param params - Parameter string, should contain username and password
 */
void Login_Handler(FCGIContext * context, char * params)
{

	if (context->control_key[0] != '\0')
	{
		FCGI_RejectJSON(context, "Already logged in.");
		return;
	}

	char * user = ""; // The username supplied through CGI
	char * pass = ""; // The password supplied through CGI
						//TODO: Make sure these are passed through HTTPS, *not* HTTP .... otherwise people can eavesdrop on the passwords

	FCGIValue values[] = {
		{"user", &user, FCGI_REQUIRED(FCGI_STRING_T)},
		{"pass", &pass, FCGI_REQUIRED(FCGI_STRING_T)},
	};

	//enum to avoid the use of magic numbers
	typedef enum {
		USER,
		PASS,
		LOGOUT
	} LoginParams;

	// Fill values appropriately
	if (!FCGI_ParseRequest(context, params, values, sizeof(values)/sizeof(FCGIValue)))
	{
		// Error occured; FCGI_RejectJSON already called
		return;
	}


	// Trim leading whitespace (the BUFSIZ check is to make sure incorrectly terminated strings don't cause an infinite loop)
	int i = 0;
	for (i = 0; i < BUFSIZ && isspace(user[0]) && user[0] != '\0'; ++i,++user);

	// Truncate string at first non alphanumeric character
	for (i = 0; i < BUFSIZ && isalnum(user[i]) && user[i] != '\0'; ++i);
	user[i] = '\0';



	
	bool authenticated = true;
	
	switch (g_options.auth_method)
	{

		case AUTH_LDAP:
		{
			if (strlen(pass) <= 0)
			{
				FCGI_RejectJSON(context, "No password supplied.");
				return;
			}

			//TODO: Generate the DN in some sane way
			char dn[BUFSIZ];
		
			// On a simple LDAP server:
			int len = sprintf(dn, "uid=%s,%s", user, g_options.ldap_base_dn);
	
			// At UWA (hooray)
			//char * user_type = (user[0] != '0') : "Students" ? "Staff";
			//int len = sprintf(dn, "cn=%s,ou=%s", user, user_type, g_options.ldap_dn_base);
		

			if (len >= BUFSIZ)
			{
				FCGI_RejectJSON(context, "DN too long! Recompile with increased BUFSIZ");
			}
		
			authenticated = (Login_LDAP_Bind(g_options.auth_uri, dn, pass) == LDAP_SUCCESS);
			break;
		}
		case AUTH_SHADOW:
		{
			authenticated = Login_Shadow(user, pass, g_options.auth_uri);
			break;
		}
		default:
		{
			Log(LOGWARN, "No authentication!");
			break;
		}
	}
		
	// error check	
	
	if (!authenticated)
	{
		FCGI_RejectJSON(context, "Authentication failure.");
		return;
	}

	FCGI_LockControl(context, false);
	
	// Give the user a cookie
	FCGI_PrintRaw("Content-type: text\r\n");
	FCGI_PrintRaw("Set-Cookie: %s\r\n\r\n", context->control_key);
	
}
