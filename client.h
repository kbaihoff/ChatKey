#ifndef CHATKEY_CLIENT
#define CHATKEY_CLIENT

#define SERVER_PROCESS 0

void run_client();
int open_client_socket();
void handle_communication_to_server(int client_socket);

#endif