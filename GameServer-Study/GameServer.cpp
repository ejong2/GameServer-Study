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

    // SO_KEEPALIVE: �� �ɼ��� �����ϸ� TCP ������ ���°� �ֱ������� Ȯ�ε˴ϴ�.
    // ���� ���, ���� ������ ������ �Ҹ��ҹ� ���� �����ٸ� �� �ɼ��� ���� ������ �� �ֽ��ϴ�.
    bool enable = true;
    ::setsockopt(serverSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&enable, sizeof(enable));

    // SO_LINGER: �� �ɼ��� ������ ���� �� ���� ���۵��� ���� �����Ͱ� �ִ� ��� ��� ó�������� �����մϴ�.
    // linger.l_onoff �� 0�̸� closesocket()�� �ٷ� �����ϰ�, �ƴϸ� linger.l_linger �� ��ŭ ����մϴ�. 
    LINGER linger;
    linger.l_onoff = 1;
    linger.l_linger = 5;
    ::setsockopt(serverSocket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));

    // SO_SNDBUF �� SO_RCVBUF: �̵� �ɼ��� �۽� �� ���� ������ ũ�⸦ �����մϴ�.
    int32 sendBufferSize;
    int32 optionLen = sizeof(sendBufferSize);
    ::getsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, &optionLen);
    cout << "�۽� ���� ũ�� : " << sendBufferSize << endl;

    int32 recvBufferSize;
    optionLen = sizeof(sendBufferSize);
    ::getsockopt(serverSocket, SOL_SOCKET, SO_RCVBUF, (char*)&recvBufferSize, &optionLen);
    cout << "���� ���� ũ�� : " << recvBufferSize << endl;

    // SO_REUSEADDR: �� �ɼ��� ����ϸ� ������ ������ �ּҿ� ��Ʈ ��ȣ�� ������ �� �ֽ��ϴ�.
    {
        bool enable = true;
        ::setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(enable));
    }

    // TCP_NODELAY: �� �ɼ��� Nagle �˰����� Ȱ��ȭ �Ǵ� ��Ȱ��ȭ �մϴ�.
   // Nagle �˰����� �����Ͱ� ����� ũ�� ������, �׷��� ������ �����Ͱ� ����� ���� ������ ����մϴ�.
   // �� �ɼ��� true �����ϸ� ���ӿ��� ���� ���� �ð��� �ʿ�� �ϴ� ���(���� ��� �ǽð� FPS ����) ������ �� �ֽ��ϴ�.
   // �׷���, �� �ɼ��� Ȱ��ȭ�ϸ� ��Ʈ��ũ �󿡼� ���� ��Ŷ�� ���ʿ��ϰ� ���� �߻��ϴ� ���� ������ �� �����ϴ�.
    {
        bool enable = true;
        ::setsockopt(serverSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(enable));
    }

    // ���� ����� ��ġ�� ������ �����մϴ�.
    ::WSACleanup();
}