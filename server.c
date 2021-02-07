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

/**
 * Global server struct for this file
 */
struct chatkey_server cks;

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
	cks.num_clients = 0;
	cks.last_activity = clock();
	printf("Server listening for connections...\n");

	// accept incoming connections to this server
	while (difftime(clock(), cks.last_activity) < INACTIVE_TIMEOUT)
	{
		if (cks.num_clients < MAX_CLIENTS)
		{
			if ((connect_fd = accept(server_socket, (struct sockaddr *)&client_ip_addr, &addrlen)) < 0)
			{
				fprintf(stderr, "accept failed with WSA error %d\n", WSAGetLastError());
				exit(EXIT_FAILURE);
			}

			// create thread to handle messaging to/from this client
			add_client(connect_fd);

			// new activity - update the timestamp
			cks.last_activity = clock();
		}
	}

	// client clean up
	for (client_idx = 0; client_idx < cks.num_clients; client_idx++) {
		cleanup_communication_thread(cks.client_threads[client_idx]);
		closesocket(cks.client_sockets[client_idx]);
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

/**
 * @name create_communication_thread
 * @brief Create a thread to listen for and broadcast messages from the clients
 * @param client_socket The socket the client is connected to
 * @returns The thread handle
 */
HANDLE create_communication_thread(int client_socket)
{
	HANDLE thread_handle;
	memset(&thread_handle, 0, sizeof(thread_handle));

	// Default security attributes
	// Default stack size
	// Thread should run run_communication_thread
	// Pass client_socket as parameter to run_communication_thread
	// Default creation flags
	// Don't save the thread identifier
	thread_handle = CreateThread(NULL, 0, run_communication_thread, &client_socket, 0, NULL);
	if (thread_handle == NULL)
	{
		fprintf(stderr, "CreateThread failed with WSA error %d\n", WSAGetLastError());
	}

	// success - return the successful thread handle
	return thread_handle;
}

/**
 * @name run_communication_thread
 * @brief Wrapper function to handle_communication_to_client; threads should call this function
 * @param lp_client_socket The the LP parameter of the socket this client is connected to
 */
DWORD WINAPI run_communication_thread(LPVOID lp_client_socket)
{
	int client_socket = *(int *)(lp_client_socket);
	handle_communication_to_client(client_socket);
	printf("Client %d is disconnecting...\n", client_socket);
	return EXIT_SUCCESS;
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
	printf("Server accepted client: %d\n", client_socket);
	broadcast_message("A new user has joined the group!\n");
	cks.client_sockets[cks.num_clients] = client_socket;
	cks.client_threads[cks.num_clients] = create_communication_thread(client_socket);
	cks.num_clients++;
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
	for (client_idx = 0; client_idx < cks.num_clients; client_idx++)
	{
		if (cks.client_sockets[client_idx] == client_socket)
		{
			break;
		}
	}

	// cleanup thread
	cleanup_communication_thread(cks.client_threads[client_idx]);

	// shift client sockets and threads
	for (shift_idx = client_idx + 1; shift_idx < cks.num_clients; shift_idx++) {
		cks.client_sockets[shift_idx - 1] = cks.client_sockets[shift_idx];
		cks.client_threads[shift_idx - 1] = cks.client_threads[shift_idx];
	}
	cks.num_clients--;
}
#pragma endregion ServerManagement

#pragma region Messaging
/**
 * @name broadcast_message
 * @brief Send a message to all connected clients
 * @param cks The ChatKey server that should broadcast the messages
 * @param msg The message to broadcast
 */
void broadcast_message(char *msg)
{
	int i;
	for (i = 0; i < cks.num_clients; i++)
	{
		send_message(cks.client_sockets[i], msg);
	}
}

/**
 * @name send_message
 * @brief Send a message over the server
 * @param client_socket The socket this client is connected to
 * @param buffer The messaget to send
 */
void send_message(int client_socket, char *buffer)
{
	if (send(client_socket, buffer, sizeof(buffer), 0) == SOCKET_ERROR)
	{
		fprintf(stderr, "send failed with WSA error %d\n", WSAGetLastError());
		return;
	}
}

/**
 * @name handle_communication_to_client
 * @brief Handle messaging with one client
 * @param client_fd The client socket file descriptor
 */
void handle_communication_to_client(int client_fd)
{
	int bytes_read;
	char buffer[MAX_BUFFER];

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

/**
 * @name stop_communication
 * @brief Determine whether client wants to stop communication
 * @param buffer The message from the client
 * @returns 1 if the client wants to leave, otherwise 0
 */
int stop_communication(char *buffer)
{
	if (strncmp(buffer, EXIT_MSG, strlen(EXIT_MSG)) == 0)
	{
		return 1;
	}
	if (strncmp(buffer, QUIT_MSG, strlen(QUIT_MSG)) == 0)
	{
		return 1;
	}
	if (strncmp(buffer, DISCONNECT_CLIENT_MSG, strlen(DISCONNECT_CLIENT_MSG)) == 0)
	{
		return 1;
	}
	return 0;
}
#pragma endregion Messaging

#pragma region Cleanup
/**
 * @name cleanup_communication_thread
 * @brief Clean up thread resources and close handles
 * @param thread_handle The thread to close
 */
void cleanup_communication_thread(HANDLE thread_handle)
{
	CloseHandle(thread_handle);
}
#pragma endregion Cleanup