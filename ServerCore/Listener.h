#pragma once
#include "IocpCore.h"
#include "NetAddress.h"

class AcceptEvent;

/*----------------
     Listener
-----------------*/

class Listener : public IocpObject
{
public:
    Listener() = default;
    ~Listener();

public:
    bool StartAccept(NetAddress netAddress); // 소켓을 열고 연결을 받기 시작하는 메서드
    void CloseSocket(); // 열려있는 소켓을 닫는 메서드

public:
    /* 인터페이스 구현 */
    virtual HANDLE GetHandle() override; // Listener의 소켓 핸들을 가져오는 메서드
    virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override; // IO 완료 이벤트 처리 메서드

private:
    /* 수신 관련 */
    void RegisterAccept(AcceptEvent* acceptEvent); // Accept 작업 등록 메서드
    void ProcessAccept(AcceptEvent* acceptEvent); // Accept 완료 처리 메서드

protected:
    SOCKET _socket = INVALID_SOCKET; // Listener의 소켓
    Vector<AcceptEvent*> _acceptEvents; // AcceptEvent 리스트
};

