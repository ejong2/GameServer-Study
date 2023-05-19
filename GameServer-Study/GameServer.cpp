#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"
#include <winsock2.h>  // Windows 전용 네트워크 프로그래밍 헤더
#include <mswsock.h>  // Windows 소켓 API에 대한 Microsoft 전용 확장
#include <ws2tcpip.h> // Winsock 함수, 구조체, 정의의 대부분을 포함
#pragma comment(lib, "ws2_32.lib") // ws2_32 라이브러리 연결

int main()
{
	WSAData wsaData; // 윈도우 소켓 정보를 담은 구조체
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) // 윈도우 소켓 초기화
		return 0;

	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0); // 서버 소켓 생성 (IPv4, TCP)
	if (listenSocket == INVALID_SOCKET) // 소켓 생성 실패 시
	{
		int32 errCode = ::WSAGetLastError(); // 에러 코드 추출
		cout << "Socket ErrorCode : " << errCode << endl;
		return 0;
	}

	SOCKADDR_IN serverAddr; // 서버 주소를 담는 IPv4 구조체
	::memset(&serverAddr, 0, sizeof(serverAddr)); // 주소 구조체 초기화
	serverAddr.sin_family = AF_INET; // 주소 체계 지정 (IPv4)
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY); // IP 주소는 시스템에 의해 자동 설정
	serverAddr.sin_port = ::htons(7777); // 포트 번호 지정 (7777)

	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) // 서버 소켓에 주소 바인드
	{
		int32 errCode = ::WSAGetLastError(); // 에러 코드 추출
		cout << "Bind ErrorCode : " << errCode << endl;
		return 0;
	}

	if (::listen(listenSocket, 10) == SOCKET_ERROR) // 서버 소켓을 리스닝 상태로 변경 (백로그 큐 크기 10)
	{
		int32 errCode = ::WSAGetLastError(); // 에러 코드 추출
		cout << "Listen ErrorCode : " << errCode << endl;
		return 0;
	}

	while (true) // 클라이언트의 연결 요청을 계속 기다림
	{
		SOCKADDR_IN clientAddr; // 클라이언트 주소를 담는 IPv4 구조체
		::memset(&clientAddr, 0, sizeof(clientAddr)); // 주소 구조체 초기화
		int32 addrLen = sizeof(clientAddr);
		SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen); // 클라이언트의 연결 요청 수락
		if (clientSocket == INVALID_SOCKET) // 연결 수락 실패 시
		{
			int32 errCode = ::WSAGetLastError(); // 에러 코드 추출
			cout << "Accept ErrorCode : " << errCode << endl;
			return 0;
		}

		char ipAddress[16]; // IP 주소를 담을 배열
		::inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, sizeof(ipAddress)); // 클라이언트의 IP 주소를 문자열로 변환
		cout << "Client Connected! IP = " << ipAddress << endl; // 연결된 클라이언트의 IP 주소 출력

		// TODO: 클라이언트와 데이터 통신 코드를 여기에 작성
	}

	::WSACleanup(); // 윈속 종료
}