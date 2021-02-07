#ifndef CHATKEY
#define CHATKEY

#define CHATKEY_PORT 8888
#define MAX_BUFFER 128

void send_message(int client_socket, char *buffer);
int stop_communication(char *buffer);

#endif