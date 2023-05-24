#pragma once

/*--------------------
      IocpObject
---------------------*/

// 각종 IO 객체에 대한 인터페이스를 정의한 클래스
class IocpObject
{
public:
    // 각 IO 객체의 핸들을 반환하는 가상 함수
    virtual HANDLE GetHandle() abstract;
    // IOCP에서 이벤트가 발생했을 때 호출되는 가상 함수
    virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfBytes = 0) abstract;
};

/*--------------------
       IocpCore
---------------------*/

// IOCP를 관리하는 핵심 클래스
class IocpCore
{
public:
    IocpCore();  // 생성자에서 IOCP 핸들을 초기화함
    ~IocpCore();  // 소멸자에서 IOCP 핸들을 닫음

    // IOCP 핸들을 반환하는 함수
    HANDLE GetHandle() { return _iocpHandle; }

    // IOCP에 IocpObject를 등록하는 함수
    bool Register(class IocpObject* iocpObject);
    // IOCP에서 이벤트를 가져와 처리하는 함수
    bool Dispatch(int32 timeoutMs = INFINITE);

private:
    HANDLE _iocpHandle;  // IOCP 핸들
};

// TEMP
extern IocpCore GIocpCore;


