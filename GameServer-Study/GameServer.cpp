#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include "RefCounting.h" // 참조 카운팅과 스마트 포인터 코드 포함
#include "Memory.h"

// 참조 카운팅을 사용하는 Knight 클래스 정의
class Knight : public RefCountable
{
public:
    // 생성자에서 메시지 출력
    Knight()
    {
        cout << "Kngiht()" << endl;
    }

    Knight(int32 hp) : _hp(hp)
    {
        cout << "Knight(hp)" << endl;
    }

    // 소멸자에서 메시지 출력
    ~Knight()
    {
        cout << "~Knight()" << endl;
    }

    /*static void* operator new(size_t size)
    {
        cout << "new! " << size << endl;
        void* ptr = ::malloc(size);
        return ptr;
    }

    static void operator delete(void* ptr)
    {
        cout << "delete! " << endl;
        ::free(ptr);
    }*/

    int32 _hp = 100;
    int32 _mp = 100;
};

void* operator new(size_t size)
{
    cout << "new! " << size << endl;
    void* ptr = ::malloc(size);
    return ptr;
}

void operator delete(void* ptr)
{
    cout << "delete! " << endl;
    ::free(ptr);
}

// new operator overloading ()Global
void* operator new[](size_t size)
{
    cout << "new[]! " << size << endl;
    void* ptr = ::malloc(size);
    return ptr;
}

void operator delete[](void* ptr)
{
    cout << "delete![]" << endl;
    ::free(ptr);
}

int main()
{
    Knight* knight = xnew<Knight>(100);

    xdelete(knight);
}