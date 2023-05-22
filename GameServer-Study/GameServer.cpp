#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

// Winsock 라이브러리를 사용하기 위한 헤더 파일
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include "Memory.h"

// 에러 발생 시 원인과 에러 코드를 출력하는 함수
void HandleError(const char* cause)
{
    int32 errCode = ::WSAGetLastError();
    cout << cause << " ErrorCode : " << errCode << endl;
}

const int32 BUFSIZE = 1000; // 수신 데이터를 저장할 버퍼의 크기를 정의합니다.

// 세션 데이터를 저장할 구조체입니다.
struct Session
{
    SOCKET socket = INVALID_SOCKET; // 클라이언트와의 연결 소켓입니다.
    char recvBuffer[BUFSIZE] = {}; // 수신 데이터를 저장할 버퍼입니다.
    int32 recvBytes = 0; // 수신한 바이트 수를 저장합니다.
};

// I/O 작업의 종류를 나타내는 열거형입니다.
enum IO_TYPE
{
    READ,
    WRITE,
    ACCEPT,
    CONNECT,
};

// Overlapped I/O 작업에 대한 정보를 저장할 구조체입니다.
struct OverlappedEx
{
    WSAOVERLAPPED overlapped = {}; // WSAOVERLAPPED 구조체입니다. 비동기 I/O 작업의 상태를 저장합니다.
    int32 type = 0; // I/O 작업의 종류입니다.
};

// 작업 쓰레드의 메인 함수입니다. I/O 완료 패킷을 처리합니다.
void WorkerThreadMain(HANDLE iocpHandle)
{
    while (true)
    {
        DWORD bytesTransferred = 0;
        Session* session = nullptr;
        OverlappedEx* overlappedEx = nullptr;

        // IOCP에서 완료 패킷을 가져옵니다.
        BOOL ret = ::GetQueuedCompletionStatus(iocpHandle, &bytesTransferred,
            (ULONG_PTR*)&session, (LPOVERLAPPED*)&overlappedEx, INFINITE);

        // 완료 패킷을 가져오는 데 실패하거나 전송된 바이트가 없는 경우 연결이 끊어진 것으로 간주합니다.
        if (ret == FALSE || bytesTransferred == 0)
        {
            // TODO : 연결 끊김 처리
            continue;
        }

        ASSERT_CRASH(overlappedEx->type == IO_TYPE::READ); // READ 타입인지 확인합니다.

        cout << "Recv Data IOCP = " << bytesTransferred << endl;

        WSABUF wsaBuf;
        wsaBuf.buf = session->recvBuffer; // 버퍼를 설정합니다.
        wsaBuf.len = BUFSIZE; // 버퍼의 크기를 설정합니다.

        DWORD recvLen = 0;
        DWORD flags = 0;
        // 비동기 소켓 수신 함수입니다. 데이터가 도착하면 IOCP에 완료 패킷을 넣습니다.
        ::WSARecv(session->socket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL);
    }
}

int main()
{
    // Windows Sockets 초기화
    WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

    // TCP 소켓을 생성합니다.
    SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET)
        return 0;

    // 서버의 주소와 포트를 설정합니다.
    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    serverAddr.sin_port = ::htons(7777);

    // 소켓에 서버의 주소와 포트를 바인드합니다.
    if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        return 0;

    // 클라이언트의 연결 요청을 대기하도록 설정합니다.
    if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
        return 0;

    cout << "Accept" << endl;

    vector<Session*> sessionManager;

    // IOCP 객체를 생성합니다.
    HANDLE iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    // 작업 쓰레드를 생성합니다. 
    for (int32 i = 0; i < 5; i++)
        GThreadManager->Launch([=]() { WorkerThreadMain(iocpHandle); });

    // 메인 쓰레드는 클라이언트의 연결 요청을 수락합니다.
    while (true)
    {
        SOCKADDR_IN clientAddr;
        int32 addrLen = sizeof(clientAddr);

        // 클라이언트의 연결 요청을 수락합니다.
        SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET)
            return 0;

        // 새 세션을 생성하고 세션 매니저에 추가합니다.
        Session* session = xnew<Session>();
        session->socket = clientSocket;
        sessionManager.push_back(session);

        cout << "Client Connected !" << endl;

        // 클라이언트 소켓을 IOCP에 등록합니다.
        ::CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, /*Key*/(ULONG_PTR)session, 0);

        WSABUF wsaBuf;
        wsaBuf.buf = session->recvBuffer; // 버퍼를 설정합니다.
        wsaBuf.len = BUFSIZE; // 버퍼의 크기를 설정합니다.

        OverlappedEx* overlappedEx = new OverlappedEx();
        overlappedEx->type = IO_TYPE::READ; // IO 타입을 READ로
        // ADD_REF
        DWORD recvLen = 0;
        DWORD flags = 0;

        // 클라이언트로부터 비동기적으로 데이터를 받기 시작합니다.
        ::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &overlappedEx->overlapped, NULL);

        // 유저가 게임 접속 종료!
        // Session* s = sessionManager.back();
        // sessionManager.pop_back();
        // xdelete(s);

        // 여기서 클라이언트 소켓과 이벤트를 닫을 수 있습니다.
        //::closesocket(session.socket);
        //::WSACloseEvent(wsaEvent);
    }

    // 모든 쓰레드가 종료될 때까지 기다립니다.
    GThreadManager->Join();

    // Windows Sockets를 종료합니다.
    ::WSACleanup();
}

