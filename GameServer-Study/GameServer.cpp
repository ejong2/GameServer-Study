#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>

mutex m;
queue<int32> q;
HANDLE handle;

// 참고) CV는 User-Level Object (커널 오브젝트 X)

/* condition_variable은 C++ 표준 라이브러리에서 제공하는 동기화 메커니즘으로, 특정 조건을 만족할 때까지 스레드를 대기 상태로 둘 수 있습니다.
 이를 통해 스레드 간의 작업 순서를 보장할 수 있으며, 자원 경쟁 및 동시성 문제를 해결할 수 있습니다. */
condition_variable cv;

void Producer()
{
    while (true)
    {
        // 1) Lock을 잡고
        // 2) 공유 변수 값을 수정
        // 3) Lock을 풀고
        // 4) 조건변수 통해 다른 쓰레드에게 통지
        {
            unique_lock<mutex> lock(m);
            q.push(100);
        }

        cv.notify_one(); // 대기 중인 스레드가 있으면 하나를 깨운다.

        //this_thread::sleep_for(10000ms);
    }
}

void Consumer()
{
    while (true)
    {
        unique_lock<mutex> lock(m);
        cv.wait(lock, []() { return q.empty() == false; });
        // 1) Lock을 잡고
        // 2) 조건 확인
        // - 만족O => 빠져 나와서 이어서 코드를 진행
        // - 만족X => Lock을 풀어주고 대기 상태

        // 그런데 notify_one을 했으면 항상 조건식을 만족하는거 아닐까?
        // Spurious Wakeup (가짜 기상?)
        // notify_one할 때 lock을 잡고 있는 것이 아니기 때문

        //while (q.empty() == false)
        {
            int32 data = q.front();
            q.pop();
            cout << q.size() << '\n';
        }
    }
}

int main()
{
    // 커널 오브젝트
    // Usage Count
    // Signal (파란불) / Non-Signal (빨간불) << bool
    // Auto / Manual << bool

    thread t1(Producer);
    thread t2(Consumer);

    t1.join();
    t2.join();
}

/* condition_variable과 Event를 사용한 Lock의 주요 차이점은 다음과 같습니다.

플랫폼 독립성: condition_variable은 C++ 표준 라이브러리의 일부로, 플랫폼에 종속되지 않습니다. 반면에 Event는 Windows API를 사용하므로 플랫폼에 종속됩니다.

사용 방식: condition_variable은 wait() 함수를 사용하여 특정 조건을 만족할 때까지 스레드를 대기 상태로 둘 수 있습니다. 이를 통해 스레드 간의 작업 순서를 보장할 수 있습니다. Event는 신호 상태와 비신호 상태를 통해 스레드 간의 동기화를 처리합니다. */

// -----------------------------------------------------------------------------------------------------

/* 가짜 기상(Spurious Wakeup) 참고 설명

주석에서 언급된 "가짜 기상(Spurious Wakeup)"은 대기 상태에서 깨어난 스레드가 실제로 깨어날 조건을 만족하지 않았음에도 불구하고 깨어나는 현상을 말합니다. 이 현상은 대부분의 시스템에서 거의 발생하지 않지만, 발생할 수 있는 가능성이 있기 때문에 고려해야 합니다.

가짜 기상이 발생할 수 있는 이유는 notify_one() 함수가 뮤텍스를 잡고 있지 않기 때문입니다. 따라서, notify_one() 함수가 호출되었을 때 항상 조건식이 만족되는 것은 아닙니다.

이러한 가짜 기상 문제를 해결하기 위해 cv.wait() 함수는 람다 함수를 인수로 사용하여 조건을 검사합니다. 이를 통해 스레드가 깨어날 때마다 해당 조건이 만족되었는지 확인할 수 있습니다. 조건이 만족되지 않으면 스레드는 다시 대기 상태로 돌아갑니다.

예를 들어, 다음 코드에서 cv.wait(lock, []() { return q.empty() == false; }); 는 스레드가 깨어났을 때 큐가 비어있지 않은지 확인합니다. 만약 큐가 비어있다면, 스레드는 다시 대기 상태로 돌아갑니다.

이렇게 조건 변수의 wait() 함수에 조건을 전달함으로써, 가짜 기상 현상을 방지하고 올바른 동작을 보장할 수 있습니다. */