/**
 * FILE: server.c
 * MODULE: ChatKey
 * SUMMARY: Windows TCP server for the ChatKey program
 * FUNCTIONS:
 * AUTHOR: Kai Hoffman
 * DATE: February 06, 2021
 */

#pragma comment(lib, "Ws2_32.lib")

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <WinSock2.h>
#include <Windows.h>
#include "ChatKey.h"
#include "server.h"
#include "communication_thread.h"

/**
 * Global server struct for this file
 */
struct chatkey_server CK_SERVER;

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
	int server_socket, connect_fd, client_idx;
	struct sockaddr_in client_ip_addr;
	int addrlen = sizeof(client_ip_addr);

	// initialize Winsock (need this for socket functions to work)
	WSAStartup(MAKEWORD(2, 2), &wsa_data);

	// open and initialize a server socket for clients to connect to
	server_socket = open_server_socket(CHATKEY_PORT);
	CK_SERVER.num_clients = 0;
	CK_SERVER.last_activity = clock();
	printf("Server listening for connections...\n");

	// accept incoming connections to this server
	while (difftime(clock(), CK_SERVER.last_activity) < INACTIVE_TIMEOUT)
	{
		if (CK_SERVER.num_clients < MAX_CLIENTS)
		{
			if ((connect_fd = accept(server_socket, (struct sockaddr *)&client_ip_addr, &addrlen)) < 0)
			{
				fprintf(stderr, "accept failed with WSA error %d\n", WSAGetLastError());
				exit(EXIT_FAILURE);
			}

			// create thread to handle messaging to/from this client
			add_client(connect_fd);

			// new activity - update the timestamp
			CK_SERVER.last_activity = clock();
		}
	}

	// client clean up
	for (client_idx = 0; client_idx < CK_SERVER.num_clients; client_idx++)
	{
		cleanup_communication_thread(CK_SERVER.client_threads[client_idx]);
		closesocket(CK_SERVER.client_sockets[client_idx]);
	}

	// server clean up
	if (closesocket(server_socket) == SOCKET_ERROR)
	{
		fprintf(stderr, "closesocket failed with WSA error %d\n", WSAGetLastError());
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
		fprintf(stderr, "socket failed with WSA error %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	// set socket options for socketfd to reuse address and port
	// else have to wait ~2 minutes before reusing the same port
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt_val, sizeof(opt_val)))
	{
		fprintf(stderr, "setsockopt failed with WSA error %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	// set the IP address and port number for this server
	memset(&server_ip_addr, 0, sizeof(server_ip_addr));
	server_ip_addr.sin_family = AF_INET;
	server_ip_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	server_ip_addr.sin_port = htons(CHATKEY_PORT);

	// attach socket to the port
	if (bind(socket_fd, (struct sockaddr *)&server_ip_addr, sizeof(server_ip_addr)))
	{
		fprintf(stderr, "bind failed with WSA error %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	// Put server in the listening state and set queue length
	if (listen(socket_fd, QUEUE_LENGTH) < 0)
	{
		fprintf(stderr, "listen failed with WSA error %d\n", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	// success - return the successful file descriptor
	return socket_fd;
}
#pragma endregion Initialization

#pragma region ServerManagement
/**
 * @name add_client
 * @brief Add a new client (socket and thread) to this server - calls create_communication_thread
 * @param client_socket The client to add
 */
void add_client(int client_socket)
{
	void (*communication_handler)() = &handle_communication_to_client;
	printf("Server accepted client: %d\n", client_socket);
	broadcast_message("A new user has joined the group!\n");
	CK_SERVER.client_sockets[CK_SERVER.num_clients] = client_socket;
	CK_SERVER.client_threads[CK_SERVER.num_clients] = create_communication_thread(communication_handler);
	CK_SERVER.num_clients++;
}

/**
 * @name remove_client
 * @brief Remove new client from this server
 * @param client_socket The client to remove
 */
void remove_client(int client_socket)
{
	int client_idx, shift_idx;

	// find client index
	for (client_idx = 0; client_idx < CK_SERVER.num_clients; client_idx++)
	{
		if (CK_SERVER.client_sockets[client_idx] == client_socket)
		{
			break;
		}
	}

	// cleanup thread
	cleanup_communication_thread(CK_SERVER.client_threads[client_idx]);

	// shift client sockets and threads
	for (shift_idx = client_idx + 1; shift_idx < CK_SERVER.num_clients; shift_idx++)
	{
		CK_SERVER.client_sockets[shift_idx - 1] = CK_SERVER.client_sockets[shift_idx];
		CK_SERVER.client_threads[shift_idx - 1] = CK_SERVER.client_threads[shift_idx];
	}
	CK_SERVER.num_clients--;
}
#pragma endregion ServerManagement

#pragma region Messaging
/**
 * @name broadcast_message
 * @brief Send a message to all connected clients
 * @param msg The message to broadcast
 */
void broadcast_message(char *msg)
{
	int i;
	for (i = 0; i < CK_SERVER.num_clients; i++)
	{
		send_message(CK_SERVER.client_sockets[i], msg);
	}
}

/**
 * @name handle_communication_to_client
 * @brief Handle messaging with one client
 */
void handle_communication_to_client()
{
	int bytes_read;
	char buffer[MAX_BUFFER];
	int client_fd = CK_SERVER.client_sockets[CK_SERVER.num_clients];

	// Listen for/send messages from/to this client until they decide to leave
	while (1)
	{
		memset(buffer, 0, sizeof(buffer));
		bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);
		if (bytes_read == SOCKET_ERROR)
		{
			fprintf(stderr, "recv failed with WSA error %d\n", WSAGetLastError());
			break;
		}
		else if (bytes_read > 0)
		{
			printf("From client: %s", buffer);
			send_message(client_fd, "Got your message!");
		}
		if (stop_communication(buffer))
		{
			send_message(client_fd, DISCONNECT_CLIENT_MSG);
			remove_client(client_fd);
			break;
		}
	}
}
#pragma endregion Messaging