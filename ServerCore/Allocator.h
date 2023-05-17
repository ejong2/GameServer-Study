#pragma once

/*-------------------
    BaseAllocator
-------------------*/

// 기본 할당자 클래스 정의
class BaseAllocator
{
public:
    // 메모리를 할당하는 메서드
    static void*    Alloc(int32 size);
    // 메모리를 해제하는 메서드
    static void		Release(void* ptr);
};

/*-------------------
   StompAllocator
-------------------*/

class StompAllocator
{
    enum { PAGE_SIZE = 0x1000 }; // 페이지 크기를 나타내는 상수를 정의합니다.
public:
    static void*    Alloc(int32 size); // 정적 메소드로, 크기를 매개변수로 받아 동적 메모리를 할당합니다.
    static void	    Release(void* ptr); // 정적 메소드로, 할당된 메모리를 해제하는 역할을 합니다.
};

/*
StompAllocator 클래스는 BaseAllocator 클래스와 유사하게 메모리를 할당하고 해제하는 메소드를 가지고 있습니다.
다만, 이 클래스는 페이지 크기 단위로 메모리를 관리합니다.
*/

/*-------------------
    STL Allocator
-------------------*/

template<typename T>
class StlAllocator
{
public:
    using value_type = T; // Allocator가 관리하는 데이터의 타입을 정의합니다.

    StlAllocator() {} // 기본 생성자를 정의합니다.

    template<typename Other>
    StlAllocator(const StlAllocator<Other>&) {} // 다른 타입의 StlAllocator에서 현재 타입의 StlAllocator를 생성할 수 있도록 복사 생성자를 정의합니다.

    T* allocate(size_t count) // count 개수만큼의 T 타입 객체를 할당하는 함수를 정의합니다.
    {
        const int32 size = static_cast<int32>(count * sizeof(T)); // 할당할 메모리의 크기를 계산합니다.
        return static_cast<T*>(StompAllocator::Alloc(size)); // StompAllocator를 사용해 메모리를 할당하고, 그 주소를 반환합니다.
    }

    void deallocate(T* ptr, size_t count) // 할당된 메모리를 해제하는 함수를 정의합니다.
    {
        StompAllocator::Release(ptr); // StompAllocator를 사용해 메모리를 해제합니다.
    }
};

/*
StlAllocator는 STL 컨테이너들이 사용하는 Allocator의 인터페이스를 구현한 사용자 정의 Allocator 클래스입니다.
이 클래스는 컨테이너가 메모리를 할당하고 해제하는 방식을 정의합니다.
여기서는 StompAllocator를 사용해 메모리를 할당하고 해제하도록 정의되어 있습니다.
이를 통해 STL 컨테이너들이 메모리를 할당하고 해제하는 방식을 개발자가 원하는 방식으로 변경할 수 있습니다.
*/