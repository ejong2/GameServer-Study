#include "pch.h"
#include <iostream>
#include <winsock2.h>  // Windows 전용 네트워크 프로그래밍 헤더
#include <mswsock.h>  // Windows 소켓 API에 대한 Microsoft 전용 확장
#include <ws2tcpip.h> // Winsock 함수, 구조체, 정의의 대부분을 포함
#pragma comment(lib, "ws2_32.lib") // ws2_32 라이브러리 연결

int main()
{
	WSAData wsaData; // 윈도우 소켓 정보를 담은 구조체
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) // 윈도우 소켓 초기화
		return 0;

	SOCKET clientSocket = ::socket(AF_INET, SOCK_STREAM, 0); // 클라이언트 소켓 생성 (IPv4, TCP)
	if (clientSocket == INVALID_SOCKET) // 소켓 생성 실패 시
	{
		int32 errCode = ::WSAGetLastError(); // 에러 코드 추출
		cout << "Socket ErrorCode : " << errCode << endl;
		return 0;
	}

	SOCKADDR_IN serverAddr; // 서버 주소를 담는 IPv4 구조체
	::memset(&serverAddr, 0, sizeof(serverAddr)); // 주소 구조체 초기화
	serverAddr.sin_family = AF_INET; // 주소 체계 지정 (IPv4)
	::inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); // 연결할 서버의 IP 주소 지정
	serverAddr.sin_port = ::htons(7777); // 연결할 서버의 포트 번호 지정 (7777)

	if (::connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) // 서버에 연결 요청
	{
		int32 errCode = ::WSAGetLastError(); // 에러 코드 추출
		cout << "Connect ErrorCode : " << errCode << endl;
		return 0;
	}

	cout << "Connected To Server!" << endl; // 서버 연결 성공 메시지 출력
	while (true) // 서버와 데이터 통신을 계속하기 위한 루프
	{
		char sendBuffer[100] = "Hello World!"; // 서버로 보낼 메시지

		for (int32 i = 0; i < 10; i++) // 동일한 메시지를 10번 전송
		{
			int32 resultCode = ::send(clientSocket, sendBuffer, sizeof(sendBuffer), 0); // 서버로 메시지 전송
			// 주의: 여기서는 클라이언트가 한 번에 100바이트씩 데이터를 전송하지만, 서버 측에서는 1000바이트 크기의 recv 버퍼로 한 번에 데이터를 받을 수 있습니다.
			// 이는 TCP/IP 프로토콜의 특성상 데이터가 버퍼에 쌓이고, 충분한 양의 데이터가 도착하면 한 번에 읽을 수 있기 때문입니다.

			if (resultCode == SOCKET_ERROR) // 데이터 전송 오류 시
			{
				int32 errCode = ::WSAGetLastError(); // 에러 코드 추출
				cout << "Send ErrorCode : " << errCode << endl;
				return 0;
			}
		}

		cout << "Send Data! Len = " << sizeof(sendBuffer) << endl; // 전송한 데이터의 길이 출력

		/* 주석 처리된 코드는 서버로부터 데이터를 받는 코드로, 필요 시 주석 해제 후 사용 가능
		if (recvLen <= 0) // 수신된 데이터가 없거나 오류가 발생한 경우
		{
			int32 errCode = ::WSAGetLastError(); // 에러 코드 추출
			cout << "Recv ErrorCode : " << errCode << endl;
			return 0;
		}

		cout << "Recv Data! Data = " << recvBuffer << endl; // 수신된 데이터 출력
		cout << "Recv Data! Len = " << recvLen << endl; // 수신된 데이터의 길이 출력
		*/

		this_thread::sleep_for(1s); // 현재 스레드를 1초 동안 중지 (추후 데이터 통신 코드 작성 후, 필요에 따라 수정)
	}

	::closesocket(clientSocket); // 클라이언트 소켓을 닫아 리소스 반환
	::WSACleanup(); // 윈속 종료
}