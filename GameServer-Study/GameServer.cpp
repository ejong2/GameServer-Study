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

// 16����Ʈ ���ĵ� Data Ŭ������ �����մϴ�.
// �� Ŭ������ SListEntry�� ����� ���� �ְ�, ������ 64��Ʈ ������ �����մϴ�.

SListHeader* GHeader;

int main()
{
	GHeader = new SListHeader();
	ASSERT_CRASH(((uint64)GHeader % 16) == 0);
	InitializeHead(GHeader);

// main �Լ������� GHeader�� �����ϰ� �ʱ�ȭ�ϸ�, �̸� 16����Ʈ�� �����մϴ�.

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

// �� �ڵ�� GThreadManager�� ����Ͽ� 3���� ���� �����带 �����ϰ�, 
// �� ������� �� Data ��ü�� �����Ͽ� GHeader�� PushEntrySList �Լ��� ����Ͽ� �߰��մϴ�.

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

// ����, GThreadManager�� ���� 2���� ���� �����带 �����Ͽ� 
// GHeader���� PopEntrySList �Լ��� ����Ͽ� �����͸� �����ϰ� ����մϴ�.

	GThreadManager->Join();
}