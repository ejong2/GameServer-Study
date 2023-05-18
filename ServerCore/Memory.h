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
		// ~1024���� 32����, ~2048���� 128����, ~4096���� 256����
		POOL_COUNT = (1024 / 32) + (1024 / 128) + (2048 / 256), // �޸� Ǯ�� ������ ����ϴ� ����Դϴ�.
		MAX_ALLOC_SIZE = 4096 // �Ҵ��� �� �ִ� �ִ� �޸� ũ�⸦ �����ϴ� ����Դϴ�.
	};

public:
	Memory(); // �޸� Ŭ������ �������Դϴ�. ���� ũ���� �޸� Ǯ�� �����մϴ�.
	~Memory(); // �޸� Ŭ������ �Ҹ����Դϴ�. ��� �޸� Ǯ�� �����ϰ� �޸𸮸� �����մϴ�.

	void* Allocate(int32 size); // ������ ũ���� �޸𸮸� �Ҵ��ϴ� �Լ��Դϴ�. �ʿ信 ���� �޸� Ǯ���� �������ų�, �Ϲ����� ������� �޸𸮸� �Ҵ��մϴ�.
	void Release(void* ptr); // ������ �޸𸮸� �����ϴ� �Լ��Դϴ�. �޸� Ǯ�� ��ȯ�ϰų�, �Ϲ����� ������� �޸𸮸� �����մϴ�.

private:
	vector<MemoryPool*> _pools; // �پ��� ũ���� �޸� ����� �����ϴ� �޸� Ǯ�� �����ϴ� �����Դϴ�.

	// �޸� ũ�� <-> �޸� Ǯ
	// O(1) ������ ã�� ���� ���̺�
	MemoryPool* _poolTable[MAX_ALLOC_SIZE + 1]; // �޸� ũ�⿡ ���� �޸� Ǯ�� ������ ã�� ���� ���̺��Դϴ�.
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