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
    // Overlapped I/O 작업을 위한 WSAOVERLAPPED 구조체 선언. 이 구조체는 비동기 I/O 작업 상태를 유지합니다.
    WSAOVERLAPPED overlapped = {};

    SOCKET socket = INVALID_SOCKET;
    char recvBuffer[BUFSIZE] = {};
    int32 recvBytes = 0;
};

void CALLBACK RecvCallBack(DWORD error, DWORD recvLen, LPWSAOVERLAPPED overlapped, DWORD flags)
{
    cout << "Data Recv Len CallBack = " << recvLen << endl;
    // TODO : 에코 서버를 만든다면 WSASend()

    Session* session = (Session*)overlapped;
}

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

    // Overlapped 모델(Completion Routine 콜백 기반)
    // - 비동기 입출력 지원하는 소켓 생성
    // - 비동기 입출력 함수 호출 (완료 루틴의 시작 주소를 넘겨준다)
    // - 비동기 작업이 바로 완료되지 않으면, WSA_IO_PENDING 오류 코드
    // - 비동기 입출력 함수 호출한 쓰레드를 -> Alertable Wait 상태로 만든다
    // ex) WaitForSingleObject, WaitForMultipleObjectEx, SleepEx, WSAWaitForMultipleEvents
    // - 비동기 IO 완료되면, 운영체제는 완료 루틴 호출
    // - 완료 루틴 호출이 모두 끝나면, 쓰레드는 Alertable Wait 상태에서 빠져나온다
    
    // 1) 오류 발생시 0이 아닌 값
    // 2) 전송 바이트 수
    // 3) 비동기 입출력 함수 호출 시 넘겨준 WSAOVERLAPPED 구조체의 주소값
    // 4) 0
    // void CompletionRoutine()


    while (true)
    {
        SOCKADDR_IN clientAddr;
        int32 addrLen = sizeof(clientAddr);

        SOCKET clientSocket;
        while (true)
        {
            clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
            if (clientSocket != INVALID_SOCKET)
                break;

            if (::WSAGetLastError() == WSAEWOULDBLOCK)
                continue;

            // 문제 있는 상황
            return 0;
        }

        Session session = Session{ clientSocket };
        WSAEVENT wsaEvent = ::WSACreateEvent();

        cout << "Client Connected !" << endl;

        while (true)
        {
            WSABUF wsaBuf;
            wsaBuf.buf = session.recvBuffer;
            wsaBuf.len = BUFSIZE;

            DWORD recvLen = 0;
            DWORD flags = 0;

            if (::WSARecv(clientSocket, &wsaBuf, 1, &recvLen, &flags, &session.overlapped, RecvCallBack) == SOCKET_ERROR)
            {
                if (::WSAGetLastError() == WSA_IO_PENDING)
                {
                    // 비동기 작업이 완료될 때까지 기다립니다. 이는 I/O 작업이 pending 상태일 때, 해당 이벤트가 시그널 상태가 될 때까지 대기합니다.
                    // Alertable Wait                    
                    ::SleepEx(INFINITE, TRUE);
                    //::WSAWaitForMultipleEvents(1, &wsaEvent, TRUE, WSA_INFINITE, TRUE);
                                        
                }
                else
                {
                    // TODO : 문제 있는 상황
                    break;
                }
            }
            else
            {
                cout << "Data Recv Len = " << recvLen << endl;
            }
        }
        ::closesocket(session.socket);
        ::WSACloseEvent(wsaEvent);
    }

    // 모든 통신을 마친 후에는 윈속을 종료합니다.
    ::WSACleanup();
}