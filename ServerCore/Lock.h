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
    // Lock Ŭ������ Reader-Writer Lock�� �����մϴ�.
    // WriteFlag�� ������ ID�� �����ϸ�, ReadFlag�� ���� �� ī��Ʈ�� �����մϴ�.

    enum : uint32
    {
        ACQUIRE_TIMEOUT_TICK = 10000, // �� ȹ�� �õ� �ð� ����
        MAX_SPIN_COUNT = 5000, // �ִ� ���� Ƚ��
        WRITE_THREAD_MASK = 0xFFFF'0000, // ������ ID�� �����ϱ� ���� ����ũ
        READ_COUNT_MASK = 0x0000'FFFF, // ���� �� ī��Ʈ�� �����ϱ� ���� ����ũ
        EMPTY_FLAG = 0x0000'0000 // ���� ������ ���¸� ��Ÿ���� �÷���
    };
public:
    void WriteLock(const char* name); // ���� ���� ȹ���ϴ� �Լ�
    void WriteUnlock(const char* name); // ���� ���� �����ϴ� �Լ�
    void ReadLock(const char* name); // �б� ���� ȹ���ϴ� �Լ�
    void ReadUnlock(const char* name); // �б� ���� �����ϴ� �Լ�

private:
    Atomic<uint32> _lockFlag = EMPTY_FLAG; // �� �÷��� (���� 16��Ʈ�� ������ ID, ���� 16��Ʈ�� ���� �� ī��Ʈ�� ����)
    uint16 _writeCount = 0; // ���� �� ī��Ʈ
};

/*----------------
    LockGuards
----------------*/

class ReadLockGuard
{
    // ReadLockGuard Ŭ������ RAII(Resource Acquisition Is Initialization) ����� �̿��� �б� ���� �����մϴ�.
    // �����ڿ��� ���� ȹ���ϰ�, �Ҹ��ڿ��� ���� �����մϴ�.

public:
    ReadLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name) { _lock.ReadLock(name); }
    ~ReadLockGuard() { _lock.ReadUnlock(_name); }

private:
    Lock& _lock;
    const char* _name;
};

class WriteLockGuard
{
    // ReadLockGuard Ŭ������ RAII(Resource Acquisition Is Initialization) ����� �̿��� �б� ���� �����մϴ�.
    // �����ڿ��� ���� ȹ���ϰ�, �Ҹ��ڿ��� ���� �����մϴ�.

public:
    WriteLockGuard(Lock& lock, const char* name) : _lock(lock), _name(name) { _lock.WriteLock(name); }
    ~WriteLockGuard() { _lock.WriteUnlock(_name); }

private:
    Lock& _lock;
    const char* _name;
};

/*----------------------------------------------------------------------------------------------
���� �ڵ�� ReadLockGuard, WriteLockGuard Ŭ������ ������ ���Դϴ�.
�� Ŭ���� ���� RAII(Resource Acquisition Is Initialization) ����� �̿��� ���� ���� �����մϴ�.
�� ��ü�� �����Ǹ鼭 ���� ���� �����ǰ�, ��ü�� �Ҹ��� �� ���� ���� �����˴ϴ�.
���� �� Ŭ������ �̿��ϸ� ���� ���� �����ϰ� ������ �� �ֽ��ϴ�.
-----------------------------------------------------------------------------------------------*/