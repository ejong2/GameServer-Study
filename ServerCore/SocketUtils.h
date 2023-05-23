#pragma once
#include "NetAddress.h"

/*----------------
    SocketUtils
-----------------*/

class SocketUtils
{
public:
    // Ȯ�� ���� �Լ� ������ ����
    static LPFN_CONNECTEX		ConnectEx;
    static LPFN_DISCONNECTEX	DisconnectEx;
    static LPFN_ACCEPTEX		AcceptEx;

public:
    // Ŭ���� �޼��� ����
    static void Init();
    static void Clear();

    static bool BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn);
    static SOCKET CreateSocket();

    // ���� �ɼ� ���� �޼��� ����
    static bool SetLinger(SOCKET socket, uint16 onoff, uint16 linger);
    static bool SetReuseAddress(SOCKET socket, bool flag);
    static bool SetRecvBufferSize(SOCKET socket, int32 size);
    static bool SetSendBufferSize(SOCKET socket, int32 size);
    static bool SetTcpNoDelay(SOCKET socket, bool flag);
    static bool SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);

    // ���� �ּ� ���� �� ������ �޼��� ����
    static bool Bind(SOCKET socket, NetAddress netAddr);
    // ���Ͽ� ��� �ּҸ� ���ε��ϴ� �޼��� ����
    static bool BindAnyAddress(SOCKET socket, uint16 port);
    // ������ ������ ���·� ����� �޼��� ����
    static bool Listen(SOCKET socket, int32 backlog = SOMAXCONN);
    // ������ �ݴ� �޼��� ����
    static void Close(SOCKET& socket);
};

// ���� �ɼ��� �����ϴ� ���ø� �Լ� ����
template<typename T>
static inline bool SetSockOpt(SOCKET socket, int32 level, int32 optName, T optVal)
{
    return SOCKET_ERROR != ::setsockopt(socket, level, optName, reinterpret_cast<char*>(&optVal), sizeof(T));
}