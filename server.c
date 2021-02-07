/**
 * FILE: server.c
 * MODULE: ChatKey
 * SUMMARY: Windows TCP server for the ChatKey program
 * FUNCTIONS:
 * AUTHOR: Kai Hoffman
 * DATE: February 06, 2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include "server.h"

#pragma comment(lib, "ws2_32");

/**
 * @name main
 * @brief Entry point for ChatKeyServer
 * @param argc The number of arguments (including .\ChatKeyServer) that were entered on the command line
 * @param argv The arguments that were entered on the command line
 * @returns 0 on successful exit
 */
int main(int argc, char **argv)
{
	int port;

	if (argc > 2)
	{
		fprintf(stderr, "%s", USAGE);
		exit(EXIT_FAILURE);
	}

	// get the port number
	if (argc == 2)
	{
		port = atoi(argv[1]);
	}
	else
	{
		port = 8080;
	}

	run_server(port);
	exit(EXIT_SUCCESS);
}

#pragma region Helpers
/**
 * @name run_server
 * @brief Run the server for clients to connect to
 * @param port The port number to connect to
 */
void run_server(int port)
{
	int connect_fd;
	struct sockaddr_in client_ip_addr;
	int addrlen = sizeof(client_ip_addr);

	// open a server socket for clients to connect to
	int master_socket = open_server_socket(port);
	printf("Server listening for connections...");

	// accept incoming connections to this server
	while (1)
	{
		if ((connect_fd = accept(master_socket, (struct sockaddr *)&client_ip_addr, &addrlen)) < 0)
		{
			perror("accept failed");
			exit(EXIT_FAILURE);
		}

		// TODO: process requests
	}

	// make sure to close sockets to avoid using up all the FDs
	closesocket(master_socket);
}

/**
 * @name open_server_socket
 * @brief Create and bind the TCP server socket, then put it into the listenting state
 * @param port The port number to connect to
 * @returns The server socket file descriptor
 */
int open_server_socket(int port)
{
	int socket_fd;
	int opt_val = 1;
	struct sockaddr_in server_ip_addr;

	// socket creation for IPv4 protocol, TCP, IP
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// set socket options for socketfd to reuse address and port
	// else have to wait ~2 minutes before reusing the same port
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt_val, sizeof(opt_val)))
	{
		perror("setsockopt failed");
		exit(EXIT_FAILURE);
	}

	// set the IP address and port number for this server
	server_ip_addr.sin_family = AF_INET;
	server_ip_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	server_ip_addr.sin_port = htons(port);

	// attach socket to the port
	if (bind(socket_fd, (struct sockaddr *)&server_ip_addr, sizeof(server_ip_addr)))
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	// Put server in the listening state and set queue length
	if (listen(socket_fd, QUEUE_LENGTH) < 0)
	{
		perror("listen failed");
		exit(EXIT_FAILURE);
	}

	// success - return the successful file descriptor
	return socket_fd;
}
#pragma endregion Helpers