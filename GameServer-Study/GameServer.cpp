#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include "AccountManager.h"
#include "UserManager.h"

// 첫 번째 스레드 함수: Fun1
// UserManager의 ProcessSave 함수를 한 번 호출합니다.
void Fun1()
{
	for (int32 i = 0; i < 1; i++)
	{
		UserManager::Instance()->ProcessSave();
	}
}

// 두 번째 스레드 함수: Fun2
// AccountManager의 ProcessLogin 함수를 한 번 호출합니다.
void Fun2()
{
	for (int32 i = 0; i < 1; i++)
	{
		AccountManager::Instance()->ProcessLogin();
	}
}

int main()
{
	// Fun1과 Fun2 함수를 각각 스레드 t1과 t2에서 동시에 실행합니다.
	std::thread t1(Fun1);
	std::thread t2(Fun2);

	// 두 개의 스레드를 join()을 통해 메인 스레드와 동기화하여 각각의 스레드가 작업을 완료한 후 메인 스레드가 진행되도록 합니다.
	t1.join();
	t2.join();

	cout << "Jobs Done" << '\n';

	// 참고
	mutex m1;
	mutex m2;

	// 두 개의 뮤텍스를 동시에 잠급니다.
	std::lock(m1, m2); // m1.lock(); m2.lcok();

	// adopt_lock : 이미 lock된 상태니까, 나중에 소멸될 때 풀어준다.
	lock_guard<mutex> g1(m1, std::adopt_lock);
	lock_guard<mutex> g2(m2, std::adopt_lock);
}

/* 이 코드는 데드락에 대한 예제입니다.UserManager와 AccountManager에서 lock_guard를 사용하여 각각의 뮤텍스를 잠그고 있습니다.여기서 두 스레드가 동시에 실행되어 서로 다른 뮤텍스를 잠그고, 상대방이 잠긴 뮤텍스를 기다리게 되면 데드락이 발생합니다.이를 해결하기 위해 std::lock 함수를 사용하여 두 개의 뮤텍스를 동시에 잠글 수 있고, std::adopt_lock을 사용하여 이미 lock된 뮤텍스를 lock_guard에 전달할 수 있습니다.이렇게 함으로써 데드락을 방지할 수 있습니다.*/