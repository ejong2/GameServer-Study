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

// 참조 카운팅을 사용하는 Knight 클래스 정의
class Knight : public RefCountable
{
public:
    // 생성자에서 메시지 출력
    Knight()
    {
        cout << "Kngiht()" << endl;
    }

    // 소멸자에서 메시지 출력
    ~Knight()
    {
        cout << "~Knight()" << endl;
    }
};

int main()
{
    // 1) 이미 만들어진 클래스 대상으로 사용 불가  
    // 2) 순환 (Cycle) 문제

    /*
    unique_ptr<Knight> k2 = make_unique<Knight>();
    unique_ptr<Knight> k3 = std::move(k2);

    -> unique_ptr : 복사 X
    */

    // shared_ptr와 weak_ptr 사용 예제

    // [Knight | RefCountingBlock(uses, weak)]

    // [T*][RefCountBlocking*]
    //shared_ptr<Knight> spr(new Knight());

    // Knight 객체를 가리키는 shared_ptr 생성
    // 이때 RefCountBlock에서 useCount(shared)와 weakCount가 함께 증가합니다.
    shared_ptr<Knight> spr = make_shared<Knight>();

    // spr이 가리키는 객체에 대한 weak_ptr 생성
    // weak_ptr은 소유하지 않지만, 객체의 생존 여부를 알 수 있습니다.
    weak_ptr<Knight> wpr = spr;

    // wpr이 가리키는 객체가 소멸했는지 확인
    // 소멸했다면 true, 아니라면 false를 반환합니다.
    bool expired = wpr.expired();

    // wpr이 가리키는 객체에 대한 shared_ptr을 생성
    // 만약 해당 객체가 이미 소멸되었다면, nullptr을 반환합니다.
    shared_ptr<Knight> spr2 = wpr.lock();

    // spr2가 nullptr이 아닌지 확인
    // nullptr이 아니라면, wpr이 가리키는 객체는 아직 살아있습니다.
    if (spr2 != nullptr)
    {

    }
}

/*-------------------------------------------------------------------------------------------
위의 코드는 shared_ptr와 weak_ptr의 사용 예제를 보여줍니다.
shared_ptr은 객체에 대한 공유 소유권을 제공하며, 참조 카운팅을 통해 객체의 수명을 관리합니다.
weak_ptr은 객체에 대한 약한 참조를 제공하며,
객체의 수명에 영향을 주지 않지만 해당 객체가 아직 존재하는지 확인할 수 있습니다.
이를 통해 순환 참조 문제를 해결할 수 있습니다.
--------------------------------------------------------------------------------------------*/