/**
 * FILE: ChatKey.c
 * MODULE: ChatKey
 * SUMMARY: Shared client and server functions for the ChatKey program
 * FUNCTIONS:
 * AUTHOR: Kai Hoffman
 * DATE: February 07, 2021
 */

#pragma comment(lib, "Ws2_32.lib")

#include <stdio.h>
#include <string.h>
#include <WinSock2.h>
#include "ChatKey.h"

/**
 * @name send_message
 * @brief Send a message over the server
 * @param socket_fd The socket to send the message to
 * @param buffer The message to send
 */
void send_message(int socket_fd, char *buffer)
{
	if (send(socket_fd, buffer, strlen(buffer), 0) == SOCKET_ERROR)
	{
		fprintf(stderr, "send failed with WSA error %d\n", WSAGetLastError());
		return;
	}
}

/**
 * @name stop_communication
 * @brief Determine whether to stop communization
 * @param buffer The message potentially indicating communication halt
 * @returns 1 if communication should stop, otherwise 0
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