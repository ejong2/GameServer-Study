#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <future>

int64 result;

int64 Calculate()
{
	int64 sum = 0;

	for (int32 i = 0; i < 100'000; i++)
	{
		sum += i;
	}

	result = sum;

	return sum;
}

void PromiseWorker(std::promise<string>&& promise)
{
	promise.set_value("비밀 메시지");
}

void TaskWorker(std::packaged_task<int64(void)>&& task)
{
	task();
}

int main()
{
	// 동기 (synchronous) 실행 예제
	// int64 sum = Calculate();
	// cout << sum << '\n';

	// std::future 사용법
	{
		// 1) deferred		   -> 지연 평가 (lazy evaluation)
		// 2) async			   -> 별도의 쓰레드에서 실행
		// 3) deferred | async -> 두 가지 중 적절한 방법 선택

		// 언젠가 미래에 결과를 반환하는 객체 생성
		std::future<int64> future = std::async(std::launch::async, Calculate);

		// TODO
		//std::future_status status = future.wait_for(1ms);

		int64 sum = future.get(); // 여기서 결과를 가져옴

		/*class Knight // 객체를 사용하는 경우 예제
		{
		public:
			int64 GetHp() { return 100; }
		};

		Knight knight;
		std::future<int64> future2 = std::async(std::launch::async, &Knight::GetHp, knight);*/
	}

	// std::promise 사용법
	{
		// 미래(std::future)에 결과를 반환할 것이라는 약속(std::promise) 생성 (계약서 느낌)
		std::promise<string> promise;
		std::future<string> future = promise.get_future();

		thread t(PromiseWorker, std::move(promise));

		string message = future.get();
		cout << message << '\n';

		t.join();
	}

	// std::packaged_task 사용법
	{
		std::packaged_task<int64(void)> task(Calculate);
		std::future<int64> future = task.get_future();

		std::thread t(TaskWorker, std::move(task));

		int64 sum = future.get();
		cout << sum << '\n';

		t.join();
	}

	// 결론)
	// mutex, condition_variable까지 사용하지 않고, 간단한 작업을 처리할 수 있다.
	// 특히, 한 번 발생하는 이벤트에 유용하다.

	// 1) async			-> 원하는 함수를 비동기적으로 실행
	// 2) promise		-> 결과물을 promise를 통해 future로 받아줌
	// 3) packaged_task -> 원하는 함수의 실행 결과를 packaged_task를 통해 future로 받아줌
}

