/**
 * @file webserver.c
 * @purpose Test implementing a minimalistic webserver
 * 
 */

#define _POSIX_C_SOURCE 200809L // needed for some low level POSIX stuff to work

// --- Standard headers --- //
#include <stdlib.h>
#include <stdio.h>
#include <string.h> // string helper functions
#include <wctype.h> // wide character classication (white space)
#include <signal.h> // for signal handling

// --- Custom headers --- //
#include "log.h" // C functions to handle logging
#include "options.h" // Options structure
#include "network.h" // C functions to handle low level networking

// --- Variable definitions --- //
Options options; // declared in "options.h"

// --- Function definitions --- //

/**
 * @funct ParseArguments
 * @purpose Parse Arguments and setup the global options variable
 * @param argc - Num args
 * @param argv - Array of args
 */
void ParseArguments(int argc, char ** argv)
{
	options.program = argv[0];
	options.verbosity = LOGDEBUG;
	options.port = 8080; // Using 8080 instead of 80 for now because to use 80 you have to run the program as root
	log_print(LOGDEBUG, "ParseArguments", "Called as %s with %d arguments.", options.program, argc);
}

/**
 * @funct SignalHandler
 * @purpose Handle signals
 * @param sig - The signal
 */
void SignalHandler(int sig)
{
	// At the moment just always exit.
	// Call `exit` so that Cleanup will be called to... clean up.
	log_print(LOGWARN, "SignalHandler", "Got signal %d (%s). Exiting.", sig, strsignal(sig));
	exit(sig);
}

/**
 * @funct Cleanup
 * @purpose Called when program exits
 */
void Cleanup()
{
	log_print(LOGDEBUG, "Cleanup", "Begin cleanup.");
	Network_close(options.sfd); // close socket
	Network_close(options.bound_sfd); // unbind
	log_print(LOGDEBUG, "Cleanup", "Unbound from port %d successfully", options.port);
	log_print(LOGDEBUG, "Cleanup", "Done.");

}


/**
 * @funct Get
 * @purpose Respond to a GET request
 * @param request - The request string
 * @param sfd - Socket to respond through
 */
void Get(const char * request, int sfd)
{
	log_print(LOGDEBUG, "Get", "Got GET request: %s", request);
	char response[BUFSIZ];

	// TODO: Magical low level interfacing stuff!

	int len = sprintf(response, "Content-type: text/plain\n\nYou requested %s using GET\n", request);
	write(sfd, response, len);
}

/**
 * @funct Post
 * @purpose Respond to a POST request
 * @param request - The request string
 * @param sfd - Socket to respond through
 */
void Post(const char * request, int sfd)
{
	log_print(LOGDEBUG, "Post", "Got POST request: %s", request);
	char response[BUFSIZ];

	// TODO: Magical low level interfacing stuff!

	int len = sprintf(response, "Content-type: text/plain\n\nYou requested %s using POST\n", request);
	write(sfd, response, len);

}

/**
 * @funct main
 * @purpose Main program
 * @param argc - Num arguments
 * @param argv - Argument string array
 * @returns error code (0 for no error)
 */

int main(int argc, char ** argv)
{
	// Parse Arguments
	ParseArguments(argc, argv);
	// Set Cleanup to be called on program exit
	atexit(Cleanup);

	// Setup signal handlers
	int signals_to_handle[] = {SIGTERM, SIGINT, SIGHUP, SIGPIPE};
	for (int i = 0; i < sizeof(signals_to_handle)/sizeof(int); ++i)
	{
		int s = signals_to_handle[i];
		if (signal(s, SignalHandler) == SIG_ERR)
			error("main", "Setting signal handler for %d (%s): %s", s, strsignal(s), strerror(errno));
	}

	// Bind to the port
	options.bound_sfd = Network_server_bind(options.port, &(options.port));
	log_print(LOGDEBUG, "main", "Bound to port %d succesfully", options.port);

	while (true)
	{
			// Listen for a client
			options.sfd = Network_server_listen(options.bound_sfd, NULL);
			
			log_print(LOGDEBUG, "main", "Connected to client");
	
			char buffer[BUFSIZ]; //NOTE: Won't be able to respond to requests longer than BUFSIZ
			// read a request
			int len = read(options.sfd, buffer, sizeof(buffer));
			log_print(LOGDEBUG, "main", "Read %d characters. Buffer is %s", len, buffer);

			// Parse request
			for (int i = 0; i < sizeof(buffer) && buffer[i] != '\0'; ++i)
			{
				// Look for "GET" or "POST" followed by a whitespace
				if (iswspace(buffer[i])) // whitespace
				{
					while (iswspace(buffer[++i]) && buffer[i] != '\0'); // Skip whitespace
					char * req = buffer+i; // set request string
					buffer[i] = '\0'; // terminate request type
					if (strcmp("GET", buffer) == 0) // Compare with "GET"
					{
						Get(req, options.sfd);
					}
					else if (strcmp("POST", buffer) == 0) // Compare with "POST"
					{
						Post(req, options.sfd);
					}
					else // Unknown request
					{
						log_print(LOGWARN, "main", "Unrecognised request type %s (request %s)", buffer, req);
						char response[] = "Content-type: text/plain\n\nError: Unrecognised request";
						write(options.sfd, response, sizeof(response));
					}
					break;
				}
			}

			// Close connection
			Network_close(options.sfd);
			log_print(LOGDEBUG, "main", "Closed connection to client");
	}


	
	
	return 0;
}


