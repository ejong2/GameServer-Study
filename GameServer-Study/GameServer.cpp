#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>

mutex m;
queue<int32> q;
HANDLE handle; // Event 핸들 선언

// Producer 메서드: 데이터를 생성하고 큐에 넣는 역할
void Producer()
{
    while (true)
    {
        {
            unique_lock<mutex> lock(m);
            q.push(100);
        }

        ::SetEvent(handle); // Event를 신호 상태로 설정하여 Consumer에게 알림

        this_thread::sleep_for(10000ms);
    }
}

// Consumer 메서드: 큐에서 데이터를 가져와서 처리하는 역할
void Consumer()
{
    while (true)
    {
        ::WaitForSingleObject(handle, INFINITE); // 신호 상태가 될 때까지 대기
        //::ResetEvent(handle); // 수동으로 신호 상태를 리셋하는 경우 사용

        unique_lock<mutex> lock(m);
        if (q.empty() == false)
        {
            int32 data = q.front();
            q.pop();
            cout << data << '\n';
        }
    }
}

int main()
{
    // 커널 오브젝트
    // Usage Count
    // Signal (파란불) / Non-Signal (빨간불) << bool
    // Auto / Manual << bool

    // Event 객체 생성
    handle = ::CreateEvent(NULL/*보안속성*/, FALSE/*bManualReset*/, FALSE/*bInitialState*/, NULL);

    thread t1(Producer);
    thread t2(Consumer);

    t1.join();
    t2.join();

    ::CloseHandle(handle); // Event 핸들 닫기
}


/* SpinLock: 스레드가 Lock을 획득할 수 있을 때까지 루프를 돌면서 기다림.짧은 기다림 시간에 효율적이지만, 장시간 기다릴 경우 CPU 자원 낭비가 심해짐.
 
   Sleep을 활용한 Lock : SpinLock과 비슷하지만, 루프 안에서 스레드를 잠시 멈추어 CPU 자원을 절약.일정 시간마다 Lock 획득 시도를 함.이로 인해 다른 스레드에게 CPU 사용 기회를 줌.

   Event를 활용한 Lock : 스레드가 Event의 신호 상태를 기다리며, 다른 스레드가 신호를 보낼 때까지 대기.이벤트를 사용하면, 스레드가 신호 상태가 될 때까지 일시 중단되어 CPU 자원을 효율적으로 사용함.

   Event를 활용한 Lock 방식은 다른 두 방식과 달리, 스레드가 신호 상태가 될 때까지 일시 중단되기 때문에 CPU 자원을 더 효율적으로 사용한다.이로 인해, 시스템의 전반적인 성능이 향상될 수 있다.그러나 Event 객체를 사용하면 코드가 복잡해질 수 있으며, Windows API와 같은 플랫폼 종속적인 기능을 사용해야 한다. */