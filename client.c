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
#include <Windows.h>
#include "ChatKey.h"
#include "client.h"
#include "communication_thread.h"

/**
 * Global client socket file descriptor for this file
 */
int CLIENT_SOCKET;

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
  HANDLE thread_handle;

  // initialize Winsock (need this for socket functions to work)
  WSAStartup(MAKEWORD(2, 2), &wsa_data);

  // open client socket and connect to server
  CLIENT_SOCKET = open_client_socket();
  printf("Type a message to send to everyone on the ChatKey server.\n");

  // create thread to handle sending messages
  void (*communication_handler)() = &handle_communication_to_server;
  thread_handle = create_communication_thread(communication_handler);

  // listen for server messages on this thread
  listen_for_messages();

  // if the thread completes its function, that means the client wants to exit
  WaitForSingleObject(thread_handle, INFINITE);
  cleanup_communication_thread(thread_handle);
  printf("Client %d is disconnecting...", CLIENT_SOCKET);

  // socket clean up
  if (closesocket(CLIENT_SOCKET) == SOCKET_ERROR)
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
 */
void handle_communication_to_server()
{
  int len;
  char buffer[MAX_BUFFER];

  while (1)
  {
    memset(buffer, 0, sizeof(buffer));
    len = 0;
    while ((buffer[len++] = getchar()) != '\n')
      ;
    send_message(CLIENT_SOCKET, buffer);
    if (stop_communication(buffer))
    {
      break;
    }
  }
}

/**
 * @name listen_for_messages
 * @brief Listen for messages from the server
 */
void listen_for_messages()
{
  int bytes_read;
  char buffer[MAX_BUFFER];

  // Listen for/send messages from/to this client until they decide to leave
  while (1)
  {
    memset(buffer, 0, sizeof(buffer));
    bytes_read = recv(CLIENT_SOCKET, buffer, sizeof(buffer), 0);
    if (bytes_read == SOCKET_ERROR)
    {
      fprintf(stderr, "recv failed with WSA error %d", WSAGetLastError());
    }
    else if (bytes_read > 0)
    {
      if (stop_communication(buffer))
      {
        break;
      }
      printf("From server: %s", buffer);
    }
  }
}
#pragma endregion Messaging