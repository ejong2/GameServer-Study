#include "pch.h"
#include "Allocator.h"

/*-------------------
	BaseAllocator
-------------------*/

// size��ŭ �޸𸮸� �Ҵ��ϰ�, �� ���� �ּҸ� ��ȯ�մϴ�.
void* BaseAllocator::Alloc(int32 size)
{
	return ::malloc(size);
}

// ptr�� ����Ű�� �޸𸮸� �����մϴ�.
void BaseAllocator::Release(void* ptr)
{
	::free(ptr);
}