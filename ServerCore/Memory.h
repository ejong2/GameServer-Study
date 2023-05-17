#pragma once
#include "Allocator.h"

// Type�� ��ü�� �����ϰ�, �� ��ü�� ���� �����͸� ��ȯ�ϴ� �Լ� ���ø�
template<typename Type, typename...Args>
Type* xnew(Args&&... args)
{
    // Type�� ��ü�� ������ �� �ִ� �޸𸮸� �Ҵ��մϴ�.
    Type* memory = static_cast<Type*>(BaseAllocator::Alloc(sizeof(Type)));

    // placement new�� �̿��Ͽ� �޸𸮿� Type�� ��ü�� �����մϴ�.
    new(memory)Type(forward<Args>(args)...);
    return memory;
}

// obj�� ����Ű�� ��ü�� �Ҹ��Ű��, �� ��ü�� �޸𸮸� �����ϴ� �Լ� ���ø�
template<typename Type>
void xdelete(Type* obj)
{
    // obj�� ����Ű�� ��ü�� �Ҹ��ڸ� ȣ���մϴ�.
    obj->~Type();
    // ��ü�� �޸𸮸� �����մϴ�.
    BaseAllocator::Release(obj);
}