#ifndef CHATKEY_CLIENT
#define CHATKEY_CLIENT

void run_client();
int open_client_socket();
DWORD WINAPI run_communication_thread(LPVOID lp_client_socket);
void handle_communication_to_server(int client_socket);
void listen_for_messages(int client_socket);

#endif