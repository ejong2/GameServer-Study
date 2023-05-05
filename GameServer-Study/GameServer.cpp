/* 이 코드는 Atomic 객체를 사용하여 공유 데이터를 관리하는 방법을 보여주는 예제입니다.
Atomic 객체는 All-Or-Nothing 원칙을 따르며, 이를 통해 멀티스레드 환경에서 공유 데이터를 안전하게 처리할 수 있습니다.
단, Atomic 객체를 사용할 때 속도 관련 이슈가 발생할 수 있으니 주의해야 합니다. */

#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>

// atomic : All-Or-Nothing
// sum 변수를 atomic<int32> 타입으로 선언하여 스레드 간 공유 데이터를 안전하게 관리합니다.
atomic<int32> sum = 0;

// 첫 번째 함수: Add
// sum 값을 1,000,000번 증가시킵니다.
void Add()
{
	for (int32 i = 0; i < 100'0000; i++)
	{
		// fetch_add() 함수를 사용하여 sum 값을 안전하게 증가시킵니다.
		sum.fetch_add(1);
		
		//sum++;
		/*int32 eax = sum;
		eax = eax + 1;
		sum = eax;*/
	}
}

// 두 번째 함수: Sub
// sum 값을 1,000,000번 감소시킵니다.
void Sub()
{
	for (int32 i = 0; i < 100'0000; i++)
	{
		// fetch_add() 함수를 사용하여 sum 값을 안전하게 감소시킵니다. (-1을 전달합니다)
		sum.fetch_add(-1);

		//sum--;
		/*int32 eax = sum;
		eax = eax - 1;
		sum = eax;*/
	}
}

int main()
{
	// Add()와 Sub() 함수를 순차적으로 호출하여 sum 값을 계산하고 출력합니다.
	Add();
	Sub();

	cout << sum << '\n';

	// 각각의 Add와 Sub 함수를 스레드 t1과 t2에서 동시에 실행합니다.
	std::thread t1(Add);
	std::thread t2(Sub);

	// 두 개의 스레드를 join()을 통해 메인 스레드와 동기화하여 각각의 스레드가 작업을 완료한 후 메인 스레드가 진행되도록 합니다.
	t1.join();
	t2.join();

	// 최종적으로 계산된 sum 값을 출력합니다.
	cout << sum << '\n';

	return 0;
}

/*이 예제에서는 atomic<int32>를 사용하여 sum 변수를 선언하고 fetch_add() 함수를 사용하여 값을 안전하게 증가시키거나 감소시킵니다. 이를 통해 경쟁 조건(race condition)을 피할 수 있습니다. 그러나 atomic 객체를사용하면 속도 관련 이슈가 발생할 수 있습니다. 이는 atomic 객체가 원자적 연산을 보장하기 위해 스레드 간 동기화를 수행하기 때문입니다. 따라서 성능이 중요한 상황에서는 atomic 객체 사용에 주의해야 합니다.*/