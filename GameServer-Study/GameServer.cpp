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

// 에러가 발생한 경우 에러 메시지와 에러 코드를 출력
void HandleError(const char* cause)
{
    int32 errCode = ::WSAGetLastError();
    cout << cause << " ErrorCode : " << errCode << endl;
}

int main()
{
    // 윈속 초기화. 실패하면 0을 반환해 프로그램 종료
    WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

    // 논블로킹(Non-Blocking)
    
    SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET)
        return 0;

    // 서버 소켓을 논-블로킹 모드로 설정
    // 논-블로킹 모드에서는 소켓 함수 호출이 즉시 완료되지 않으면 WSAEWOULDBLOCK 에러를 반환합니다.
    // 이를 통해 프로그램은 네트워크 연산을 기다리지 않고 다른 작업을 계속 수행할 수 있습니다.
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

    // 클라이언트의 접속을 계속해서 수락합니다.
    while (true)
    {
        SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET)
        {
            // accept 함수는 블로킹 함수지만, 우리는 논블로킹으로 처리하기 위해 WSAEWOULDBLOCK 에러가 발생했을 경우
            // 바로 다음 클라이언트의 접속을 수락하도록 continue합니다.
            if (::WSAGetLastError() == WSAEWOULDBLOCK)
                continue;

            // 그 외 에러 발생 시 종료
            break;
        }

        cout << "Client Connected!" << endl;

        // 클라이언트로부터 데이터를 계속 수신합니다.
        while (true)
        {
            char recvBuffer[1000];
            int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);
            if (recvLen <= SOCKET_ERROR)
            {
                // recv 함수는 블로킹 함수지만, 우리는 논블로킹으로 처리하기 위해 WSAEWOULDBLOCK 에러가 발생했을 경우
                // 바로 다음 데이터를 수신하도록 continue합니다.
                if (::WSAGetLastError() == WSAEWOULDBLOCK)
                    continue;

                // 그 외 에러 발생 시 종료
                break;
            }
            else if (recvLen == 0)
            {
                // 클라이언트가 연결을 종료하면 종료합니다.
                break;
            }

            cout << "Recv Data Len = " << recvLen << endl;

            // 받은 데이터를 클라이언트에 다시 보냅니다. (에코)
            while (true)
            {
                if (::send(clientSocket, recvBuffer, recvLen, 0) == SOCKET_ERROR)
                {
                    // send 함수는 블로킹 함수지만, 우리는 논블로킹으로 처리하기 위해 WSAEWOULDBLOCK 에러가 발생했을 경우
                    // 바로 다음 데이터를 전송하도록 continue합니다.
                    if (::WSAGetLastError() == WSAEWOULDBLOCK)
                        continue;

                    // 그 외 에러 발생 시 종료
                    break;
                }
                cout << "Send Data ! Len = " << recvLen << endl;
                break;
            }
        }
    }

    // 모든 통신을 마친 후에는 윈속을 종료합니다.
    ::WSACleanup();
}