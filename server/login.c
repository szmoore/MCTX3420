/**
 * @file login.c
 * @brief Implementation of Login related functionality
 */

#define _BSD_SOURCE

#include "login.h"
#include <ctype.h>

#define LDAP_DEPRECATED 1 // Required to use ldap_simple_bind_s
#include <ldap.h>

#define LDAP_URI "ldaps://ldap.pheme.uwa.edu.au"
#define LDAP_DN_BASE "ou=Users,ou=UWA,dc=uwads,dc=uwa,dc=edu,dc=au"	

/**
 * Attempt to bind to the LDAP_URI
 * @param user - The username
 * @param pass - The password
 * @returns An error code according to libldap; LDAP_SUCCESS if everything worked
 */
int Login_LDAP_Bind(const char * user, const char * pass)
{


	Log(LOGINFO, "Username: \"%s\"", user);

	char dn[BUFSIZ]; // Fill with the DN

	const char * user_type = "Students";

	// Staff members have numbers starting in zero
	if (user[0] == '0') 
	{
		user_type = "Staff";	
	}
	
	if (sprintf(dn, "cn=%s,ou=%s,%s", user, user_type, LDAP_DN_BASE) >= BUFSIZ)
	{
		Log(LOGERR,"DN too long; recompile with increased BUFSIZ");	
	}

	// Initialise LDAP; prepares to connect to the server
	LDAP * ld = NULL;
	int err = ldap_initialize(&ld, LDAP_URI);
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
		Log(LOGDEBUG, "Successfully bound to %s with username %s", LDAP_URI, user);
	}

	err = ldap_unbind_s(ld);
	if (err != LDAP_SUCCESS)
	{
		Log(LOGERR, "ldap_unbind_s failed - %s", ldap_err2string(err));
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
		FCGI_RejectJSON(context, "Already logged in");
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

	if (strlen(pass) <= 0)
	{
		FCGI_RejectJSON(context, "No password supplied.");
		return;
	}

	// Try to authenticate
	int err = Login_LDAP_Bind(user, pass);

	// error check	
	
	if (err == LDAP_SUCCESS)
	{
		FCGI_LockControl(context, false);
	}
	

	FCGI_BeginJSON(context, STATUS_OK);
	FCGI_JSONPair("user", user);
	FCGI_JSONPair("login", ldap_err2string(err));
	FCGI_JSONPair("key", context->control_key);
	FCGI_EndJSON();
}
