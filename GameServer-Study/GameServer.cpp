#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ConcurrentQueue.h"
#include "ConcurrentStack.h"

// 멀티스레드 환경에서 사용할 수 있는 LockQueue, LockStack 객체 생성

LockFreeQueue<int32> q;
LockFreeStack<int32> s;

void Push()
{
    while (true)
    {
        int32 value = rand() % 100;
        q.Push(value);

        //this_thread::sleep_for(10ms);
    }
}

void Pop()
{
    while (true)
    {
        auto data = q.TryPop();
        if (data != nullptr)
        {
            cout << (*data) << '\n';
        }
    }
}

int main()
{
    thread t1(Push);
    thread t2(Pop);
    thread t3(Pop);

    t1.join();
    t2.join();
    t3.join();
}

/* 위 코드에서는 멀티스레드 환경에서 자료구조인 Queue와 Stack을 안전하게 사용하기 위해, 뮤텍스를 사용하여 동시성을 보장하는 클래스를 만들어 사용합니다.
이렇게 함으로써, 여러 스레드가 동시에 자료구조에 접근하여 발생할 수 있는 데이터 경쟁이나 충돌을 방지할 수 있습니다. */