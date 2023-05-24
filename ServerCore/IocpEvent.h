#pragma once

class Session;

enum class EventType : uint8
{
    Connect,
    Accept,
    //PreRecv
    Recv,
    Send
};

/*-----------------
     iocpEvent
------------------*/

// 각 이벤트 타입을 관리하는 클래스
class IocpEvent : public OVERLAPPED
{
public:
    IocpEvent(EventType type);  // 이벤트 타입 설정

    void Init();  // 이벤트 초기화
    EventType GetType() { return _type; }  // 이벤트 타입 반환

protected:
    EventType _type;  // 이벤트 타입
};

// 여러 가지 이벤트 타입에 대한 클래스 정의 ---------------------------------

/*-----------------
    ConnectEvent
------------------*/

class ConnectEvent : public IocpEvent
{
public:
    ConnectEvent() : IocpEvent(EventType::Connect){}
};

/*-----------------
    AcceptEvent
------------------*/

class AcceptEvent : public IocpEvent
{
public:
    AcceptEvent() : IocpEvent(EventType::Accept) {}

    void SetSession(Session* session) { _session = session; }
    Session* GetSession() { return _session; }

private:
    Session* _session = nullptr;
};

/*-----------------
    RecvEvent
------------------*/

class RecvEvent : public IocpEvent
{
public:
    RecvEvent() : IocpEvent(EventType::Recv) {}
};

/*-----------------
    SendEvent
------------------*/

class SendEvent : public IocpEvent
{
public:
    SendEvent() : IocpEvent(EventType::Send) {}
};