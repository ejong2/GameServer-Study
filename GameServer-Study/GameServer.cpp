#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>

// SpinLock 클래스를 정의합니다.
class SpinLock
{
public:
    // lock 함수를 정의합니다.
    void lock()
    {
        // CAS (Compare-And-Swap)
        bool expected = false;
        bool desired = true;

        // CAS 의사 코드
        /*if (_locked == expected)
        {
            expected = _locked;
            _locked = desired;
            return true;
        }
        else
        {
            expected = _locked;
            return false;
        }*/

        // compare_exchange_strong 메서드를 사용하여 _locked 변수의 값을 안전하게 변경합니다.
        while (_locked.compare_exchange_strong(expected, desired) == false)
        {
            expected = false;
        }

        // _locked = true;
    }

    // unlock 함수를 정의합니다.
    void unlock()
    {
        // _locked = false;
        _locked.store(false);
    }

private:
    // atomic<bool> 타입의 _locked 변수를 선언합니다.
    std::atomic<bool> _locked = false;
};

int32 sum = 0;
std::mutex m;
SpinLock spinLock;

// Add 함수를 정의합니다. sum 값을 증가시키는 작업을 수행합니다.
void Add()
{
    for (int32 i = 0; i < 10'0000; i++)
    {
        std::lock_guard<SpinLock> guard(spinLock);
        sum++;
    }
}

// Sub 함수를 정의합니다. sum 값을 감소시키는 작업을 수행합니다.
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
    // volatile 변수 flag를 선언하고 초기화합니다.
    // -> 컴파일러 최적화로 인한 문제를 방지하기 위해 volatile 키워드를 사용합니다.

    /*volatile bool flag = true;
    while (flag)
    {

    }*/

    // Add()와 Sub() 함수를 각각 스레드 t1과 t2에서 실행합니다.
    std::thread t1(Add);
    std::thread t2(Sub);

    // 스레드 t1과 t2를 join()을 통해 메인 스레드와 동기화합니다.
    t1.join();
    t2.join();

    // 최종적으로 계산된 sum 값을 출력합니다.
    std::cout << sum << '\n';
}

/* 이 코드에서는 SpinLock 클래스를 사용하여 멀티스레드 환경에서 공유 데이터를 안전하게 처리하는 방법을 보여줍니다.compare_exchange_strong 메서드를 사용하여 원자적으로 값을 변경하고, 이를 통해 경쟁 조건(race condition)을 방지합니다.주의할 점은 SpinLock이 고성능이긴 하지만, 스레드가 계속해서 CPU를 사용하므로 CPU 사용률이 높아질 수 있습니다. */