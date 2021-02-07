/**
 * FILE: client.c
 * MODULE: ChatKey
 * SUMMARY: Windows TCP client for the ChatKey program
 * FUNCTIONS:
 * AUTHOR: Kai Hoffman
 * DATE: February 06, 2021
 */

#pragma comment(lib,"Ws2_32.lib")

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <WinSock2.h>
#include "server.h"

/**
 * @name main
 * @brief Entry point for ChatKeyClient
 * @param argc The number of arguments (including .\ChatKeyClient) that were entered on the command line
 * @param argv The arguments that were entered on the command line
 * @returns 0 on successful exit
 */
int main(int argc, char **argv) {
  WSADATA wsa_data;
  int socket_fd;
	struct sockaddr_in server_ip_addr;

  // initialize Winsock (need this for socket functions to work)
	WSAStartup(MAKEWORD(2, 2), &wsa_data);

	// socket creation for IPv4 protocol, TCP, IP
	if ((socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
	{
		fprintf(stderr, "socket failed [%d]", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	// set the port number for this server on localhost
	server_ip_addr.sin_family = AF_INET;
	server_ip_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	server_ip_addr.sin_port = htons(8080);

  // connect the client socket to the server socket
	if (connect(socket_fd, (struct sockaddr *)&server_ip_addr, sizeof(server_ip_addr)) != 0)
	{
		fprintf(stderr, "connect failed [%d]", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	// socket clean up
	if (closesocket(socket_fd) == SOCKET_ERROR) {
		fprintf(stderr, "closesocket failed [%d]", WSAGetLastError());
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	WSACleanup();
}