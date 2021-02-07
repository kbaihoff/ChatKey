#ifndef CHATKEY_SERVER
#define CHATKEY_SERVER

#define QUEUE_LENGTH 5
#define USAGE "Usage: .\\ChatKeyServer.exe <port>"

void run_server(int port);
int open_server_socket(int port);

#endif