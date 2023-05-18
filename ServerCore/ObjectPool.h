#pragma once
#include "Types.h"
#include "MemoryPool.h"

template<typename Type>
class ObjectPool
{
public:
    template<typename... Args>
    static Type* Pop(Args&&... args)
    {
#ifdef _STOMP
        MemoryHeader* ptr = reinterpret_cast<MemoryHeader*>(StompAllocator::Alloc(s_allocSize));
        Type* memory = static_cast<Type*>(MemoryHeader::AttachHeader(ptr, s_allocSize));
#else
        Type* memory = static_cast<Type*>(MemoryHeader::AttachHeader(s_pool.Pop(), s_allocSize));
#endif		
        new(memory)Type(forward<Args>(args)...); // placement new
        return memory;
    }

    static void Push(Type* obj)
    {
        obj->~Type();
#ifdef _STOMP
        StompAllocator::Release(MemoryHeader::DetachHeader(obj));
#else
        s_pool.Push(MemoryHeader::DetachHeader(obj));
#endif
    }

    static shared_ptr<Type> MakeShared()
    // shared_ptr ������ ��ü�� �����ϴ� �Լ��Դϴ�.
    {
        shared_ptr<Type> ptr = { Pop(), Push };
        // �޸� Ǯ���� ��ü�� ������ �Ҹ��ڷ� Push �Լ��� ������ shared_ptr�� �����մϴ�.
        return ptr;
        // ������ shared_ptr�� ��ȯ�մϴ�.
    }

private:
    static int32		s_allocSize;
    // ObjectPool���� �����ϴ� ��ü�� ũ�⸦ �����ϴ� ���� �����Դϴ�.

    static MemoryPool	s_pool;
    // �޸� Ǯ�� �����ϴ� ���� �����Դϴ�.
};

template<typename Type>
int32 ObjectPool<Type>::s_allocSize = sizeof(Type) + sizeof(MemoryHeader);
// Ÿ�� Type�� ��ü�� �޸� ����� ũ�⸦ ���� s_allocSize�� �ʱ�ȭ�մϴ�.

template<typename Type>
MemoryPool ObjectPool<Type>::s_pool{ s_allocSize };
// s_allocSize�� ����� �޸� Ǯ�� �ʱ�ȭ�մϴ�.