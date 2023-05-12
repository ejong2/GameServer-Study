#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>

// 각 스레드에서 고유하게 사용되는 thread_local 변수 LThreadId를 선언합니다.
thread_local int32 LThreadId = 0;

// 스레드의 주요 작업을 수행하는 ThreadMain 함수입니다.
void ThreadMain(int32 threadId)
{
    // 각 스레드에게 전달된 threadId 값을 thread_local 변수 LThreadId에 할당합니다.
    LThreadId = threadId;

    // 무한 루프를 통해 각 스레드가 자신의 고유한 ID를 출력합니다.
    while (true)
    {
        cout << "Hi ! I am Thread " << LThreadId << '\n';
        this_thread::sleep_for(1s);
    }
}

int main()
{
    // 스레드 객체들을 저장할 벡터를 선언합니다.
    vector<thread> threads;

    // 10개의 스레드를 생성하고, 각 스레드에 고유한 threadId를 전달합니다.
    for (int32 i = 0; i < 10; i++)
    {
        int32 threadId = i + 1;
        threads.push_back(thread(ThreadMain, threadId));
    }

    // 모든 스레드가 종료될 때까지 대기합니다.
    for (thread& t : threads)
    {
        t.join();
    }
}

/* 이 코드에서는 TLS(Thread Local Storage)를 사용하여 각 스레드가 고유한 데이터를 저장하고 사용할 수 있도록 했습니다.
TLS를 사용하면 스레드 간에 데이터 충돌이 발생하지 않으며, 동기화 메커니즘을 사용하지 않고 안전하게 데이터에 접근할 수 있습니다.
이로 인해 성능 저하가 발생하지 않습니다.
TLS를 사용하지 않은 경우, 공유 메모리를 사용하여 데이터를 관리해야 합니다. 이 경우, 동시에 여러 스레드가 공유 메모리에 접근하려 할 때 동기화 메커니즘을 사용해야 하며, 이로 인해 성능 저하가 발생할 수 있습니다. */