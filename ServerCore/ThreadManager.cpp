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

// 쓰레드를 실행하고 TLS를 초기화하고 파괴하는 작업을 수행합니다.
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

// 모든 쓰레드를 join하고 벡터를 비웁니다.
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