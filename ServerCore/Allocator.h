#pragma once

/*-------------------
	BaseAllocator
-------------------*/

// 기본 할당자 클래스 정의
class BaseAllocator
{
public:
	// 메모리를 할당하는 메서드
	static void*	Alloc(int32 size);
	// 메모리를 해제하는 메서드
	static void		Release(void* ptr);
};