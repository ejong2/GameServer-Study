#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include "RefCounting.h"
#include "Memory.h"
#include "Allocator.h"
#include "LockFreeStack.h"

DECLSPEC_ALIGN(16)
class Data // : public SListEntry
{
public:
	SListEntry _entry;
	int64 _rand = rand() % 1000;
};

// 16바이트 정렬된 Data 클래스를 선언합니다.
// 이 클래스는 SListEntry를 멤버로 갖고 있고, 임의의 64비트 정수를 포함합니다.

SListHeader* GHeader;

int main()
{
	GHeader = new SListHeader();
	ASSERT_CRASH(((uint64)GHeader % 16) == 0);
	InitializeHead(GHeader);

// main 함수에서는 GHeader를 생성하고 초기화하며, 이를 16바이트로 정렬합니다.

	for (int32 i = 0; i < 3; i++)
	{
		GThreadManager->Launch([]()
			{
				while (true)
				{
					Data* data = new Data();
					ASSERT_CRASH(((uint64)data % 16) == 0);

					PushEntrySList(GHeader, (SListEntry*)data);
					this_thread::sleep_for(10ms);
				}
			});
	}

// 이 코드는 GThreadManager를 사용하여 3개의 별도 스레드를 생성하고, 
// 각 스레드는 새 Data 객체를 생성하여 GHeader에 PushEntrySList 함수를 사용하여 추가합니다.

	for (int32 i = 0; i < 2; i++)
	{
		GThreadManager->Launch([]()
			{
				while (true)
				{
					Data* pop = nullptr;
					pop = (Data*)PopEntrySList(GHeader);

					if (pop)
					{
						cout << pop->_rand << endl;
						delete pop;
					}
					else
					{
						cout << "NONE" << endl;
					}
				}
			});
	}

// 또한, GThreadManager는 또한 2개의 별도 스레드를 생성하여 
// GHeader에서 PopEntrySList 함수를 사용하여 데이터를 제거하고 출력합니다.

	GThreadManager->Join();
}