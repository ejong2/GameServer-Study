#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"
#include <winsock2.h>  // Windows ���� ��Ʈ��ũ ���α׷��� ���
#include <mswsock.h>  // Windows ���� API�� ���� Microsoft ���� Ȯ��
#include <ws2tcpip.h> // Winsock �Լ�, ����ü, ������ ��κ��� ����
#pragma comment(lib, "ws2_32.lib") // ws2_32 ���̺귯�� ����

int main()
{
	WSAData wsaData; // ������ ���� ������ ���� ����ü
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) // ������ ���� �ʱ�ȭ
		return 0;

	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0); // ���� ���� ���� (IPv4, TCP)
	if (listenSocket == INVALID_SOCKET) // ���� ���� ���� ��
	{
		int32 errCode = ::WSAGetLastError(); // ���� �ڵ� ����
		cout << "Socket ErrorCode : " << errCode << endl;
		return 0;
	}

	SOCKADDR_IN serverAddr; // ���� �ּҸ� ��� IPv4 ����ü
	::memset(&serverAddr, 0, sizeof(serverAddr)); // �ּ� ����ü �ʱ�ȭ
	serverAddr.sin_family = AF_INET; // �ּ� ü�� ���� (IPv4)
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY); // IP �ּҴ� �ý��ۿ� ���� �ڵ� ����
	serverAddr.sin_port = ::htons(7777); // ��Ʈ ��ȣ ���� (7777)

	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) // ���� ���Ͽ� �ּ� ���ε�
	{
		int32 errCode = ::WSAGetLastError(); // ���� �ڵ� ����
		cout << "Bind ErrorCode : " << errCode << endl;
		return 0;
	}

	if (::listen(listenSocket, 10) == SOCKET_ERROR) // ���� ������ ������ ���·� ���� (��α� ť ũ�� 10)
	{
		int32 errCode = ::WSAGetLastError(); // ���� �ڵ� ����
		cout << "Listen ErrorCode : " << errCode << endl;
		return 0;
	}

	while (true) // Ŭ���̾�Ʈ�� ���� ��û�� ��� ��ٸ�
	{
		SOCKADDR_IN clientAddr; // Ŭ���̾�Ʈ �ּҸ� ��� IPv4 ����ü
		::memset(&clientAddr, 0, sizeof(clientAddr)); // �ּ� ����ü �ʱ�ȭ
		int32 addrLen = sizeof(clientAddr);
		SOCKET clientSocket = ::accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen); // Ŭ���̾�Ʈ�� ���� ��û ����
		if (clientSocket == INVALID_SOCKET) // ���� ���� ���� ��
		{
			int32 errCode = ::WSAGetLastError(); // ���� �ڵ� ����
			cout << "Accept ErrorCode : " << errCode << endl;
			return 0;
		}

		char ipAddress[16]; // IP �ּҸ� ���� �迭
		::inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, sizeof(ipAddress)); // Ŭ���̾�Ʈ�� IP �ּҸ� ���ڿ��� ��ȯ
		cout << "Client Connected! IP = " << ipAddress << endl; // ����� Ŭ���̾�Ʈ�� IP �ּ� ���

		// TODO: Ŭ���̾�Ʈ�� ������ ��� �ڵ带 ���⿡ �ۼ�
	}

	::WSACleanup(); // ���� ����
}