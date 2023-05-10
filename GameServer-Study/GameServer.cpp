#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>

// 메모리 정책

atomic<bool> flag;
atomic<bool> ready;
int32 value;

void Producer()
{
	value = 10;

	ready.store(true, memory_order::memory_order_seq_cst);

	std::atomic_thread_fence(memory_order::memory_order_release);
	// ready.store(true, memory_order::memory_order_release);
	// ----------------- 절취선 -----------------
}

void Consumer()
{
	while (ready.load(memory_order::memory_order_seq_cst) == false)
		;

	// ----------------- 절취선 -----------------
	// while (ready.load(memory_order::memory_order_acquire) == false);
		

	cout << value << '\n';
}

int main()
{
	ready = false;
	value = 0;
	thread t1(Producer);
	thread t2(Consumer);

	t1.join();
	t2.join();

	// Memory Model (정책)
	// 1) Sequentially Consistent (seq_cst)
	// 2) Acquire-Release (acquire, release)
	// 3) Relaxed (relaxed)

	// 1) seq_cst (가장 엄격 = 컴파일러 최적화 여지 적음 = 직관적)
	// 가시성 문제해결, 코드 재배치 문제 해결
	
	// 2) acquire-release
	// 딱 중간 !
	// release 명령 이전의 메모리 명령들이, 해당 명령 이후로 재배치 되는 것을 금지
	// 그리고 acquire로 같은 변수를 읽는 쓰레드가 있다면 -> release 이전의 명령들이 -> acquire 하는 순간에 관찰 가능 (가시성)

	// 3) relaxed (자유롭다 = 컴파일러 최적화 여지 많음 = 직관적이지 않음)
	// 너무나도 자유롭다 !
	// 코드 재배치도 멋대로 가능! 가시성 해결 NOT
	// 가장 기본 조건 (동일 객체에 대한 동일 관전 순서만 보장)

	// 인텔, AMD의 경우 애당초 순차척 일관성을 보장을 해서,
	// seq_cst를 써도 별다른 부하가 없음
	// ARM의 경우 꽤 차이가 있다는 얘기가 있음
}

int main_test()
{
	flag = false;

	//flag = true;
	flag.store(true);

	//bool val = flag;
	bool val = flag.load(memory_order::memory_order_seq_cst);

	// 이전 flag 값을 prev에 넣고, flag 값을 수정
	{
		bool prev = flag.exchange(true);
		//bool prev = flag;
		//flag = true;
	}

	// CAS (Compare-And-Swap) 조건부 수정
	{
		bool expected = false;
		bool desired = true;
		flag.compare_exchange_strong(expected, desired);

		// Spurious Failure
		if (flag == expected)
		{
			// 다른 쓰레드의 Interruption을 받아서 중간에 실패할 수 있음
			//expected = flag;
			flag = desired;
			return true;
		}
		else
		{
			expected = flag;
			return false;
		}

		while (true)
		{
			bool expected = false;
			bool desired = true;
			flag.compare_exchange_weak(expected, desired);
		}
	}
}