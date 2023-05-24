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

// �� �̺�Ʈ Ÿ���� �����ϴ� Ŭ����
class IocpEvent : public OVERLAPPED
{
public:
    IocpEvent(EventType type);  // �̺�Ʈ Ÿ�� ����

    void Init();  // �̺�Ʈ �ʱ�ȭ
    EventType GetType() { return _type; }  // �̺�Ʈ Ÿ�� ��ȯ

protected:
    EventType _type;  // �̺�Ʈ Ÿ��
};

// ���� ���� �̺�Ʈ Ÿ�Կ� ���� Ŭ���� ���� ---------------------------------

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