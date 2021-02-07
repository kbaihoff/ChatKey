#ifndef CHATKEY_CLIENT
#define CHATKEY_CLIENT

void run_client();
int open_client_socket();
void handle_communication_to_server();
void listen_for_messages();

#endif