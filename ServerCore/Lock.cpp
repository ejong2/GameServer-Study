#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h"
#include "DeadLockProfiler.h"

void Lock::WriteLock(const char* name)
{
#if _DEBUG
    GDeadLockProfiler->PushLock(name);
#endif

    // ������ �����尡 �����ϰ� �ִٸ� ������ ����.
    const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
    if (LThreadId == lockThreadId)
    {
        _writeCount++;
        return;
    }

    // �ƹ��� ���� �� �����ϰ� ���� ���� ��, �����ؼ� �������� ��´�.
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

void Lock::WriteUnlock(const char* name)
{
#if _DEBUG
    GDeadLockProfiler->PopLock(name);
#endif

    // ReadLock �� Ǯ�� ������ WriteUnlock �Ұ���.
    if ((_lockFlag.load() & READ_COUNT_MASK) != 0)
        CRASH("INVALID_UNLOCK_ORDER");

    const int32 lockCount = --_writeCount;
    if (lockCount == 0)
        _lockFlag.store(EMPTY_FLAG);
}
/*------------------------------------------------------------------------------------------------------------------
�� �ڵ�� ���� ���� �����ϰ� �����ϴ� �Լ��� ������ ���Դϴ�.
WriteLock() �Լ��� �����尡 ���� ���� ȹ���Ϸ��� �� �� ȣ��Ǹ�,
WriteUnlock() �Լ��� �����尡 ���� ���� �����Ϸ��� �� �� ȣ��˴ϴ�.

WriteLock() �Լ��� ���� ������ �����尡 �̹� ���� �����ϰ� �ִ��� Ȯ���մϴ�.
���� ������ �����尡 ���� �����ϰ� �ִٸ�, �� ȹ�濡 ������ ������ �����ϰ� �Լ��� �����մϴ�.
����, �ƹ��� ���� �����ϰų� �����ϰ� ���� ���� ���, ������ ���� ���� �������� ����ϴ�.
���� ���� �ð����� �� ȹ�濡 ������ ���, ���α׷��� "LOCK_TIMEOUT" ������ �߻���Ű�� ����˴ϴ�.

WriteUnlock() �Լ��� ���� �����ϴ� �Լ��Դϴ�.
����, ��� �б� ���� �����Ǿ����� Ȯ���մϴ�. 
���� �������� ���� �б� ���� �ִٸ� "INVALID_UNLOCK_ORDER" ������ �߻���ŵ�ϴ�. 
����, ���� �� ī��Ʈ�� ���ҽ�Ű��, ��� ���� ���� �����Ǿ��� ��� �� �÷��׸� EMPTY_FLAG�� �����Ͽ� ���� �����մϴ�.
--------------------------------------------------------------------------------------------------------------------*/

void Lock::ReadLock(const char* name)
{
#if _DEBUG
    GDeadLockProfiler->PushLock(name);
#endif

    // ������ �����尡 �����ϰ� �ִٸ� ������ ����.
    const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
    if (LThreadId == lockThreadId)
    {
        _lockFlag.fetch_add(1);
        return;
    }

    // �ƹ��� �����ϰ� ���� ���� �� �����ؼ� ���� ī��Ʈ�� �ø���.
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

void Lock::ReadUnlock(const char* name)
{
#if _DEBUG
    GDeadLockProfiler->PopLock(name);
#endif

    if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
        CRASH("MULTIPLE_UNLOCK");
}

/*----------------------------------------------------------------------------------------------
�� �ڵ�� �б� ���� �����ϰ� �����ϴ� �Լ��� ������ ���Դϴ�.
ReadLock() �Լ��� �����尡 �б� ���� ȹ���Ϸ��� �� �� ȣ��Ǹ�,
ReadUnlock() �Լ��� �����尡 �б� ���� �����Ϸ��� �� �� ȣ��˴ϴ�.

ReadLock() �Լ��� ���� ������ �����尡 �̹� ���� �����ϰ� �ִ��� Ȯ���մϴ�.
���� ������ �����尡 ���� �����ϰ� �ִٸ�, ���� ī��Ʈ�� ������Ű�� �Լ��� �����մϴ�.
���� �ƹ��� ���� �����ϰ� ���� ���� ���, ������ ���� ���� ī��Ʈ�� ������ŵ�ϴ�.
���� ���� �ð����� �� ȹ�濡 ������ ���, ���α׷��� "LOCK_TIMEOUT" ������ �߻���Ű�� ����˴ϴ�.

ReadUnlock() �Լ��� ���� ī��Ʈ�� ���ҽ�Ű��,
���� ��� �б� ���� �����Ǿ��ٸ� "MULTIPLE_UNLOCK" ������ �߻���ŵ�ϴ�.
------------------------------------------------------------------------------------------------*/