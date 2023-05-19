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
		// TODO: 서버와 데이터 통신 코드를 여기에 작성

		this_thread::sleep_for(1s); // 현재 스레드를 1초 동안 중지 (추후 데이터 통신 코드 작성 후, 필요에 따라 수정)
	}

	::closesocket(clientSocket); // 클라이언트 소켓을 닫아 리소스 반환
	::WSACleanup(); // 윈속 종료
}