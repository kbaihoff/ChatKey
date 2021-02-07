#ifndef CHATKEY_SERVER
#define CHATKEY_SERVER

#define CLIENT_PROCESS 0
#define MAX_CLIENTS 2
#define QUEUE_LENGTH 5

struct chatkey_server
{
  int num_clients;
  int client_sockets[MAX_CLIENTS];
};

void run_server();
int open_server_socket();
void broadcast_message(struct chatkey_server cks, char *msg);
void handle_communication_to_client();

#endif