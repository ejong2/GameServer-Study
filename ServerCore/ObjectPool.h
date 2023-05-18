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
    // shared_ptr 형태의 객체를 생성하는 함수입니다.
    {
        shared_ptr<Type> ptr = { Pop(), Push };
        // 메모리 풀에서 객체를 가져와 소멸자로 Push 함수를 지정한 shared_ptr을 생성합니다.
        return ptr;
        // 생성한 shared_ptr을 반환합니다.
    }

private:
    static int32		s_allocSize;
    // ObjectPool에서 관리하는 객체의 크기를 저장하는 정적 변수입니다.

    static MemoryPool	s_pool;
    // 메모리 풀을 저장하는 정적 변수입니다.
};

template<typename Type>
int32 ObjectPool<Type>::s_allocSize = sizeof(Type) + sizeof(MemoryHeader);
// 타입 Type의 객체와 메모리 헤더의 크기를 더해 s_allocSize를 초기화합니다.

template<typename Type>
MemoryPool ObjectPool<Type>::s_pool{ s_allocSize };
// s_allocSize를 사용해 메모리 풀을 초기화합니다.