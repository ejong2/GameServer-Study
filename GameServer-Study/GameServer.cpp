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

    // WSAEVENT 형식의 벡터를 선언합니다. 각 이벤트는 소켓의 네트워크 이벤트를 감지합니다.
    vector<WSAEVENT> wsaEvents; 

    // Session 형식의 벡터를 선언합니다. 각 세션은 클라이언트의 연결과 데이터 전송을 처리합니다.
    vector<Session> sessions; 
    sessions.reserve(100);  // 최대 100개의 세션을 저장할 수 있는 공간을 예약합니다.

    WSAEVENT listenEvent = ::WSACreateEvent();  // 새로운 이벤트 객체를 생성합니다. 이 이벤트는 리스닝 소켓에서 네트워크 이벤트를 감지합니다.
    wsaEvents.push_back(listenEvent);  // 생성한 이벤트 객체를 wsaEvents 벡터에 추가합니다.
    sessions.push_back(Session{ listenSocket });  // 새로운 세션을 생성하여 sessions 벡터에 추가합니다. 이 세션은 listenSocket을 사용하여 데이터를 처리합니다.
    if (::WSAEventSelect(listenSocket, listenEvent, FD_ACCEPT | FD_CLOSE) == SOCKET_ERROR)  // 이벤트를 특정 소켓에 연결하고, 어떤 네트워크 이벤트를 감지할 것인지 설정합니다. 여기서는 접속 수락(FD_ACCEPT)과 접속 종료(FD_CLOSE) 이벤트를 감지합니다.
        return 0;

    while (true)
    { 
        // 여러 이벤트 중 하나가 발생할 때까지 기다립니다. 발생한 이벤트의 인덱스를 반환합니다.
        int32 index = ::WSAWaitForMultipleEvents(wsaEvents.size(), &wsaEvents[0], FALSE, WSA_INFINITE, FALSE);
        if(index == WSA_WAIT_FAILED)
            continue;

        index -= WSA_WAIT_EVENT_0;

        // 발생한 이벤트에 대한 네트워크 이벤트와 오류 코드를 가져옵니다.
        WSANETWORKEVENTS networkEvents;
        if(::WSAEnumNetworkEvents(sessions[index].socket, wsaEvents[index], &networkEvents) == SOCKET_ERROR)
            continue;

        // Listener 소켓 체크
        if (networkEvents.lNetworkEvents & FD_ACCEPT)
        {
            // Error-Check
            if(networkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
                continue;

            SOCKADDR_IN clientAddr;
            int32 addrLen = sizeof(clientAddr);

            SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
            if (clientSocket != INVALID_SOCKET)
            {
                cout << "Client Connected" << endl;

                WSAEVENT clientEvent = WSACreateEvent();
                wsaEvents.push_back(clientEvent);
                sessions.push_back(Session{ clientSocket });
                if(::WSAEventSelect(clientSocket, clientEvent, FD_READ | FD_WRITE | FD_CLOSE) == SOCKET_ERROR)
                    return 0;
            }
        }
        // Client Session 소켓 체크
        if (networkEvents.lNetworkEvents & FD_READ || networkEvents.lNetworkEvents & FD_WRITE)
        {
            // Error - Check
            if((networkEvents.lNetworkEvents & FD_READ) && (networkEvents.iErrorCode[FD_READ_BIT] != 0))
                continue;
            // Error - Check
            if ((networkEvents.lNetworkEvents & FD_WRITE) && (networkEvents.iErrorCode[FD_WRITE_BIT] != 0))
                continue;

            Session& s = sessions[index];

            // Read
            if (s.recvBytes == 0)
            {
                int32 recvLen = ::recv(s.socket, s.recvBuffer, BUFSIZE, 0);
                if (recvLen == SOCKET_ERROR && ::WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    // TODO : Remove Session
                    continue;
                }
                s.recvBytes = recvLen;
                cout << "Recv Data = " << recvLen << endl;
            }
            
            // Write
            if (s.recvBytes > s.sendBytes)
            {
                int32 sendLen = ::send(s.socket, &s.recvBuffer[s.sendBytes], s.recvBytes - s.sendBytes, 0);
                if (sendLen == SOCKET_ERROR && ::GetLastError() != WSAEWOULDBLOCK)
                {
                    // TODO : Remove Session
                    continue;
                }
                s.sendBytes += sendLen;
                if (s.recvBytes == s.sendBytes)
                {
                    s.recvBytes = 0;
                    s.sendBytes = 0;
                }
                cout << "Send Data = " << sendLen << endl;
            }
        }
        // FD_CLOSE 처리
        if (networkEvents.lNetworkEvents & FD_CLOSE)
        {
            // TODO : Remove Socket
        }
    }

    // 모든 통신을 마친 후에는 윈속을 종료합니다.
    ::WSACleanup();
}