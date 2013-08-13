#include "network.h"
#include <assert.h>
#include <errno.h>
#include <sys/select.h>
#include "log.h"

#define h_addr h_addr_list[0]





int Network_get_port(int sfd)
{
	static struct sockaddr_in sin;
	static socklen_t len = sizeof(struct sockaddr_in);

	if (getsockname(sfd, (struct sockaddr *)&sin, &len) != 0)
   		 error("Network_port", "getsockname : %s", strerror(errno));
	return ntohs(sin.sin_port);
}

int Network_server_bind(int port, int * bound)
{
	int sfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sfd < 0)
	{
		error("Network_server", "Creating socket on port %d : %s", port, strerror(errno));
	}

	struct   sockaddr_in name;

	name.sin_family = AF_INET;
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	name.sin_port = htons(port);

	if (bind( sfd, (struct sockaddr *) &name, sizeof(name) ) < 0)
	{
		error("Network_server", "Binding socket on port %d : %s", port, strerror(errno));
	}

	if (bound != NULL)
		*bound = Network_get_port(sfd);
	return sfd;	
}

int Network_server_listen(int sfd, char * addr)
{
	int port = Network_get_port(sfd);
	if (listen(sfd, 1) < 0)
	{
		error("Network_server", "Listening on port %d : %s", port, strerror(errno));
	}
	
	int psd;
	if (addr == NULL)
		psd = accept(sfd, 0, 0);
	else
	{
		struct	sockaddr_in client;
		struct  hostent *hp;

		client.sin_family = AF_INET;
		hp = gethostbyname(addr);
		bcopy ( hp->h_addr, &(client.sin_addr.s_addr), hp->h_length);
		client.sin_port = htons(port);
		socklen_t len = sizeof(client);

		psd = accept(sfd, (struct sockaddr*)&client, &len);
	}
	//close(sfd); // don't close the bind socket here; we might want to reuse the port
	assert(psd >= 0);
	return psd;
}

int Network_server(char * addr, int port)
{
	int bind = Network_server_bind(port, &port);
	int sfd = Network_server_listen(bind, addr);
	close(bind); // won't be able to reuse the port (it goes into TIME_WAIT)
	return sfd;
}

int Network_client(const char * addr, int port, int timeout)
{
	int sfd = socket(PF_INET, SOCK_STREAM, 0);

	//log_print(2, "Network_client", "Created socket");
	long arg = fcntl(sfd, F_GETFL, NULL);
	arg |= O_NONBLOCK;
	fcntl(sfd, F_SETFL, arg);

	if (sfd < 0)
	{
		error("Network_client", "Creating socket for address %s:%d : %s", addr, port, strerror(errno));
	}
	struct	sockaddr_in server;
	struct  hostent *hp;


	server.sin_family = AF_INET;
	hp = gethostbyname(addr);
	if (hp == NULL)
	{
		error("Network_client", "Can't get host by name %s", addr);
	}
	bcopy ( hp->h_addr, &(server.sin_addr.s_addr), hp->h_length);
	server.sin_port = htons(port);


	int res = connect(sfd, (struct sockaddr *) &server, sizeof(server));
	

	if (res < 0 && errno == EINPROGRESS)
	{
		
		fd_set writeSet;
		FD_ZERO(&writeSet);
		FD_SET(sfd, &writeSet);

		struct timeval tv;
		tv.tv_sec = timeout;
		tv.tv_usec = 0;

		struct timeval * tp;
		tp = (timeout < 0) ? NULL : &tv;
		
		int err = select(sfd+1, NULL, &writeSet, NULL, tp);
		
		if (err == 0)
		{
			error("Network_client", "Timed out trying to connect to %s:%d after %d seconds", addr, port, timeout);
		}
		else if (err < 0)
		{
			error("Network_client", "Connecting to %s:%d - Error in select(2) call : %s", addr, port, strerror(errno));
		}
		else if (FD_ISSET(sfd, &writeSet))
		{
			int so_error;
			socklen_t len = sizeof so_error;
			getsockopt(sfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
			if (so_error != 0)
			{
				error("Network_client", "Connecting to %s:%d : %s", addr, port, strerror(so_error));
			}
		}
		else
		{
			error("Network_client", "select(2) returned %d but the socket is not writable!?", err);
		}
	}
	else
	{
		error("Network_client", "Connecting to %s:%d : %s", addr, port, strerror(errno));
	}

	arg = fcntl(sfd, F_GETFL, NULL);
	arg &= (~O_NONBLOCK);
	fcntl(sfd, F_SETFL, arg);
	
	
	
	return sfd;
}

void Network_close(int sfd)
{
	if (shutdown(sfd, 2) != 0)
	{
		error("Network_close", "Closing socket : %s", strerror(errno));
	}
	close(sfd);
}
