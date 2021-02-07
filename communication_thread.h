#ifndef CHATKEY_COMTHREAD
#define CHATKEY_COMTHREAD

HANDLE create_communication_thread(void (*)());
DWORD WINAPI run_communication_thread(LPVOID);
void cleanup_communication_thread(HANDLE);

#endif