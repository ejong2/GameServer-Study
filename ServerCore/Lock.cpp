#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h"

void Lock::WriteLock()
{
    // 동일한 쓰레드가 소유하고 있다면 무조건 성공.
    const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
    if (LThreadId == lockThreadId)
    {
        _writeCount++;
        return;
    }

    // 아무도 소유 및 공유하고 있지 않을 때, 경합해서 소유권을 얻는다.
    const int64 beginTick = ::GetTickCount64();
    const uint32 desired = ((LThreadId << 16) & WRITE_THREAD_MASK);
    while (true)
    {
        for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
        {
            uint32 expected = EMPTY_FLAG;
            if (_lockFlag.compare_exchange_strong(OUT expected, desired))
            {
                _writeCount++;
                return;
            }
        }

        if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
            CRASH("LOCK_TIMEOUT");

        this_thread::yield();
    }
}

void Lock::WriteUnlock()
{
    // ReadLock 다 풀기 전에는 WriteUnlock 불가능.
    if ((_lockFlag.load() & READ_COUNT_MASK) != 0)
        CRASH("INVALID_UNLOCK_ORDER");

    const int32 lockCount = --_writeCount;
    if (lockCount == 0)
        _lockFlag.store(EMPTY_FLAG);
}
/*------------------------------------------------------------------------------------------------------------------
위 코드는 쓰기 락을 설정하고 해제하는 함수를 구현한 것입니다.
WriteLock() 함수는 쓰레드가 쓰기 락을 획득하려고 할 때 호출되며,
WriteUnlock() 함수는 쓰레드가 쓰기 락을 해제하려고 할 때 호출됩니다.

WriteLock() 함수는 먼저 동일한 쓰레드가 이미 락을 소유하고 있는지 확인합니다.
만약 동일한 쓰레드가 락을 소유하고 있다면, 락 획득에 성공한 것으로 간주하고 함수를 종료합니다.
이후, 아무도 락을 소유하거나 공유하고 있지 않을 경우, 경합을 통해 락의 소유권을 얻습니다.
만약 일정 시간동안 락 획득에 실패할 경우, 프로그램은 "LOCK_TIMEOUT" 오류를 발생시키며 종료됩니다.

WriteUnlock() 함수는 락을 해제하는 함수입니다.
먼저, 모든 읽기 락이 해제되었는지 확인합니다. 
아직 해제되지 않은 읽기 락이 있다면 "INVALID_UNLOCK_ORDER" 오류를 발생시킵니다. 
이후, 쓰기 락 카운트를 감소시키고, 모든 쓰기 락이 해제되었을 경우 락 플래그를 EMPTY_FLAG로 설정하여 락을 해제합니다.
--------------------------------------------------------------------------------------------------------------------*/

void Lock::ReadLock()
{
    // 동일한 쓰레드가 소유하고 있다면 무조건 성공.
    const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
    if (LThreadId == lockThreadId)
    {
        _lockFlag.fetch_add(1);
        return;
    }

    // 아무도 소유하고 있지 않을 때 경합해서 공유 카운트를 올린다.
    const int64 beginTick = ::GetTickCount64();
    while (true)
    {
        for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
        {
            uint32 expected = (_lockFlag.load() & READ_COUNT_MASK);
            if (_lockFlag.compare_exchange_strong(OUT expected, expected + 1))
                return;
        }

        if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
            CRASH("LOCK_TIMEOUT");

        this_thread::yield();
    }
}

void Lock::ReadUnlock()
{
    if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
        CRASH("MULTIPLE_UNLOCK");
}

/*----------------------------------------------------------------------------------------------
위 코드는 읽기 락을 설정하고 해제하는 함수를 구현한 것입니다.
ReadLock() 함수는 쓰레드가 읽기 락을 획득하려고 할 때 호출되며,
ReadUnlock() 함수는 쓰레드가 읽기 락을 해제하려고 할 때 호출됩니다.

ReadLock() 함수는 먼저 동일한 쓰레드가 이미 락을 소유하고 있는지 확인합니다.
만약 동일한 쓰레드가 락을 소유하고 있다면, 공유 카운트를 증가시키고 함수를 종료합니다.
만약 아무도 락을 소유하고 있지 않을 경우, 경합을 통해 공유 카운트를 증가시킵니다.
만약 일정 시간동안 락 획득에 실패할 경우, 프로그램은 "LOCK_TIMEOUT" 오류를 발생시키며 종료됩니다.

ReadUnlock() 함수는 공유 카운트를 감소시키고,
만약 모든 읽기 락이 해제되었다면 "MULTIPLE_UNLOCK" 오류를 발생시킵니다.
------------------------------------------------------------------------------------------------*/