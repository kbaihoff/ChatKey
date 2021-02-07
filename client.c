/**
 * FILE: client.c
 * MODULE: ChatKey
 * SUMMARY: Windows TCP client for the ChatKey program
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
#include "client.h"

/**
 * @name main
 * @brief Entry point for ChatKeyClient
 * @param argc The number of arguments (including .\ChatKeyClient) that were entered on the command line
 * @param argv The arguments that were entered on the command line
 * @returns 0 on successful exit
 */
int main(int argc, char **argv)
{
  run_client();
  exit(EXIT_SUCCESS);
}

#pragma region Initialization
/**
 * @name run_client
 * @brief Run and connect the client to the server
 */
void run_client()
{
  WSADATA wsa_data;
  int client_socket, pid;

  // initialize Winsock (need this for socket functions to work)
  WSAStartup(MAKEWORD(2, 2), &wsa_data);

  // open client socket and connect to server
  client_socket = open_client_socket();
  printf("Type a message to send to everyone on the ChatKey server.\n");

  // fork child process to handle messaging to/from this server
  //pid = fork();
  //if (pid == SERVER_PROCESS)
  //{
  //closesocket(server_socket);
  handle_communication_to_server(client_socket);
  printf("Client %d is disconnecting...", client_socket);
  //exit(0);
  //}
  //else
  //{
  // closesocket(connect_fd);
  //}

  // socket clean up
  if (closesocket(client_socket) == SOCKET_ERROR)
  {
    fprintf(stderr, "closesocket failed with WSA error %d", WSAGetLastError());
    WSACleanup();
    exit(EXIT_FAILURE);
  }
  WSACleanup();
}

/**
 * @name open_client_socket
 * @brief Create the TCP client socket, then connect it to the server
 * @returns The client socket file descriptor
 */
int open_client_socket()
{
  int socket_fd;
  struct sockaddr_in server_ip_addr;

  // socket creation for IPv4 protocol, TCP, IP
  if ((socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
  {
    fprintf(stderr, "socket failed with WSA error %d", WSAGetLastError());
    exit(EXIT_FAILURE);
  }

  // set the port number for this server on localhost
  memset(&server_ip_addr, 0, sizeof(server_ip_addr));
  server_ip_addr.sin_family = AF_INET;
  server_ip_addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
  server_ip_addr.sin_port = htons(CHATKEY_PORT);

  // connect the client socket to the server socket
  if (connect(socket_fd, (struct sockaddr *)&server_ip_addr, sizeof(server_ip_addr)) != 0)
  {
    fprintf(stderr, "connect failed with WSA error %d", WSAGetLastError());
    exit(EXIT_FAILURE);
  }

  // success - return the successful file descriptor
  return socket_fd;
}
#pragma endregion Initialization

#pragma region Messaging
/**
 * @name handle_communication_to_server
 * @brief Handle sending messages to the server
 * @param client_socket The socket this client is connected to
 */
void handle_communication_to_server(int client_socket)
{
  int len;
  char buffer[MAX_BUFFER];

  while (1)
  {
    memset(buffer, 0, sizeof(buffer));
    len = 0;
    while ((buffer[len++] = getchar()) != '\n')
      ;
    send_message(client_socket, buffer);
    if (stop_communication(buffer))
    {
      break;
    }
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
 * @name stop_communication
 * @brief Determine whether client wants to stop communication
 * @param buffer The message from the client
 * @returns 1 if the client wants to leave, otherwise 0
 */
int stop_communication(char *buffer)
{
  if (strncmp("exit", buffer, 4) == 0)
  {
    return 1;
  }
  if (strncmp("quit", buffer, 4) == 0)
  {
    return 1;
  }
  if (strncmp("stop", buffer, 4) == 0)
  {
    return 1;
  }
  if (strncmp("leave", buffer, 5) == 0)
  {
    return 1;
  }
  return 0;
}
#pragma endregion Messaging