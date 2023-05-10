#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>

// 가시성, 코드 재배치
int32 x = 0;
int32 y = 0;
int32 r1 = 0;
int32 r2 = 0;

volatile bool ready;

// Thread_1 함수에서는 변수 y에 1을 할당하고, r1에 변수 x의 값을 할당합니다.
void Thread_1()
{
	while (!ready)
		;

	y = 1;
	r1 = x;
}

// Thread_2 함수에서는 변수 x에 1을 할당하고, r2에 변수 y의 값을 할당합니다.
void Thread_2()
{
	while (!ready)
		;


	x = 1;
	r2 = y;
}

int main()
{
	int32 count = 0;

	while (true)
	{
		ready = false;

		count++;

		x = y = r1 = r2 = 0;

		// Thread_1과 Thread_2를 실행합니다.
		thread t1(Thread_1);
		thread t2(Thread_2);

		ready = true;

		t1.join();
		t2.join();

		// r1과 r2 모두 0인 경우, 즉 CPU 파이프라인의 최적화로 인한 메모리 순서 재배치가 발생한 경우를 확인합니다.
		if (r1 == 0 && r2 == 0)
		{
			break;
		}
	}

	// 해당 상황이 발생한 횟수를 출력합니다.
	cout << count << " 번만에 빠져나옴." << '\n';
}

/* 이 코드는 CPU 파이프라인 최적화로 인해 메모리 순서가 재배치되는 상황을 보여줍니다.
코드에서 두 스레드가 동시에 실행되는데, 각 스레드에서 서로 다른 변수에 1을 할당하고,
서로 반대 변수의 값을 읽어옵니다. 이상적인 상황에서는 r1과 r2가 동시에 0이 될 수 없습니다.
하지만 CPU 파이프라인 최적화로 인해 메모리 순서가 재배치되어 의도치 않은 결과가 발생할 수 있습니다. 이를 통해 파이프라인 최적화의 영향을 확인할 수 있습니다. */