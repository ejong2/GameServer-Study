#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

// ������ �߻��� ��� ���� �޽����� ���� �ڵ带 ���
void HandleError(const char* cause)
{
    // ���������� �߻��� ���� �ڵ带 �����ɴϴ�.
    int32 errCode = ::WSAGetLastError();
    // ���� ���ΰ� ���� �ڵ带 ����մϴ�.
    cout << cause << " ErrorCode : " << errCode << endl;
}

const int32 BUFSIZE = 1000;

struct Session
{
    // Ŭ���̾�Ʈ�� ����� ���� ������ �����մϴ�.
    SOCKET socket = INVALID_SOCKET;
    // �����͸� ���� �����Դϴ�.
    char recvBuffer[BUFSIZE] = {};
    // ���ŵ� �������� ũ�⸦ �����մϴ�.
    int32 recvBytes = 0;
    // ���� �������� ũ�⸦ �����մϴ�.
    int32 sendBytes = 0;
};

int main()
{
    // Winsock�� �ʱ�ȭ�մϴ�. ���� �� 0�� ��ȯ�Ͽ� ���α׷��� �����մϴ�.
    WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

    // ����ŷ ������ �����մϴ�.
    SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET)
        return 0;

    // ���� ������ ����ŷ ���� �����մϴ�.
    u_long on = 1;
    if (::ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET)
        return 0;

    // ���� ������ ���ε��մϴ�. ���� �� 0�� ��ȯ�Ͽ� ���α׷��� �����մϴ�.
    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    serverAddr.sin_port = ::htons(7777);

    if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        return 0;

    // ���� ������ ������ ���·� ����ϴ�. ���� �� 0�� ��ȯ�Ͽ� ���α׷��� �����մϴ�.
    if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
        return 0;

    cout << "Accept" << endl;

    // WSAEVENT ������ ���͸� �����մϴ�. �� �̺�Ʈ�� ������ ��Ʈ��ũ �̺�Ʈ�� �����մϴ�.
    vector<WSAEVENT> wsaEvents; 

    // Session ������ ���͸� �����մϴ�. �� ������ Ŭ���̾�Ʈ�� ����� ������ ������ ó���մϴ�.
    vector<Session> sessions; 
    sessions.reserve(100);  // �ִ� 100���� ������ ������ �� �ִ� ������ �����մϴ�.

    WSAEVENT listenEvent = ::WSACreateEvent();  // ���ο� �̺�Ʈ ��ü�� �����մϴ�. �� �̺�Ʈ�� ������ ���Ͽ��� ��Ʈ��ũ �̺�Ʈ�� �����մϴ�.
    wsaEvents.push_back(listenEvent);  // ������ �̺�Ʈ ��ü�� wsaEvents ���Ϳ� �߰��մϴ�.
    sessions.push_back(Session{ listenSocket });  // ���ο� ������ �����Ͽ� sessions ���Ϳ� �߰��մϴ�. �� ������ listenSocket�� ����Ͽ� �����͸� ó���մϴ�.
    if (::WSAEventSelect(listenSocket, listenEvent, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)  // �̺�Ʈ�� Ư�� ���Ͽ� �����ϰ�, � ��Ʈ��ũ �̺�Ʈ�� ������ ������ �����մϴ�. ���⼭�� ���� ����(FD_ACCEPT)�� ���� ����(FD_CLOSE) �̺�Ʈ�� �����մϴ�.
        return 0;

    while (true)
    { 
        // ���� �̺�Ʈ �� �ϳ��� �߻��� ������ ��ٸ��ϴ�. �߻��� �̺�Ʈ�� �ε����� ��ȯ�մϴ�.
        int32 index = ::WSAWaitForMultipleEvents(wsaEvents.size(), &wsaEvents[0], FALSE, WSA_INFINITE, FALSE);
        if(index == WSA_WAIT_FAILED)
            continue;

        index -= WSA_WAIT_EVENT_0;

        // �߻��� �̺�Ʈ�� ���� ��Ʈ��ũ �̺�Ʈ�� ���� �ڵ带 �����ɴϴ�.
        WSANETWORKEVENTS networkEvents;
        if(::WSAEnumNetworkEvents(sessions[index].socket, wsaEvents[index], &networkEvents) == SOCKET_ERROR)
            continue;

        // Listener ���� üũ
        if (networkEvents.lNetworkEvents & FD_ACCEPT)
        {
            // Error-Check
            if(networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
                continue;

            SOCKADDR_IN clientAddr;
            int32 addrLen = sizeof(clientAddr);

            SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
            if (clientSocket != INVALID_SOCKET)
            {
                cout << "Client Connected" << endl;

                WSAEVENT clientEvent = WSACreateEvent();
                wsaEvents.push_back(clientEvent);
                sessions.push_back(Session{ clientSocket });
                if(::WSAEventSelect(clientSocket, clientEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
                    return 0;
            }
        }
        // Client Session ���� üũ
        if (networkEvents.lNetworkEvents & FD_READ || networkEvents.lNetworkEvents & FD_WRITE)
        {
            // Error - Check
            if((networkEvents.lNetworkEvents & FD_READ) && (networkEvents.iErrorCode[FD_READ_BIT] != 0))
                continue;
            // Error - Check
            if ((networkEvents.lNetworkEvents & FD_WRITE) && (networkEvents.iErrorCode[FD_WRITE_BIT] != 0))
                continue;

            Session& s = sessions[index];

            // Read
            if (s.recvBytes == 0)
            {
                int32 recvLen = ::recv(s.socket, s.recvBuffer, BUFSIZE, 0);
                if (recvLen == SOCKET_ERROR && ::WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    // TODO : Remove Session
                    continue;
                }
                s.recvBytes = recvLen;
                cout << "Recv Data = " << recvLen << endl;
            }
            
            // Write
            if (s.recvBytes > s.sendBytes)
            {
                int32 sendLen = ::send(s.socket, &s.recvBuffer[s.sendBytes], s.recvBytes - s.sendBytes, 0);
                if (sendLen == SOCKET_ERROR && ::GetLastError() != WSAEWOULDBLOCK)
                {
                    // TODO : Remove Session
                    continue;
                }
                s.sendBytes += sendLen;
                if (s.recvBytes == s.sendBytes)
                {
                    s.recvBytes = 0;
                    s.sendBytes = 0;
                }
                cout << "Send Data = " << sendLen << endl;
            }
        }
        // FD_CLOSE ó��
        if (networkEvents.lNetworkEvents & FD_CLOSE)
        {
            // TODO : Remove Socket
        }
    }

    // ��� ����� ��ģ �Ŀ��� ������ �����մϴ�.
    ::WSACleanup();
}