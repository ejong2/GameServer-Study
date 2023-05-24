#pragma once

/*--------------------
      IocpObject
---------------------*/

// ���� IO ��ü�� ���� �������̽��� ������ Ŭ����
class IocpObject
{
public:
    // �� IO ��ü�� �ڵ��� ��ȯ�ϴ� ���� �Լ�
    virtual HANDLE GetHandle() abstract;
    // IOCP���� �̺�Ʈ�� �߻����� �� ȣ��Ǵ� ���� �Լ�
    virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) abstract;
};

/*--------------------
       IocpCore
---------------------*/

// IOCP�� �����ϴ� �ٽ� Ŭ����
class IocpCore
{
public:
    IocpCore();  // �����ڿ��� IOCP �ڵ��� �ʱ�ȭ��
    ~IocpCore();  // �Ҹ��ڿ��� IOCP �ڵ��� ����

    // IOCP �ڵ��� ��ȯ�ϴ� �Լ�
    HANDLE GetHandle() { return _iocpHandle; }

    // IOCP�� IocpObject�� ����ϴ� �Լ�
    bool Register(class IocpObject* iocpObject);
    // IOCP���� �̺�Ʈ�� ������ ó���ϴ� �Լ�
    bool Dispatch(int32 timeoutMs = INFINITE);

private:
    HANDLE _iocpHandle;  // IOCP �ڵ�
};

// TEMP
extern IocpCore GIocpCore;


