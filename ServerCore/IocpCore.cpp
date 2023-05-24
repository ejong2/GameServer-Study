#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"

// TEMP
IocpCore GIocpCore;

/*--------------------
       IocpCore
---------------------*/

IocpCore::IocpCore()
{
    // IOCP 핸들 생성
    _iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
    ASSERT_CRASH(_iocpHandle != INVALID_HANDLE_VALUE);
}

IocpCore::~IocpCore()
{
    // IOCP 핸들 닫기
    ::CloseHandle(_iocpHandle);
}

bool IocpCore::Register(IocpObject* iocpObject)
{
    // IocpObject를 IOCP에 등록
    return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, reinterpret_cast<ULONG_PTR>(iocpObject), 0);
}

bool IocpCore::Dispatch(int32 timeoutMs)
{
    // IOCP로부터 이벤트를 가져와 처리
    // 에러 발생 시 에러 코드에 따라 대응
    DWORD numOfBytes = 0;
    IocpObject* iocpObject = nullptr;
    IocpEvent* iocpEvent = nullptr;

    if (::GetQueuedCompletionStatus(_iocpHandle, OUT & numOfBytes, OUT reinterpret_cast<PULONG_PTR>(&iocpObject),        OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs))
    {
        iocpObject->Dispatch(iocpEvent, numOfBytes);
    }
    else
    {
        int32 errCode = ::WSAGetLastError();
        switch (errCode)
        {
        case WAIT_TIMEOUT:
                return false;
        default:
            // TODO : 로그 찍기
            iocpObject->Dispatch(iocpEvent, numOfBytes);
            break;
        }
    }

    return false;
}
