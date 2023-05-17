#include "pch.h"
#include "Allocator.h"

/*-------------------
	BaseAllocator
-------------------*/

// size만큼 메모리를 할당하고, 그 시작 주소를 반환합니다.
void* BaseAllocator::Alloc(int32 size)
{
	return ::malloc(size);
}

// ptr이 가리키는 메모리를 해제합니다.
void BaseAllocator::Release(void* ptr)
{
	::free(ptr);
}