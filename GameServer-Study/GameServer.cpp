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

    // SO_KEEPALIVE: 이 옵션을 설정하면 TCP 연결의 상태가 주기적으로 확인됩니다.
    // 예를 들어, 만약 상대방이 연결을 소리소문 없이 끊었다면 이 옵션을 통해 감지할 수 있습니다.
    bool enable = true;
    ::setsockopt(serverSocket, SOL_SOCKET, SO_KEEPALIVE, (char*)&enable, sizeof(enable));

    // SO_LINGER: 이 옵션은 소켓을 닫을 때 아직 전송되지 않은 데이터가 있는 경우 어떻게 처리할지를 결정합니다.
    // linger.l_onoff 가 0이면 closesocket()이 바로 리턴하고, 아니면 linger.l_linger 초 만큼 대기합니다. 
    LINGER linger;
    linger.l_onoff = 1;
    linger.l_linger = 5;
    ::setsockopt(serverSocket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));

    // SO_SNDBUF 및 SO_RCVBUF: 이들 옵션은 송신 및 수신 버퍼의 크기를 설정합니다.
    int32 sendBufferSize;
    int32 optionLen = sizeof(sendBufferSize);
    ::getsockopt(serverSocket, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufferSize, &optionLen);
    cout << "송신 버퍼 크기 : " << sendBufferSize << endl;

    int32 recvBufferSize;
    optionLen = sizeof(sendBufferSize);
    ::getsockopt(serverSocket, SOL_SOCKET, SO_RCVBUF, (char*)&recvBufferSize, &optionLen);
    cout << "수신 버퍼 크기 : " << recvBufferSize << endl;

    // SO_REUSEADDR: 이 옵션을 사용하면 소켓이 동일한 주소와 포트 번호를 재사용할 수 있습니다.
    {
        bool enable = true;
        ::setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(enable));
    }

    // TCP_NODELAY: 이 옵션은 Nagle 알고리즘을 활성화 또는 비활성화 합니다.
   // Nagle 알고리즘은 데이터가 충분히 크면 보내고, 그렇지 않으면 데이터가 충분히 쌓일 때까지 대기합니다.
   // 이 옵션을 true 설정하면 게임에서 빠른 반응 시간을 필요로 하는 경우(예를 들어 실시간 FPS 게임) 유용할 수 있습니다.
   // 그러나, 이 옵션을 활성화하면 네트워크 상에서 작은 패킷이 불필요하게 많이 발생하는 것을 방지할 수 없습니다.
    {
        bool enable = true;
        ::setsockopt(serverSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(enable));
    }

    // 소켓 통신을 마치면 윈속을 종료합니다.
    ::WSACleanup();
}