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
    bool StartAccept(NetAddress netAddress); // ������ ���� ������ �ޱ� �����ϴ� �޼���
    void CloseSocket(); // �����ִ� ������ �ݴ� �޼���

public:
    /* �������̽� ���� */
    virtual HANDLE GetHandle() override; // Listener�� ���� �ڵ��� �������� �޼���
    virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) override; // IO �Ϸ� �̺�Ʈ ó�� �޼���

private:
    /* ���� ���� */
    void RegisterAccept(AcceptEvent* acceptEvent); // Accept �۾� ��� �޼���
    void ProcessAccept(AcceptEvent* acceptEvent); // Accept �Ϸ� ó�� �޼���

protected:
    SOCKET _socket = INVALID_SOCKET; // Listener�� ����
    Vector<AcceptEvent*> _acceptEvents; // AcceptEvent ����Ʈ
};

