#pragma once
#include "Types.h"

/*----------------
    RW SpinLock
-----------------*/

/*--------------------------------------------
[WWWWWWWW][WWWWWWWW][RRRRRRRR][RRRRRRRR]
W : WriteFlag (Exclusive Lock Owner ThreadId)
R : ReadFlag (Shared Lock Count)
---------------------------------------------*/

class Lock
{
    // Lock 클래스는 Reader-Writer Lock을 관리합니다.
    // WriteFlag는 쓰레드 ID를 저장하며, ReadFlag는 공유 락 카운트를 저장합니다.

    enum : uint32
    {
        ACQUIRE_TIMEOUT_TICK = 10000, // 락 획득 시도 시간 제한
        MAX_SPIN_COUNT = 5000, // 최대 스핀 횟수
        WRITE_THREAD_MASK = 0xFFFF'0000, // 쓰레드 ID를 추출하기 위한 마스크
        READ_COUNT_MASK = 0x0000'FFFF, // 공유 락 카운트를 추출하기 위한 마스크
        EMPTY_FLAG = 0x0000'0000 // 락이 해제된 상태를 나타내는 플래그
    };
public:
    void WriteLock(); // 쓰기 락을 획득하는 함수
    void WriteUnlock(); // 쓰기 락을 해제하는 함수
    void ReadLock(); // 읽기 락을 획득하는 함수
    void ReadUnlock(); // 읽기 락을 해제하는 함수

private:
    Atomic<uint32> _lockFlag = EMPTY_FLAG; // 락 플래그 (상위 16비트는 쓰레드 ID, 하위 16비트는 공유 락 카운트를 저장)
    uint16 _writeCount = 0; // 쓰기 락 카운트
};

/*----------------
    LockGuards
----------------*/

class ReadLockGuard
{
    // ReadLockGuard 클래스는 RAII(Resource Acquisition Is Initialization) 방식을 이용해 읽기 락을 관리합니다.
    // 생성자에서 락을 획득하고, 소멸자에서 락을 해제합니다.

public:
    ReadLockGuard(Lock& lock) : _lock(lock) { _lock.ReadLock(); }
    ~ReadLockGuard() { _lock.ReadUnlock(); }

private:
    Lock& _lock;
};

class WriteLockGuard
{
    // ReadLockGuard 클래스는 RAII(Resource Acquisition Is Initialization) 방식을 이용해 읽기 락을 관리합니다.
    // 생성자에서 락을 획득하고, 소멸자에서 락을 해제합니다.

public:
    WriteLockGuard(Lock& lock) : _lock(lock) { _lock.WriteLock(); }
    ~WriteLockGuard() { _lock.WriteUnlock(); }

private:
    Lock& _lock;
};

/*----------------------------------------------------------------------------------------------
위의 코드는 ReadLockGuard, WriteLockGuard 클래스를 정의한 것입니다.
이 클래스 역시 RAII(Resource Acquisition Is Initialization) 방식을 이용해 쓰기 락을 관리합니다.
이 객체가 생성되면서 쓰기 락이 설정되고, 객체가 소멸할 때 쓰기 락이 해제됩니다.
따라서 이 클래스를 이용하면 쓰기 락을 안전하게 관리할 수 있습니다.
-----------------------------------------------------------------------------------------------*/