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
    static void* Alloc(int32 size); // 정적 메소드로, 크기를 매개변수로 받아 동적 메모리를 할당합니다.
    static void	    Release(void* ptr); // 정적 메소드로, 할당된 메모리를 해제하는 역할을 합니다.
};

/*
StompAllocator 클래스는 BaseAllocator 클래스와 유사하게 메모리를 할당하고 해제하는 메소드를 가지고 있습니다.
다만, 이 클래스는 페이지 크기 단위로 메모리를 관리합니다.
*/