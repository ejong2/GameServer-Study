#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IocpEvent.h"
#include "Session.h"

/*----------------
     Listener
-----------------*/

Listener::~Listener()
{
    SocketUtils::Close(_socket); // 소켓을 닫음

    for (AcceptEvent* acceptEvent : _acceptEvents)
    {
        // TODO : 나중에 추가할 로직
        xdelete(acceptEvent); // AcceptEvent 객체들 삭제
    }
}

bool Listener::StartAccept(NetAddress netAddress)
{
    _socket = SocketUtils::CreateSocket(); // 소켓 생성
    if (_socket == INVALID_SOCKET)
        return false;

    if (GIocpCore.Register(this) == false) // IOCP에 현재 Listener 등록
        return false;

    // 추가적인 소켓 설정
    if (SocketUtils::SetReuseAddress(_socket, true) == false)
        return false;
    if (SocketUtils::SetLinger(_socket, 0, 0) == false)
        return false;
    if (SocketUtils::Bind(_socket, netAddress) == false)
        return false;
    if (SocketUtils::Listen(_socket) == false)
        return false;

    // AcceptEvent 생성 후 등록
    const int32 acceptCount = 1;
    for (int32 i = 0; i < acceptCount; i++)
    {
        AcceptEvent* acceptEvents = xnew<AcceptEvent>();
        _acceptEvents.push_back(acceptEvents);
        RegisterAccept(acceptEvents);
    }

    return true;
}

void Listener::CloseSocket()
{
    SocketUtils::Close(_socket); // 소켓을 닫음
}

HANDLE Listener::GetHandle()
{
    // 현재 Listener의 소켓 핸들을 반환
    return reinterpret_cast<HANDLE>(_socket);
}


void Listener::Dispatch(IocpEvent* iocpEvent, int32 numOfBytes)
{
    ASSERT_CRASH(iocpEvent->GetType() == EventType::Accept);

    AcceptEvent* acceptEvent = static_cast<AcceptEvent*>(iocpEvent);
    ProcessAccept(acceptEvent);
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
    Session* session = xnew<Session>();

    acceptEvent->Init();
    acceptEvent->SetSession(session);

    DWORD bytesReceived = 0;
    if (false == SocketUtils::AcceptEx(_socket, session->GetSocket(), session->_recvBuffer, 0,
        sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
        OUT & bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent)))
    {
        const int32 errorCode = WSAGetLastError();
        if (errorCode != WSA_IO_PENDING)
        {
            // 일단 다시 Accept 걸어준다
            RegisterAccept(acceptEvent);
        }
    }
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
    Session* session = acceptEvent->GetSession();

    if (false == SocketUtils::SetUpdateAcceptSocket(session->GetSocket(), _socket))
    {
        RegisterAccept(acceptEvent);
        return;
    }

    SOCKADDR_IN sockAddress;
    int32 sizeOfSockAddress = sizeof(sockAddress);
    if (SOCKET_ERROR == ::getpeername(session->GetSocket(), OUT reinterpret_cast<SOCKADDR*>(&sockAddress), &sizeOfSockAddress))
    {
        RegisterAccept(acceptEvent);
        return;
    }

    session->SetNetAddress(NetAddress(sockAddress));

    cout << "Client Connected!" << endl;

    // TODO

    RegisterAccept(acceptEvent);
}
