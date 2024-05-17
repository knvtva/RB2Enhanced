/*
    RB2Enhanced - net.h
    Platform-agnostic networking functionality.
*/

typedef enum _RB2E_SocketTypes
{
    RB2E_TYPE_TCP = 0,
    RB2E_TYPE_UDP = 1
} RB2E_SocketTypes;

int RB2E_CreateSocket(int protocol);
void RB2E_DisposeSocket(int socket);
int RB2E_LastError();
int RB2E_BindPort(int socket, unsigned short port);
int RB2E_SetNonBlocking(int socket, int enable);
int RB2E_SetRecvTimeout(int socket, int timeout_ms);
int RB2E_SetSendTimeout(int socket, int timeout_ms);
int RB2E_SetTimeout(int socket, int timeout_ms);
int RB2E_UDP_SendTo(int socket, unsigned int ipv4, unsigned short port, void *data, int size);
int RB2E_UDP_RecvFrom(int socket, unsigned int *ipv4, unsigned short *port, void *data, int size);
int RB2E_TCP_Connect(int socket, unsigned int ipv4, unsigned short port);
int RB2E_TCP_Send(int socket, void *data, int size);
int RB2E_TCP_Recv(int socket, void *data, int size);
int RB2E_TCP_Listen(int socket);
int RB2E_TCP_Accept(int socket, unsigned int *ipv4, unsigned short *port);
unsigned int RB2E_GetInternalIP();
unsigned int RB2E_GetGatewayIP();

#ifdef RB2E_XBOX
#define AWAIT_SOCKET(...) (RB2E_LastError() == 10035)
#elif RB2E_WII
#define AWAIT_SOCKET(r) (r == -6)
#endif
