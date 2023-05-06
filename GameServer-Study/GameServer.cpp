/* 이 코드는 뮤텍스를 사용하여 공유 데이터를 관리하는 방법을 보여주는 예제입니다.
뮤텍스(mutex)는 상호배제(mutual exclusion) 원칙을 따르며, 이를 통해 멀티스레드 환경에서 공유 데이터를 안전하게 처리할 수 있습니다.
RAII(Resource Acquisition Is Initialization) 기술을 사용하여 LockGuard 클래스를 정의하고, 이를 사용하여 Lock을 잠그고 해제합니다.
이를 통해 뮤텍스 잠금 및 해제에 대한 예외 처리 및 메모리 누수와 같은 문제를 방지할 수 있습니다.
*/

#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>

// [1][2][3][4][][][][][][][][][][][]
vector<int32> v;

// Mutual Exclusive (상호베타적)
mutex m;

// RAII (Resources Acquisition Is Initialization)
template<typename T>
class LockGuard
{
public:
	LockGuard(T& m)
	{
		_mutex = &m;
		_mutex->lock();
	}

	~LockGuard()
	{
		_mutex->unlock();
	}
private:
	T* _mutex;
};

// Push 함수 : Lock을 걸고, 벡터에 값을 추가하는 함수
void Push()
{
	for (int32 i = 0; i < 10000; i++)
	{
		// 자물쇠 잠그기
		//LockGuard<std::mutex> LockGuard(m);

		std::lock_guard<std::mutex> LockGuard(m);

		//std::unique_lock<std::mutex> uniqueLock(m, std::defer_lock);
		//uniqueLock.lock();

		//m.lock();
		//m.lock();

		v.push_back(i);

		if (i == 5000)
		{
			//m.unlock();
			break;
		}

		// 자물쇠 풀기
		//m.unlock();
		//m.unlock();
	}
}

int main()
{
	// Push 함수를 스레드 t1과 t2에서 동시에 실행합니다.
	std::thread t1(Push);
	std::thread t2(Push);

	// 두 개의 스레드를 join()을 통해 메인 스레드와 동기화하여 각각의 스레드가 작업을 완료한 후 메인 스레드가 진행되도록 합니다.
		t1.join();
	t2.join();

	// 최종적으로 계산된 벡터의 크기를 출력합니다.
	cout << v.size() << '\n';

	return 0;
}