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

    // ���ǵ��� ������ ���͸� �����մϴ�.
    vector<Session> sessions;
    sessions.reserve(100);

    // select �Լ��� ����ϱ� ���� fd_set �������� �����մϴ�.
    fd_set reads;
    fd_set writes;

    while (true)
    {
        // fd_set �������� �ʱ�ȭ�մϴ�.
        FD_ZERO(&reads);
        FD_ZERO(&writes);

        // ListenSocket�� �б� set�� ����մϴ�.
        FD_SET(listenSocket, &reads);

        // ������ ���ϵ��� �б� Ȥ�� ���� set�� ����մϴ�.
        for (Session& s : sessions)
        {
            if (s.recvBytes <= s.sendBytes)
                FD_SET(s.socket, &reads);
            else
                FD_SET(s.socket, &writes);
        }

        // select �Լ��� ȣ���Ͽ� ��ϵ� ���ϵ� �� read Ȥ�� write ������ ������ �ִ��� Ȯ���մϴ�.
        int32 retVal = ::select(0, &reads, &writes, nullptr, nullptr);
        if (retVal == SOCKET_ERROR)
            break;

        // ListenSocket�� üũ�Ͽ� ���ο� Ŭ���̾�Ʈ�� �����ߴ��� Ȯ���մϴ�.
        if (FD_ISSET(listenSocket, &reads))
        {
            SOCKADDR_IN clientAddr;
            int32 addrLen = sizeof(clientAddr);
            SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
            if (clientSocket != INVALID_SOCKET)
            {
                cout << "Client Connected" << endl;
                sessions.push_back(Session{ clientSocket });
            }
        }
        // ���� ���ϵ��� üũ�Ͽ� �����Ͱ� �б� Ȥ�� ���� ������ �������� Ȯ���մϴ�.
        for (Session& s : sessions)
        {
            // Read
            if (FD_ISSET(s.socket, &reads))
            {
                int32 recvLen = ::recv(s.socket, s.recvBuffer, BUFSIZE, 0);
                if (recvLen <= 0)
                {
                    // �����͸� ���� ���߰ų� ������ ������ ��� ������ �����ؾ� �մϴ�.
                    continue;
                }
                s.recvBytes = recvLen;
            }

            // Write
            if (FD_ISSET(s.socket, &writes))
            {
                // ���ŷ ��� -> ��� ������ �� ����
                // ����ŷ ��� -> �Ϻθ� ���� ���� ����(���� ���� ���� ��Ȳ�� ����)
                int32 sendLen = ::send(s.socket, &s.recvBuffer[s.sendBytes], s.recvBytes - s.sendBytes, 0);
                if (sendLen == SOCKET_ERROR)
                {
                    // �����͸� ������ ���߰ų� ������ ������ ��� ������ �����ؾ� �մϴ�.
                    continue;
                }
                s.sendBytes += sendLen;
                if (s.recvBytes == s.sendBytes)
                {
                    s.recvBytes = 0;
                    s.sendBytes = 0;
                }
            }
        }
    }

    // ��� ����� ��ģ �Ŀ��� ������ �����մϴ�.
    ::WSACleanup();
}