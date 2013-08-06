#ifndef _NETWORK_H
#define _NETWORK_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>
#include <stdarg.h>

extern int Network_get_port(int socket); // get port used by socket
extern int Network_server(char * addr, int port);
extern int Network_client(const char * addr, int port, int timeout);

extern int Network_server_bind(int port, int * bound);
extern int Network_server_listen(int sfd, char * addr);

extern void Network_close(int sfd);

#endif //_NETWORK_H

//EOF
