#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

// Winsock ���̺귯���� ����ϱ� ���� ��� ����
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "Memory.h"

// ���� �߻� �� ���ΰ� ���� �ڵ带 ����ϴ� �Լ�
void HandleError(const char* cause)
{
    int32 errCode = ::WSAGetLastError();
    cout << cause << " ErrorCode : " << errCode << endl;
}

const int32 BUFSIZE = 1000; // ���� �����͸� ������ ������ ũ�⸦ �����մϴ�.

// ���� �����͸� ������ ����ü�Դϴ�.
struct Session
{
    SOCKET socket = INVALID_SOCKET; // Ŭ���̾�Ʈ���� ���� �����Դϴ�.
    char recvBuffer[BUFSIZE] = {}; // ���� �����͸� ������ �����Դϴ�.
    int32 recvBytes = 0; // ������ ����Ʈ ���� �����մϴ�.
};

// I/O �۾��� ������ ��Ÿ���� �������Դϴ�.
enum IO_TYPE
{
    READ,
    WRITE,
    ACCEPT,
    CONNECT,
};

// Overlapped I/O �۾��� ���� ������ ������ ����ü�Դϴ�.
struct OverlappedEx
{
    WSAOVERLAPPED overlapped = {}; // WSAOVERLAPPED ����ü�Դϴ�. �񵿱� I/O �۾��� ���¸� �����մϴ�.
    int32 type = 0; // I/O �۾��� �����Դϴ�.
};

// �۾� �������� ���� �Լ��Դϴ�. I/O �Ϸ� ��Ŷ�� ó���մϴ�.
void WorkerThreadMain(HANDLE iocpHandle)
{
    while (true)
    {
        DWORD bytesTransferred = 0;
        Session* session = nullptr;
        OverlappedEx* overlappedEx = nullptr;

        // IOCP���� �Ϸ� ��Ŷ�� �����ɴϴ�.
        BOOL ret = ::GetQueuedCompletionStatus(iocpHandle, &bytesTransferred,
            (ULONG_PTR*)&session, (LPOVERLAPPED*)&overlappedEx, INFINITE);

        // �Ϸ� ��Ŷ�� �������� �� �����ϰų� ���۵� ����Ʈ�� ���� ��� ������ ������ ������ �����մϴ�.
        if (ret == FALSE || bytesTransferred == 0)
        {
            // TODO : ���� ���� ó��
            continue;
        }

        ASSERT_CRASH(overlappedEx->type == IO_TYPE::READ); // READ Ÿ������ Ȯ���մϴ�.

        cout << "Recv Data IOCP = " << bytesTransferred << endl;

        WSABUF wsaBuf;
        wsaBuf.buf = session->recvBuffer; // ���۸� �����մϴ�.
        wsaBuf.len = BUFSIZE; // ������ ũ�⸦ �����մϴ�.

        DWORD recvLen = 0;
        DWORD flags = 0;
        // �񵿱� ���� ���� �Լ��Դϴ�. �����Ͱ� �����ϸ� IOCP�� �Ϸ� ��Ŷ�� �ֽ��ϴ�.
        ::WSARecv(session->socket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL);
    }
}

int main()
{
    // Windows Sockets �ʱ�ȭ
    WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

    // TCP ������ �����մϴ�.
    SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET)
        return 0;

    // ������ �ּҿ� ��Ʈ�� �����մϴ�.
    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    serverAddr.sin_port = ::htons(7777);

    // ���Ͽ� ������ �ּҿ� ��Ʈ�� ���ε��մϴ�.
    if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        return 0;

    // Ŭ���̾�Ʈ�� ���� ��û�� ����ϵ��� �����մϴ�.
    if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
        return 0;

    cout << "Accept" << endl;

    vector<Session*> sessionManager;

    // IOCP ��ü�� �����մϴ�.
    HANDLE iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    // �۾� �����带 �����մϴ�. 
    for (int32 i = 0; i < 5; i++)
        GThreadManager->Launch([=]() { WorkerThreadMain(iocpHandle); });

    // ���� ������� Ŭ���̾�Ʈ�� ���� ��û�� �����մϴ�.
    while (true)
    {
        SOCKADDR_IN clientAddr;
        int32 addrLen = sizeof(clientAddr);

        // Ŭ���̾�Ʈ�� ���� ��û�� �����մϴ�.
        SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET)
            return 0;

        // �� ������ �����ϰ� ���� �Ŵ����� �߰��մϴ�.
        Session* session = xnew<Session>();
        session->socket = clientSocket;
        sessionManager.push_back(session);

        cout << "Client Connected !" << endl;

        // Ŭ���̾�Ʈ ������ IOCP�� ����մϴ�.
        ::CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, /*Key*/(ULONG_PTR)session, 0);

        WSABUF wsaBuf;
        wsaBuf.buf = session->recvBuffer; // ���۸� �����մϴ�.
        wsaBuf.len = BUFSIZE; // ������ ũ�⸦ �����մϴ�.

        OverlappedEx* overlappedEx = new OverlappedEx();
        overlappedEx->type = IO_TYPE::READ; // IO Ÿ���� READ��
        // ADD_REF
        DWORD recvLen = 0;
        DWORD flags = 0;

        // Ŭ���̾�Ʈ�κ��� �񵿱������� �����͸� �ޱ� �����մϴ�.
        ::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL);

        // ������ ���� ���� ����!
        // Session* s = sessionManager.back();
        // sessionManager.pop_back();
        // xdelete(s);

        // ���⼭ Ŭ���̾�Ʈ ���ϰ� �̺�Ʈ�� ���� �� �ֽ��ϴ�.
        //::closesocket(session.socket);
        //::WSACloseEvent(wsaEvent);
    }

    // ��� �����尡 ����� ������ ��ٸ��ϴ�.
    GThreadManager->Join();

    // Windows Sockets�� �����մϴ�.
    ::WSACleanup();
}

