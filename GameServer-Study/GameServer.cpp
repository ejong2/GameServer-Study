#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include <vector>
#include <thread>

// 소수를 판별하는 함수
bool isPrime(int number)
{
	if (number <= 1)  // 1 이하의 수는 소수가 아니다.
		return false;
	if (number == 2 || number == 3)  // 2와 3은 소수다.
		return true;

	// 2부터 number-1까지로 나누어 떨어지는지 확인한다.
	for (int i = 2; i < number; i++)
	{
		if ((number % i) == 0)  // 나누어 떨어지면 소수가 아니다.
			return false;
	}
	return true;  // 나누어 떨어지는 수가 없다면 소수다.
}

// 주어진 범위 내의 소수 개수를 계산하는 함수
int CountPrime(int start, int end)
{
	int count = 0;

	// start부터 end까지의 숫자 중에서 소수인 숫자를 찾는다.
	for (int number = start; number <= end; number++)
	{
		if (isPrime(number))  // 소수라면 카운트를 증가시킨다.
			count++;
	}
	return count;
}

int main()
{
	const int MAX_NUMBER = 100'0000;  // 최대 범위 설정

	vector<thread> threads;

	int coreCount = thread::hardware_concurrency();  // 사용 가능한 CPU 코어 개수
	int jobCount = (MAX_NUMBER / coreCount) + 1;  // 각 쓰레드가 처리할 작업 범위

	atomic<int> primeCount = 0;  // 소수 개수를 담을 변수 (멀티스레드 환경에서 안전하게 동작하도록 atomic 타입 사용)
	for (int i = 0; i < coreCount; i++)
	{
		int start = (i * jobCount) + 1;  // 각 쓰레드가 처리할 시작 번호
		int end = min(MAX_NUMBER, ((i + 1) * jobCount));  // 각 쓰레드가 처리할 끝 번호

		// 각 쓰레드에게 범위 내 소수 개수 계산 작업을 분배한다.
		threads.push_back(thread([start, end, &primeCount]()
			{
				primeCount += CountPrime(start, end);  // 해당 범위의 소수 개수를 구해 primeCount에 더한다.
			}));
	}

	// 모든 쓰레드가 작업을 마칠 때까지 대기한다.
	for (thread& t : threads)
		t.join();

	// 최종 계산된 소수의 개수를 출력한다.
	cout << primeCount << endl;
}
