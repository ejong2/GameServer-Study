#include "pch.h"
#include "Allocator.h"

/*-------------------
	BaseAllocator
-------------------*/

// size만큼 메모리를 할당하고, 그 시작 주소를 반환합니다.
//void* BaseAllocator::Alloc(int32 size)
//{
//	return ::malloc(size);
//}
//
//// ptr이 가리키는 메모리를 해제합니다.
//void BaseAllocator::Release(void* ptr)
//{
//	::free(ptr);
//}

/*-------------------
   StompAllocator
-------------------*/

void* StompAllocator::Alloc(int32 size)
{
	const int64 pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE; // 필요한 페이지 수를 계산합니다.
	const int64 dataOffset = pageCount * PAGE_SIZE - size; // 데이터 오프셋을 계산합니다.

	void* baseAddress = ::VirtualAlloc(NULL, pageCount * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE); // 페이지 크기만큼의 메모리를 할당합니다.
	return static_cast<void*>(static_cast<int8*>(baseAddress) + dataOffset); // 할당된 메모리의 시작 주소를 반환합니다.
}

void StompAllocator::Release(void* ptr)
{
	const int64 address = reinterpret_cast<int64>(ptr); // 메모리 주소를 정수형으로 변환합니다.
	const int64 baseAddress = address - (address % PAGE_SIZE); // 페이지의 시작 주소를 계산합니다.
	::VirtualFree(reinterpret_cast<void*>(baseAddress), 0, MEM_RELEASE); // 해당 페이지를 해제합니다.
}

/*
StompAllocator의 Release 메서드는 할당된 메모리를 해제합니다.
여기서는 주어진 포인터로부터 페이지의 시작 주소를 계산한 후,
그 주소로부터의 메모리를 해제합니다. 이를 통해 페이지 단위로 메모리를 관리하는 것이 가능합니다.
*/

/*
StompAllocator는 메모리를 페이지 단위로 할당하고 해제하는 방식을 제공합니다.
이는 특정 페이지에서 발생하는 오류를 쉽게 발견할 수 있도록 돕습니다. 
예를 들어, 메모리 할당의 경계를 넘어서는 접근(버퍼 오버플로우) 등의 문제를 빠르게 찾아낼 수 있습니다.
이렇게 페이지 단위로 메모리를 관리하면,
메모리 사용의 효율성은 다소 떨어질 수 있지만, 대신 버그를 더 쉽게 찾아낼 수 있는 장점이 있습니다.
*/