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

    // UDP ���� ����. �����ϸ� ���� �޽��� ��� �� ���α׷� ����
    SOCKET serverSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        HandleError("Socket");
        return 0;
    }

    // ���� �ּ� ����. ���⼭�� ��� IP�κ����� ������ ���
    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    serverAddr.sin_port = ::htons(7777);

    // ���Ͽ� �ּ� ���ε�. �����ϸ� ���� �޽��� ��� �� ���α׷� ����
    if (::bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        HandleError("Bind");
        return 0;
    }

    while (true)
    {
        // Ŭ���̾�Ʈ�� �ּҸ� ������ ����ü
        SOCKADDR_IN clientAddr;
        ::memset(&clientAddr, 0, sizeof(clientAddr));
        int32 addrLen = sizeof(clientAddr);

        // recvBuffer�� Ŭ���̾�Ʈ�κ��� ���� �����͸� ������ ����
        char recvBuffer[1000];

        // Ŭ���̾�Ʈ�κ��� �����͸� ����. ���� �����ʹ� recvBuffer�� ����ǰ�, �������� ���̴� recvLen�� �����.
        int32 recvLen = ::recvfrom(serverSocket, recvBuffer, sizeof(recvBuffer), 0,
            (SOCKADDR*)&clientAddr, &addrLen);

        // ������ ������ �����ϰų� Ŭ���̾�Ʈ�κ��� ������ ������ ��� ���� ó��
        if (recvLen <= 0)
        {
            HandleError("RecvFrom");
            return 0;
        }

        // ������ �����Ϳ� �������� ���� ���
        cout << "Recv Data! Data = " << recvBuffer << endl;
        cout << "Recv Data! Len = " << recvLen << endl;
        // Ŭ���̾�Ʈ���� ���� �����͸� �״�� �ٽ� ���� (echo)
        int32 errorCode = ::sendto(serverSocket, recvBuffer, recvLen, 0,
            (SOCKADDR*)&clientAddr, sizeof(clientAddr));

        // ������ ������ ������ ��� ���� ó��
        if (errorCode == SOCKET_ERROR)
        {
            HandleError("SendTo");
            return 0;
        }

        // ������ �������� ���̸� ���
        cout << "Send Data! Len = " << recvLen << endl;
    }

    // ���� ����� ��ġ�� ������ ����
    ::WSACleanup();
}