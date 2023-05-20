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

    // UDP 소켓 생성. 실패하면 에러 메시지 출력 후 프로그램 종료
    SOCKET serverSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        HandleError("Socket");
        return 0;
    }

    // 서버 주소 설정. 여기서는 모든 IP로부터의 접속을 허용
    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    serverAddr.sin_port = ::htons(7777);

    // 소켓에 주소 바인딩. 실패하면 에러 메시지 출력 후 프로그램 종료
    if (::bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        HandleError("Bind");
        return 0;
    }

    while (true)
    {
        // 클라이언트의 주소를 저장할 구조체
        SOCKADDR_IN clientAddr;
        ::memset(&clientAddr, 0, sizeof(clientAddr));
        int32 addrLen = sizeof(clientAddr);

        // recvBuffer는 클라이언트로부터 받은 데이터를 저장할 공간
        char recvBuffer[1000];

        // 클라이언트로부터 데이터를 받음. 받은 데이터는 recvBuffer에 저장되고, 데이터의 길이는 recvLen에 저장됨.
        int32 recvLen = ::recvfrom(serverSocket, recvBuffer, sizeof(recvBuffer), 0,
            (SOCKADDR*)&clientAddr, &addrLen);

        // 데이터 수신이 실패하거나 클라이언트로부터 연결이 끊어진 경우 에러 처리
        if (recvLen <= 0)
        {
            HandleError("RecvFrom");
            return 0;
        }

        // 수신한 데이터와 데이터의 길이 출력
        cout << "Recv Data! Data = " << recvBuffer << endl;
        cout << "Recv Data! Len = " << recvLen << endl;
        // 클라이언트에게 받은 데이터를 그대로 다시 보냄 (echo)
        int32 errorCode = ::sendto(serverSocket, recvBuffer, recvLen, 0,
            (SOCKADDR*)&clientAddr, sizeof(clientAddr));

        // 데이터 전송이 실패한 경우 에러 처리
        if (errorCode == SOCKET_ERROR)
        {
            HandleError("SendTo");
            return 0;
        }

        // 전송한 데이터의 길이를 출력
        cout << "Send Data! Len = " << recvLen << endl;
    }

    // 소켓 통신을 마치면 윈속을 종료
    ::WSACleanup();
}