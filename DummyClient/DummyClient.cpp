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
    // 윈속 초기화. 실패하면 0을 반환해 프로그램 종료
    WSAData wsaData;
    if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

    // UDP 소켓 생성. 실패하면 에러 메시지 출력 후 프로그램 종료
    SOCKET clientSocket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        HandleError("Socket");
        return 0;
    }

    // 서버 주소 설정. 여기서는 로컬 호스트(자기 자신)를 가리킴
    SOCKADDR_IN serverAddr;
    ::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    ::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = ::htons(7777);

    // 서버와의 연결을 설정 (Connected UDP)
    ::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

    // ---------------------------

    while (true)
    {
        // 클라이언트가 서버로 보낼 메시지
        char sendBuffer[100] = "Hello World!";

        // 서버로 메시지 전송 (Connected UDP 사용)
        int32 resultCode = ::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0);

        // 메시지 전송이 실패한 경우 에러 처리
        if (resultCode == SOCKET_ERROR)
        {
            HandleError("SendTo");
            return 0;
        }

        // 전송한 메시지의 길이 출력
        cout << "Send Data! Len = " << sizeof(sendBuffer) << endl;

        // 서버로부터의 응답을 받을 주소 구조체
        SOCKADDR_IN recvAddr;
        ::memset(&recvAddr, 0, sizeof(recvAddr));
        int32 addrLen = sizeof(recvAddr);
        // 서버로부터의 응답을 받을 버퍼
        char recvBuffer[1000];

        // 서버로부터 데이터를 받음. 받은 데이터는 recvBuffer에 저장되고, 데이터의 길이는 recvLen에 저장됨.
        int32 recvLen = ::recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);

        // 데이터 수신이 실패하거나 서버로부터 연결이 끊어진 경우 에러 처리
        if (recvLen <= 0)
        {
            HandleError("RecvFrom");
            return 0;
        }

        // 수신한 데이터와 데이터의 길이 출력
        cout << "Recv Data! Data = " << recvBuffer << endl;
        cout << "Recv Data! Len = " << recvLen << endl;

        // 1초 대기
        this_thread::sleep_for(1s);
    }

    // ---------------------------

    // 소켓 리소스 반환
    ::closesocket(clientSocket);

    // 윈속 종료
    ::WSACleanup();
}