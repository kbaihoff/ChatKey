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
	int server_socket, connect_fd, pid;
	struct sockaddr_in client_ip_addr;
	int addrlen = sizeof(client_ip_addr);

	// initialize Winsock (need this for socket functions to work)
	WSAStartup(MAKEWORD(2, 2), &wsa_data);

	// open a server socket for clients to connect to
	server_socket = open_server_socket(CHATKEY_PORT);
	printf("Server listening for connections...\n");

	// accept incoming connections to this server
	while (1)
	{
		if ((connect_fd = accept(server_socket, (struct sockaddr *)&client_ip_addr, &addrlen)) < 0)
		{
			fprintf(stderr, "accept failed with WSA error %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		// fork child process to handle messaging to/from this client
		printf("Server accepted client: %d\n", connect_fd);
		//pid = fork();
		//if (pid == CLIENT_PROCESS)
		//{
		//closesocket(server_socket);
		handle_communication_to_client(connect_fd);
		printf("Client %d is disconnecting...\n", connect_fd);
		//exit(0);
		//}
		//else
		//{
		// closesocket(connect_fd);
		//}
	}

	// socket clean up
	if (closesocket(server_socket) == SOCKET_ERROR)
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
	memset(&server_ip_addr, 0, sizeof(server_ip_addr));
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

#pragma region Messaging
/**
 * @name broadcast_message
 * @brief Send a message to all connected clients
 * @param cks The ChatKey server that should broadcast the messages
 * @param msg The message to broadcast
 */
void broadcast_message(struct chatkey_server cks, char *msg)
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
    fprintf(stderr, "send failed with WSA error %d", WSAGetLastError());
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
			fprintf(stderr, "recv failed with WSA error %d", WSAGetLastError());
		}
		else if (bytes_read > 0)
		{
			printf("From client: %s", buffer);
			send_message(client_fd, "Got your message!");
		}
		if (stop_communication(buffer))
		{
			send_message(client_fd, DISCONNECT_CLIENT_MSG);
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