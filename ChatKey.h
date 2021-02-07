#ifndef CHATKEY
#define CHATKEY

#define CHATKEY_PORT 8888
#define DISCONNECT_CLIENT_MSG "\t\r\n"
#define EXIT_MSG "exit"
#define MAX_BUFFER 128
#define QUIT_MSG "quit"

HANDLE create_communication_thread(int client_socket);
void send_message(int client_socket, char *buffer);
int stop_communication(char *buffer);
void cleanup_communication_thread(HANDLE thread_handle);

#endif