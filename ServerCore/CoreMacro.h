#pragma once

#define OUT

/*---------------------
          Lock
----------------------*/

// Lock 객체를 여러 개 생성하는 매크로. count 개수 만큼 Lock 객체를 생성합니다.
#define USE_MANY_LOCKS(count)   Lock _locks[count];  

// Lock 객체를 하나 생성하는 매크로
#define USE_LOCK                USE_MANY_LOCKS(1);  

// ReadLockGuard 객체를 생성하고, 생성자에서 읽기 잠금을 획득하는 매크로
// idx는 Lock 객체 배열에서 읽기 잠금을 획득할 Lock 객체의 인덱스입니다.
#define READ_LOCK_IDX(idx)      ReadLockGuard readLockGuard_##idx(_locks[idx], typeid(this).name()); 

// 첫 번째 Lock 객체에 대해 읽기 잠금을 획득하는 매크로
#define READ_LOCK               READ_LOCK_IDX(0)

// WriteLockGuard 객체를 생성하고, 생성자에서 쓰기 잠금을 획득하는 매크로
// idx는 Lock 객체 배열에서 쓰기 잠금을 획득할 Lock 객체의 인덱스입니다.
#define WRITE_LOCK_IDX(idx)     WriteLockGuard writeLockGuard_##idx(_locks[idx], typeid(this).name());

// 첫 번째 Lock 객체에 대해 쓰기 잠금을 획득하는 매크로
#define WRITE_LOCK              WRITE_LOCK_IDX(0)


/*----------------
      Memory
-----------------*/

//#ifdef _DEBUG
//#define xalloc(size)		BaseAllocator::Alloc(size)
//#define xrelease(ptr)		BaseAllocator::Release(ptr)
//#else
//#define xalloc(size)		BaseAllocator::Alloc(size)
//#define xrelease(ptr)		BaseAllocator::Release(ptr)
//#endif

/*---------------------
         CRASH
----------------------*/

// CRASH 매크로는 오류가 발생했을 때 프로그램을 중단시킵니다.
#define CRASH(cause)                        \
{                                           \
    uint32* crash = nullptr;                \
    __analysis_assume(crash != nullptr);    \
    *crash = 0xDEADBEEF;                    \
}

// ASSERT_CRASH 매크로는 주어진 표현식이 false일 경우 프로그램을 중단시킵니다.
#define ASSERT_CRASH(expr)                  \
{                                           \
    if (!(expr))                            \
    {                                       \
        CRASH("ASSERT_CRASH");              \
        __analysis_assume(expr);            \
    }                                       \
}