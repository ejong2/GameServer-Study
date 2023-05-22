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

    // 서버에 메시지 전송
    while (true)
    {
        if (::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0) == SOCKET_ERROR)
        {
            // 송신이 블로킹 상태일 경우 논블로킹으로 변경하여 계속 송신 시도
            if (::WSAGetLastError() == WSAEWOULDBLOCK)
                continue;

            // 그 외의 에러 발생시 종료
            break;
        }
        cout << "Send Data ! Len = " << sizeof(sendBuffer) << endl;

        // 서버로부터의 응답을 수신
        while (true)
        {
            char recvBuffer[1000];
            int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
            if (recvLen <= SOCKET_ERROR)
            {
                // 수신이 블로킹 상태일 경우 논블로킹으로 변경하여 계속 수신 시도
                if (::WSAGetLastError() == WSAEWOULDBLOCK)
                    continue;

                // 그 외의 에러 발생시 종료
                break;
            }
            else if (recvLen == 0)
            {
                // 서버와의 연결이 끊어진 경우
                break;
            }
            cout << "Recv Data Len = " << recvLen << endl;
            break;
        }

        this_thread::sleep_for(1s);
    }

    ::closesocket(clientSocket);

    // 윈속 종료
    ::WSACleanup();
}