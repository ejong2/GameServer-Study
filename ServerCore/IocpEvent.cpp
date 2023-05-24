#include "pch.h"
#include "IocpEvent.h"

/*-----------------
     iocpEvent
------------------*/

IocpEvent::IocpEvent(EventType type) : _type(type)
{
    // �̺�Ʈ �ʱ�ȭ
    Init();
}

void IocpEvent::Init()
{
    // OVERLAPPED ����ü �ʱ�ȭ
    OVERLAPPED::hEvent = 0;
    OVERLAPPED::Internal = 0;
    OVERLAPPED::InternalHigh = 0;
    OVERLAPPED::Offset = 0;
    OVERLAPPED::OffsetHigh = 0;
}
