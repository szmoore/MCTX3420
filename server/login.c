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
 * This is here... because all better options have been exhausted
 * @param user - The username
 * @param pass - The password
 * @returns Privelage level of the user or USER_UNAUTH for failure to authenticate
 */
UserType Login_Shadow(const char * user, const char * pass, const char * shadow)
{
	if (strlen(user) + strlen(pass) >= BUFSIZ-1)
	{
		Log(LOGERR, "User/Password too long!\n");
		return USER_UNAUTH;
	}

	FILE * f = fopen(shadow, "r");
	if (f == NULL)
	{
		Log(LOGERR,"Can't open %s - %s\n", shadow, strerror(errno));
		return USER_UNAUTH;
	}

	char buffer[BUFSIZ];
	int passwd_index = -1;

	while (fgets(buffer, BUFSIZ, f) != NULL) // NOTE: Restrict username+password strings to BUFSIZ... what could possibly go wrong?
	{

		//Log(LOGDEBUG,"Scanning %d: %s", strlen(buffer), buffer);
	
		for (int i = 0; i < BUFSIZ-1 && buffer[i] != '\0'; ++i)
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
			//Log(LOGDEBUG,"User matches! %s\n", buffer);
			break;
		} 
		passwd_index = -1;
	}

	if (passwd_index <= 0)
	{
		//Log(LOGDEBUG,"No user found matching %s\n", user);
		return USER_UNAUTH;
	}

	int gid_index = -1;
	for (int i = passwd_index; i < BUFSIZ-1 && buffer[i] != '\0'; ++i)
	{
		if (buffer[i] == ':')
		{
			gid_index = i+1;
			buffer[i] = '\0';
		}
		if (buffer[i] == '\n')
			buffer[i] = '\0';
	}
	char * end = buffer+gid_index;
	UserType user_type = USER_NORMAL;
	if (gid_index > passwd_index && gid_index < BUFSIZ-1)
	{
		int gid = strtol(buffer+gid_index, &end,10);
		Log(LOGDEBUG, "Usertype %d %s", gid, buffer+gid_index);
		if (*end == '\0' && gid == 0)
		{
			Log(LOGDEBUG, "Admin");
			user_type = USER_ADMIN;
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

//	Log(LOGDEBUG,"Salted Entry: %s\n", buffer+passwd_index);
//	Log(LOGDEBUG,"Salted Attempt: %s\n", crypt(pass, salt));
	
	if (strcmp(crypt(pass, salt), buffer+passwd_index) == 0)
	{
		return user_type;
	}
	return USER_UNAUTH;
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
	FCGI_AcceptJSON(context, "Logged out", "0");
}


/**
 * Handle a Login Request
 * @param context - The context
 * @param params - Parameter string, should contain username and password
 */
void Login_Handler(FCGIContext * context, char * params)
{
	char * user; // The username supplied through CGI
	char * pass; // The password supplied through CGI

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

	// Trim leading whitespace
	int i = 0;
	for (i = 0; isspace(user[0]) && user[0] != '\0'; ++i, ++user);

	// Truncate string at first non alphanumeric character
	for (i = 0; isalnum(user[i]) && user[i] != '\0'; ++i);
	user[i] = '\0';

	
	UserType user_type = USER_UNAUTH;
	
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
			//int len = sprintf(dn, "uid=%s,%s", user, g_options.ldap_base_dn);
	
			// At UWA (hooray)
			char * user_group = "Students";
			if (user[0] == '0')
				user_group = "Staff";
			int len = sprintf(dn, "cn=%s,ou=%s,%s", user, user_group, g_options.ldap_base_dn);
		

			if (len >= BUFSIZ)
			{
				FCGI_RejectJSON(context, "DN too long! Recompile with increased BUFSIZ");
				return;
			}
		
			if (Login_LDAP_Bind(g_options.auth_uri, dn, pass) == LDAP_SUCCESS)
			{
				if (user[0] == '0')
					user_type = USER_ADMIN;
				else
					user_type = USER_NORMAL;
			}
			break;
		}
		case AUTH_SHADOW:
		{
			user_type = Login_Shadow(user, pass, g_options.auth_uri);
			break;
		}
		default:
		{
			Log(LOGWARN, "No authentication!");
			user_type = USER_ADMIN;
			break;
		}
	}
		
	// error check	
	
	if (user_type == USER_UNAUTH)
	{
		Log(LOGDEBUG, "Authentication failure for %s", user);
		FCGI_RejectJSONEx(context, STATUS_UNAUTHORIZED, "Authentication failure.");
	}
	else
	{
		// Try and gain control over the system
		if (FCGI_LockControl(context, user, user_type))
		{
			FCGI_EscapeText(context->user_name); //Don't break javascript pls
			// Give the user a cookie
			FCGI_AcceptJSON(context, "Logged in", context->control_key);
			Log(LOGDEBUG, "Successful authentication for %s", user);
		}
		else
		{
			Log(LOGDEBUG, "%s successfully authenticated but system was in use by %s", user, context->user_name);
			FCGI_RejectJSON(context, "Someone else is already logged in (and you are not an admin)");
		}
	}
}
