/**
 * FILE: server.c
 * MODULE: ChatKey
 * SUMMARY: Windows TCP server for the ChatKey program
 * FUNCTIONS:
 * AUTHOR: Kai Hoffman
 * DATE: February 06, 2021
 */

#pragma comment(lib, "Ws2_32.lib")

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include "ChatKey.h"
#include "server.h"

/**
 * @name main
 * @brief Entry point for ChatKeyServer
 * @param argc The number of arguments (including .\ChatKeyServer) that were entered on the command line
 * @param argv The arguments that were entered on the command line
 * @returns 0 on successful exit
 */
int main(int argc, char **argv)
{
	run_server();
	exit(EXIT_SUCCESS);
}

#pragma region Initialization
/**
 * @name run_server
 * @brief Run the server for clients to connect to
 */
void run_server()
{
	WSADATA wsa_data;
	int connect_fd;
	struct sockaddr_in client_ip_addr;
	int addrlen = sizeof(client_ip_addr);

	// initialize Winsock (need this for socket functions to work)
	WSAStartup(MAKEWORD(2, 2), &wsa_data);

	// open a server socket for clients to connect to
	int master_socket = open_server_socket(CHATKEY_PORT);
	printf("Server listening for connections...");

	// accept incoming connections to this server
	while (1)
	{
		if ((connect_fd = accept(master_socket, (struct sockaddr *)&client_ip_addr, &addrlen)) < 0)
		{
			fprintf(stderr, "accept failed with WSA error %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		// TODO: process requests
		printf("Server accepted client: %d", connect_fd);
	}

	// socket clean up
	if (closesocket(master_socket) == SOCKET_ERROR)
	{
		fprintf(stderr, "closesocket failed with WSA error %d", WSAGetLastError());
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	WSACleanup();
}

/**
 * @name open_server_socket
 * @brief Create and bind the TCP server socket, then put it into the listenting state
 * @returns The server socket file descriptor
 */
int open_server_socket()
{
	int socket_fd;
	int opt_val = 1;
	struct sockaddr_in server_ip_addr;

	// socket creation for IPv4 protocol, TCP, IP
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		fprintf(stderr, "socket failed with WSA error %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	// set socket options for socketfd to reuse address and port
	// else have to wait ~2 minutes before reusing the same port
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt_val, sizeof(opt_val)))
	{
		fprintf(stderr, "setsockopt failed with WSA error %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	// set the IP address and port number for this server
	server_ip_addr.sin_family = AF_INET;
	server_ip_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	server_ip_addr.sin_port = htons(CHATKEY_PORT);

	// attach socket to the port
	if (bind(socket_fd, (struct sockaddr *)&server_ip_addr, sizeof(server_ip_addr)))
	{
		fprintf(stderr, "bind failed with WSA error %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	// Put server in the listening state and set queue length
	if (listen(socket_fd, QUEUE_LENGTH) < 0)
	{
		fprintf(stderr, "listen failed with WSA error %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	// success - return the successful file descriptor
	return socket_fd;
}
#pragma endregion Initialization