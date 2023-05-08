#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>

class SpinLock
{
public:
    void lock()
    {
        // CAS (Compare-And-Swap)
        bool expected = false;
        bool desired = true;

        while (_locked.compare_exchange_strong(expected, desired) == false)
        {
            expected = false;

             /* sleep을 사용하여 현재 스레드를 일정 시간 동안 대기시킵니다.
             이를 통해 다른 스레드가 lock을 얻을 수 있는 기회를 제공하고, CPU 사용률을 낮춥니다.
             std::chrono::milliseconds(100)를 사용하면 스레드가 100밀리초 동안 대기합니다.
            this_thread::sleep_for(std::chrono::milliseconds(100)); */

            // 0ms를 사용하면 스레드가 최소한의 시간만 대기하고 다시 lock을 얻으려고 시도합니다.
            this_thread::sleep_for(0ms);

            // this_thread::yield()를 사용하면 스레드가 자발적으로 CPU를 양보하여 다른 스레드에게 실행 기회를 제공합니다.
            //this_thread::yield();
        }
    }

    void unlock()
    {
        _locked.store(false);
    }

private:
    std::atomic<bool> _locked = false;
};

int32 sum = 0;
std::mutex m;
SpinLock spinLock;

void Add()
{
    for (int32 i = 0; i < 10'0000; i++)
    {
        std::lock_guard<SpinLock> guard(spinLock);
        sum++;
    }
}

void Sub()
{
    for (int32 i = 0; i < 10'0000; i++)
    {
        std::lock_guard<SpinLock> guard(spinLock);
        sum--;
    }
}

int main()
{

    std::thread t1(Add);
    std::thread t2(Sub);

    t1.join();
    t2.join();

    std::cout << sum << '\n';
}
