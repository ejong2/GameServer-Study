#pragma once

/*-------------------
	BaseAllocator
-------------------*/

// �⺻ �Ҵ��� Ŭ���� ����
class BaseAllocator
{
public:
	// �޸𸮸� �Ҵ��ϴ� �޼���
	static void*	Alloc(int32 size);
	// �޸𸮸� �����ϴ� �޼���
	static void		Release(void* ptr);
};