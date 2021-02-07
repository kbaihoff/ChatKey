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

int main(int argc, char** argv) {
	int master_socket = open_server_socket(8080);
	exit(EXIT_SUCCESS);
}

#pragma region Helpers
/**
 * @name open_server_socket
 * @brief Create and bind the TCP server socket, then put it into the listenting state
 * @param port The port number to connect to
 * @returns The server socket file descriptor
 */
int open_server_socket(int port) {
	int socket_fd;
	int opt_val = 1;
	struct sockaddr_in server_ip_addr;

	// socket creation for IPv4 protocol, TCP, IP
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket failed...");
		exit(EXIT_FAILURE);
	}

	// set socket options for socketfd to reuse address and port
	// else have to wait ~2 minutes before reusing the same port
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt_val, sizeof(opt_val))) {
		perror("setsockopt failed...");
		exit(EXIT_FAILURE);
	}

	// set the IP address and port number for this server
	server_ip_addr.sin_family = AF_INET;
	server_ip_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	server_ip_addr.sin_port = htons(port);

	// attach socket to the port
	if (bind(socket_fd, (struct sockaddr*)&server_ip_addr, sizeof(server_ip_addr))) {
		perror("bind failed...");
		exit(EXIT_FAILURE);
	}

	// Put server in the listening state and set queue length
	if (listen(socket_fd, QUEUE_LENGTH) < 0) {
		perror("listen failed");
		exit(EXIT_FAILURE);
	}

	printf("Socket created succeeded!");
	return socket_fd;
}
#pragma endregion Helpers