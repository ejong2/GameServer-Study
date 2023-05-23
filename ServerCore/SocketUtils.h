#pragma once
#include "NetAddress.h"

/*----------------
    SocketUtils
-----------------*/

class SocketUtils
{
public:
    // 확장 소켓 함수 포인터 정의
    static LPFN_CONNECTEX		ConnectEx;
    static LPFN_DISCONNECTEX	DisconnectEx;
    static LPFN_ACCEPTEX		AcceptEx;

public:
    // 클래스 메서드 정의
    static void Init();
    static void Clear();

    static bool BindWindowsFunction(SOCKET socket, GUID guid, LPVOID* fn);
    static SOCKET CreateSocket();

    // 소켓 옵션 설정 메서드 정의
    static bool SetLinger(SOCKET socket, uint16 onoff, uint16 linger);
    static bool SetReuseAddress(SOCKET socket, bool flag);
    static bool SetRecvBufferSize(SOCKET socket, int32 size);
    static bool SetSendBufferSize(SOCKET socket, int32 size);
    static bool SetTcpNoDelay(SOCKET socket, bool flag);
    static bool SetUpdateAcceptSocket(SOCKET socket, SOCKET listenSocket);

    // 소켓 주소 설정 및 리스닝 메서드 정의
    static bool Bind(SOCKET socket, NetAddress netAddr);
    // 소켓에 모든 주소를 바인드하는 메서드 정의
    static bool BindAnyAddress(SOCKET socket, uint16 port);
    // 소켓을 리스닝 상태로 만드는 메서드 정의
    static bool Listen(SOCKET socket, int32 backlog = SOMAXCONN);
    // 소켓을 닫는 메서드 정의
    static void Close(SOCKET& socket);
};

// 소켓 옵션을 설정하는 템플릿 함수 정의
template<typename T>
static inline bool SetSockOpt(SOCKET socket, int32 level, int32 optName, T optVal)
{
    return SOCKET_ERROR != ::setsockopt(socket, level, optName, reinterpret_cast<char*>(&optVal), sizeof(T));
}