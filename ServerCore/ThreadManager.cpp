#include "pch.h"
#include "ThreadManager.h"
#include "CoreTls.h"
#include "CoreGlobal.h"

ThreadManager::ThreadManager()
{
    // Main Thread
    InitTLS();
}

ThreadManager::~ThreadManager()
{
    Join();
}

// �����带 �����ϰ� TLS�� �ʱ�ȭ�ϰ� �ı��ϴ� �۾��� �����մϴ�.
void ThreadManager::Launch(function<void(void)> callback)
{
    LockGuard guard(_lock);


    _threads.push_back(thread([=]()
        {
            InitTLS();
            callback();
            DestroyTLS();
        }));
}

// ��� �����带 join�ϰ� ���͸� ���ϴ�.
void ThreadManager::Join()
{
    for (thread& t : _threads)
    {
        if (t.joinable())
            t.join();
    }
    _threads.clear();
}

void ThreadManager::InitTLS()
{
    static Atomic<uint32> SThreadId = 1;
    LThreadId = SThreadId.fetch_add(1);
}

void ThreadManager::DestroyTLS()
{

}