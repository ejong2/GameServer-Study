#pragma once
#include "Allocator.h"

class MemoryPool;

/*-------------
	Memory
---------------*/

class Memory
{
	enum
	{
		// ~1024까지 32단위, ~2048까지 128단위, ~4096까지 256단위
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256), // 메모리 풀의 개수를 계산하는 상수입니다.
		MAX_ALLOC_SIZE = 4096 // 할당할 수 있는 최대 메모리 크기를 정의하는 상수입니다.
	};

public:
	Memory(); // 메모리 클래스의 생성자입니다. 여러 크기의 메모리 풀을 생성합니다.
	~Memory(); // 메모리 클래스의 소멸자입니다. 모든 메모리 풀을 삭제하고 메모리를 해제합니다.

	void* Allocate(int32 size); // 지정된 크기의 메모리를 할당하는 함수입니다. 필요에 따라 메모리 풀에서 꺼내오거나, 일반적인 방식으로 메모리를 할당합니다.
	void Release(void* ptr); // 지정된 메모리를 해제하는 함수입니다. 메모리 풀로 반환하거나, 일반적인 방식으로 메모리를 해제합니다.

private:
	vector<MemoryPool*> _pools; // 다양한 크기의 메모리 블록을 관리하는 메모리 풀을 저장하는 벡터입니다.

	// 메모리 크기 <-> 메모리 풀
	// O(1) 빠르게 찾기 위한 테이블
	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1]; // 메모리 크기에 따른 메모리 풀을 빠르게 찾기 위한 테이블입니다.
};


template<typename Type, typename... Args>
Type* xnew(Args&&... args)
{
	Type* memory = static_cast<Type*>(xalloc(sizeof(Type)));
	new(memory)Type(forward<Args>(args)...); // placement new
	return memory;
}

template<typename Type>
void xdelete(Type* obj)
{
	obj->~Type();
	xrelease(obj);
}