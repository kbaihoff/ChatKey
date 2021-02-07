/**
 * FILE: communication_thread.c
 * MODULE: ChatKey
 * SUMMARY: Thread functions for the ChatKey program
 * FUNCTIONS:
 * AUTHOR: Kai Hoffman
 * DATE: February 07, 2021
 */

#pragma comment(lib, "Ws2_32.lib")

#include <stdio.h>
#include <Windows.h>
#include "communication_thread.h"

/**
 * @name create_communication_thread
 * @brief Create a thread to execute the specified function
 * @param function_pointer A pointer to the function for the thread to execute
 * @returns The thread handle
 */
HANDLE create_communication_thread(void (*function_pointer)())
{
  HANDLE thread_handle;
  memset(&thread_handle, 0, sizeof(thread_handle));

  // Default security attributes
  // Default stack size
  // Thread should run run_communication_thread
  // Pass function_pointer as parameter to run_communication_thread
  // Default creation flags
  // Don't save the thread identifier
  thread_handle = CreateThread(NULL, 0, run_communication_thread, function_pointer, 0, NULL);
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
DWORD WINAPI run_communication_thread(LPVOID lp_function_pointer)
{
  void (*function_pointer)() = (void (*)())(lp_function_pointer);
  (*function_pointer)();
  return EXIT_SUCCESS;
}

/**
 * @name cleanup_communication_thread
 * @brief Clean up thread resources and close handles
 * @param thread_handle The thread to close
 */
void cleanup_communication_thread(HANDLE thread_handle)
{
  CloseHandle(thread_handle);
}