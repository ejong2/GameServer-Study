#include "pch.h"
#include <iostream>

#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

// 에러가 발생한 경우 에러 메시지와 에러 코드를 출력
void HandleError(const char* cause)
{
    int32 errCode = ::WSAGetLastError();
    cout << cause << " ErrorCode : " << errCode << endl;
}


int main()
{
    this_thread::sleep_for(1s);

    // 윈속 초기화. 실패하면 0을 반환해 프로그램 종료
    WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

    SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
        return 0;

    // 소켓을 논블로킹(non-blocking)으로 설정
    u_long on = 1;
    if (::ioctlsocket(clientSocket, FIONBIO, &on) == INVALID_SOCKET)
        return 0;

    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    ::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = ::htons(7777);

    // 서버에 연결 시도
    while (true)
    {
        if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            // 연결 시도가 블로킹 상태일 경우 논블로킹으로 변경하여 계속 연결 시도
            if (::WSAGetLastError() == WSAEWOULDBLOCK)
                continue;

            if (::WSAGetLastError() == WSAEISCONN)
                break;

            // 그 외의 에러 발생시 종료
            break;
        }
    }

    cout << "Connected To Server!" << endl;

    char sendBuffer[100] = "Hello World";
    WSAEVENT wsaEvent = ::WSACreateEvent(); // 비동기 I/O 작업 완료를 알리기 위한 이벤트를 생성합니다.
    WSAOVERLAPPED overlapped = {}; // Overlapped I/O 작업을 위한 WSAOVERLAPPED 구조체 생성.
    overlapped.hEvent = wsaEvent; // 생성한 이벤트를 Overlapped 구조체의 hEvent 멤버에 연결합니다. 이 이벤트는 비동기 I/O 작업이 완료될 때 시그널 상태로 설정됩니다.

    // 서버에 메시지 전송
    while (true)
    {
        WSABUF wsaBuf;
        wsaBuf.buf = sendBuffer;
        wsaBuf.len = 100;

        DWORD sendLen = 0;
        DWORD flags = 0;
        if (::WSASend(clientSocket, &wsaBuf, 1, &sendLen, flags, &overlapped, nullptr))
        {
            if (::WSAGetLastError() == WSA_IO_PENDING) // 비동기 I/O 작업이 즉시 완료되지 않았지만, 정상적으로 시작되었다는 것을 의미합니다.
            {
                // 이벤트가 시그널 상태가 될 때까지 대기합니다. 이 함수는 비동기 I/O 작업이 완료되기를 기다립니다.
                ::WSAWaitForMultipleEvents(1, &wsaEvent, TRUE, WSA_INFINITE, FALSE);
                // Overlapped I/O 작업이 완료된 후에 해당 작업의 결과를 확인합니다.
                ::WSAGetOverlappedResult(clientSocket, &overlapped, &sendLen, FALSE, &flags);
            }
            else
            {
                // 진짜 문제 있는 상황
                break;
            }
        }

        cout << "Send Data ! Len = " << sizeof(sendBuffer) << endl;

        this_thread::sleep_for(1s);
    }
    ::closesocket(clientSocket);

    // 윈속 종료
    ::WSACleanup();
}