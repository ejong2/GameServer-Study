#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>

/* 이 코드는 게임 서버를 공부하면서 스레드(thread)에 대한 개념을 이해하기 위한 예제입니다.
스레드(thread)는 프로그램에서 독립적으로 실행되는 최소한의 처리 단위입니다.
이 예제에서는 두 개의 함수와 메인 함수에서 스레드를 생성하여 실행하는 방법을 보여줍니다. */

// 첫 번째 함수: HelloThread
// 출력: "Hello Thread"
void HelloThread()
{
	cout << "Hello Thread" << '\n';
}

// 두 번째 함수: HelloThread_2
// 입력: int32 num (정수형 변수)
// 출력: 입력받은 num 값
void HelloThread_2(int32 num)
{
	cout << num << '\n';
}

int main()
{
	//HelloThread();
	vector<std::thread> v;

	// for문을 사용하여 총 10개의 스레드를 생성합니다.
	for (int32 i = 0; i < 10; i++)
	{
		// 각 스레드는 HelloThread_2 함수를 실행하며, i 값을 인자로 전달합니다.
		v.push_back(std::thread(HelloThread_2, i));
	}

	// 생성된 10개의 스레드를 join()을 통해 메인 스레드와 동기화합니다.
	// 이를 통해 각각의 스레드가 작업을 완료한 후 메인 스레드가 진행되도록 합니다.
	for (int32 i = 0; i < 10; i++)
	{
		if (v[i].joinable())
			v[i].join();
	}

	// 모든 스레드가 종료된 후 메인 스레드에서 "Hello Main"을 출력합니다.
	cout << "Hello Main" << '\n';
}