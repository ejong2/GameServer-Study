#include "pch.h"
#include "Allocator.h"

/*-------------------
	BaseAllocator
-------------------*/

// size��ŭ �޸𸮸� �Ҵ��ϰ�, �� ���� �ּҸ� ��ȯ�մϴ�.
//void* BaseAllocator::Alloc(int32 size)
//{
//	return ::malloc(size);
//}
//
//// ptr�� ����Ű�� �޸𸮸� �����մϴ�.
//void BaseAllocator::Release(void* ptr)
//{
//	::free(ptr);
//}

/*-------------------
   StompAllocator
-------------------*/

void* StompAllocator::Alloc(int32 size)
{
	const int64 pageCount = (size + PAGE_SIZE - 1) / PAGE_SIZE; // �ʿ��� ������ ���� ����մϴ�.
	const int64 dataOffset = pageCount * PAGE_SIZE - size; // ������ �������� ����մϴ�.

	void* baseAddress = ::VirtualAlloc(NULL, pageCount * PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE); // ������ ũ�⸸ŭ�� �޸𸮸� �Ҵ��մϴ�.
	return static_cast<void*>(static_cast<int8*>(baseAddress) + dataOffset); // �Ҵ�� �޸��� ���� �ּҸ� ��ȯ�մϴ�.
}

void StompAllocator::Release(void* ptr)
{
	const int64 address = reinterpret_cast<int64>(ptr); // �޸� �ּҸ� ���������� ��ȯ�մϴ�.
	const int64 baseAddress = address - (address % PAGE_SIZE); // �������� ���� �ּҸ� ����մϴ�.
	::VirtualFree(reinterpret_cast<void*>(baseAddress), 0, MEM_RELEASE); // �ش� �������� �����մϴ�.
}

/*
StompAllocator�� Release �޼���� �Ҵ�� �޸𸮸� �����մϴ�.
���⼭�� �־��� �����ͷκ��� �������� ���� �ּҸ� ����� ��,
�� �ּҷκ����� �޸𸮸� �����մϴ�. �̸� ���� ������ ������ �޸𸮸� �����ϴ� ���� �����մϴ�.
*/

/*
StompAllocator�� �޸𸮸� ������ ������ �Ҵ��ϰ� �����ϴ� ����� �����մϴ�.
�̴� Ư�� ���������� �߻��ϴ� ������ ���� �߰��� �� �ֵ��� �����ϴ�. 
���� ���, �޸� �Ҵ��� ��踦 �Ѿ�� ����(���� �����÷ο�) ���� ������ ������ ã�Ƴ� �� �ֽ��ϴ�.
�̷��� ������ ������ �޸𸮸� �����ϸ�,
�޸� ����� ȿ������ �ټ� ������ �� ������, ��� ���׸� �� ���� ã�Ƴ� �� �ִ� ������ �ֽ��ϴ�.
*/