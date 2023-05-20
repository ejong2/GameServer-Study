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
    int32 errCode = ::WSAGetLastError();
    cout << cause << " ErrorCode : " << errCode << endl;
}

int main()
{
    // ���� �ʱ�ȭ. �����ϸ� 0�� ��ȯ�� ���α׷� ����
    WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

    // ����ŷ(Non-Blocking)
    
    SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET)
        return 0;

    // ���� ������ ��-���ŷ ���� ����
    // ��-���ŷ ��忡���� ���� �Լ� ȣ���� ��� �Ϸ���� ������ WSAEWOULDBLOCK ������ ��ȯ�մϴ�.
    // �̸� ���� ���α׷��� ��Ʈ��ũ ������ ��ٸ��� �ʰ� �ٸ� �۾��� ��� ������ �� �ֽ��ϴ�.
    u_long on = 1;
    if (::ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET)
        return 0;

    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    serverAddr.sin_port = ::htons(7777);

    if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        return 0;

    if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
        return 0;

    cout << "Accept" << endl;

    SOCKADDR_IN clientAddr;
    int32 addrLen = sizeof(clientAddr);

    // Ŭ���̾�Ʈ�� ������ ����ؼ� �����մϴ�.
    while (true)
    {
        SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET)
        {
            // accept �Լ��� ���ŷ �Լ�����, �츮�� ����ŷ���� ó���ϱ� ���� WSAEWOULDBLOCK ������ �߻����� ���
            // �ٷ� ���� Ŭ���̾�Ʈ�� ������ �����ϵ��� continue�մϴ�.
            if (::WSAGetLastError() == WSAEWOULDBLOCK)
                continue;

            // �� �� ���� �߻� �� ����
            break;
        }

        cout << "Client Connected!" << endl;

        // Ŭ���̾�Ʈ�κ��� �����͸� ��� �����մϴ�.
        while (true)
        {
            char recvBuffer[1000];
            int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
            if (recvLen <= SOCKET_ERROR)
            {
                // recv �Լ��� ���ŷ �Լ�����, �츮�� ����ŷ���� ó���ϱ� ���� WSAEWOULDBLOCK ������ �߻����� ���
                // �ٷ� ���� �����͸� �����ϵ��� continue�մϴ�.
                if (::WSAGetLastError() == WSAEWOULDBLOCK)
                    continue;

                // �� �� ���� �߻� �� ����
                break;
            }
            else if (recvLen == 0)
            {
                // Ŭ���̾�Ʈ�� ������ �����ϸ� �����մϴ�.
                break;
            }

            cout << "Recv Data Len = " << recvLen << endl;

            // ���� �����͸� Ŭ���̾�Ʈ�� �ٽ� �����ϴ�. (����)
            while (true)
            {
                if (::send(clientSocket, recvBuffer, recvLen, 0) == SOCKET_ERROR)
                {
                    // send �Լ��� ���ŷ �Լ�����, �츮�� ����ŷ���� ó���ϱ� ���� WSAEWOULDBLOCK ������ �߻����� ���
                    // �ٷ� ���� �����͸� �����ϵ��� continue�մϴ�.
                    if (::WSAGetLastError() == WSAEWOULDBLOCK)
                        continue;

                    // �� �� ���� �߻� �� ����
                    break;
                }
                cout << "Send Data ! Len = " << recvLen << endl;
                break;
            }
        }
    }

    // ��� ����� ��ģ �Ŀ��� ������ �����մϴ�.
    ::WSACleanup();
}