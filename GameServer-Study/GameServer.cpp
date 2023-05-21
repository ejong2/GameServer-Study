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
    // 마지막으로 발생한 에러 코드를 가져옵니다.
    int32 errCode = ::WSAGetLastError();
    // 에러 원인과 에러 코드를 출력합니다.
    cout << cause << " ErrorCode : " << errCode << endl;
}

const int32 BUFSIZE = 1000;

struct Session
{
    // 클라이언트와 통신을 위한 소켓을 저장합니다.
    SOCKET socket = INVALID_SOCKET;
    // 데이터를 받을 버퍼입니다.
    char recvBuffer[BUFSIZE] = {};
    // 수신된 데이터의 크기를 저장합니다.
    int32 recvBytes = 0;
    // 보낸 데이터의 크기를 저장합니다.
    int32 sendBytes = 0;
};

int main()
{
    // Winsock을 초기화합니다. 실패 시 0을 반환하여 프로그램을 종료합니다.
    WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

    // 논블로킹 소켓을 생성합니다.
    SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET)
        return 0;

    // 서버 소켓을 논블로킹 모드로 설정합니다.
    u_long on = 1;
    if (::ioctlsocket(listenSocket, FIONBIO, &on) == INVALID_SOCKET)
        return 0;

    // 서버 소켓을 바인딩합니다. 실패 시 0을 반환하여 프로그램을 종료합니다.
    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    serverAddr.sin_port = ::htons(7777);

    if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        return 0;

    // 서버 소켓을 리스닝 상태로 만듭니다. 실패 시 0을 반환하여 프로그램을 종료합니다.
    if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
        return 0;

    cout << "Accept" << endl;

    // 세션들을 저장할 벡터를 선언합니다.
    vector<Session> sessions;
    sessions.reserve(100);

    // select 함수를 사용하기 위한 fd_set 변수들을 선언합니다.
    fd_set reads;
    fd_set writes;

    while (true)
    {
        // fd_set 변수들을 초기화합니다.
        FD_ZERO(&reads);
        FD_ZERO(&writes);

        // ListenSocket을 읽기 set에 등록합니다.
        FD_SET(listenSocket, &reads);

        // 세션의 소켓들을 읽기 혹은 쓰기 set에 등록합니다.
        for (Session& s : sessions)
        {
            if (s.recvBytes <= s.sendBytes)
                FD_SET(s.socket, &reads);
            else
                FD_SET(s.socket, &writes);
        }

        // select 함수를 호출하여 등록된 소켓들 중 read 혹은 write 가능한 소켓이 있는지 확인합니다.
        int32 retVal = ::select(0, &reads, &writes, nullptr, nullptr);
        if (retVal == SOCKET_ERROR)
            break;

        // ListenSocket을 체크하여 새로운 클라이언트가 접속했는지 확인합니다.
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
        // 세션 소켓들을 체크하여 데이터가 읽기 혹은 쓰기 가능한 상태인지 확인합니다.
        for (Session& s : sessions)
        {
            // Read
            if (FD_ISSET(s.socket, &reads))
            {
                int32 recvLen = ::recv(s.socket, s.recvBuffer, BUFSIZE, 0);
                if (recvLen <= 0)
                {
                    // 데이터를 받지 못했거나 연결이 끊어진 경우 세션을 제거해야 합니다.
                    continue;
                }
                s.recvBytes = recvLen;
            }

            // Write
            if (FD_ISSET(s.socket, &writes))
            {
                // 블로킹 모드 -> 모든 데이터 다 보냄
                // 논블로킹 모드 -> 일부만 보낼 수가 있음(상대방 수신 버퍼 상황에 따라)
                int32 sendLen = ::send(s.socket, &s.recvBuffer[s.sendBytes], s.recvBytes - s.sendBytes, 0);
                if (sendLen == SOCKET_ERROR)
                {
                    // 데이터를 보내지 못했거나 연결이 끊어진 경우 세션을 제거해야 합니다.
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

    // 모든 통신을 마친 후에는 윈속을 종료합니다.
    ::WSACleanup();
}