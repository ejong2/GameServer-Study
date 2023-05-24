#include "pch.h"
#include "IocpEvent.h"

/*-----------------
     iocpEvent
------------------*/

IocpEvent::IocpEvent(EventType type) : _type(type)
{
    // 이벤트 초기화
    Init();
}

void IocpEvent::Init()
{
    // OVERLAPPED 구조체 초기화
    OVERLAPPED::hEvent = 0;
    OVERLAPPED::Internal = 0;
    OVERLAPPED::InternalHigh = 0;
    OVERLAPPED::Offset = 0;
    OVERLAPPED::OffsetHigh = 0;
}
