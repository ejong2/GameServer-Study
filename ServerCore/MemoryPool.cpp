#include "pch.h"
#include "MemoryPool.h"

/*-----------------
	MemoryPool
------------------*/

// 메모리 풀의 생성자
MemoryPool::MemoryPool(int32 allocSize) : _allocSize(allocSize)
{
}

// 메모리 풀의 소멸자. 풀에 남아있는 모든 메모리를 해제한다.
MemoryPool::~MemoryPool()
{
	while (_queue.empty() == false)
	{
		MemoryHeader* header = _queue.front();
		_queue.pop();
		::free(header);
	}
}

// 메모리 풀에 메모리를 반환하는 함수
void MemoryPool::Push(MemoryHeader* ptr)
{
	WRITE_LOCK;
	ptr->allocSize = 0;

	// Pool에 메모리 반납
	_queue.push(ptr);

	_allocCount.fetch_sub(1);
}

// 메모리 풀에서 메모리를 가져오는 함수
MemoryHeader* MemoryPool::Pop()
{
	MemoryHeader* header = nullptr;

	{
		WRITE_LOCK;
		// Pool에 여분이 있는지?
		if (_queue.empty() == false)
		{
			// 있으면 하나 꺼내온다
			header = _queue.front();
			_queue.pop();
		}
	}

	// 없으면 새로 만들다
	if (header == nullptr)
	{
		header = reinterpret_cast<MemoryHeader*>(::malloc(_allocSize));
	}
	else
	{
		ASSERT_CRASH(header->allocSize == 0);
	}

	_allocCount.fetch_add(1);

	return header;
}