#ifndef CHATKEY_SERVER
#define CHATKEY_SERVER

#define INACTIVE_TIMEOUT 30000
#define MAX_CLIENTS 5
#define QUEUE_LENGTH 5

struct chatkey_server
{
  int num_clients;
  int client_sockets[MAX_CLIENTS];
  HANDLE client_threads[MAX_CLIENTS];
  clock_t last_activity;
};

void run_server();
int open_server_socket();
void add_client(int);
void remove_client(int);
void broadcast_message(char *);
void handle_communication_to_client();

#endif