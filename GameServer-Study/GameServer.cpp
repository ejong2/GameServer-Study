#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include "RefCounting.h" // 참조 카운팅과 스마트 포인터 코드 포함

// Wraight 클래스 정의. RefCountable을 상속받아 참조 카운팅을 사용함.
class Wraight : public RefCountable
{
public:
    int _hp = 150; // 체력 값
    int _posX = 0; // x 위치
    int _posY = 0; // y 위치
};

// WraightRef라는 이름으로 Wraight 객체에 대한 스마트 포인터 타입 정의
using WraightRef = TSharedPtr<Wraight>;

// Missile 클래스 정의. RefCountable을 상속받아 참조 카운팅을 사용함.
class Missile : public RefCountable
{
public:
    void SetTarget(WraightRef target) // 타겟 설정 함수
    {
        _target = target;
    }


    bool Update()  // 미사일 업데이트 함수
    {
        if (_target == nullptr)  // 타겟이 없다면 종료
            return true;

        int posX = _target->_posX;  // 타겟의 x 위치
        int posY = _target->_posY;  // 타겟의 y 위치

        // TODO : 쫓아간다

        if (_target->_hp == 0)  // 타겟의 체력이 0이라면 타겟을 없애고 종료
        {
            _target = nullptr;
            return true;
        }

        return false;  // 미사일이 아직 타겟을 추적 중
    }

    WraightRef _target = nullptr;  // 타겟에 대한 스마트 포인터
};

// MissileRef라는 이름으로 Missile 객체에 대한 스마트 포인터 타입 정의
using MissileRef = TSharedPtr<Missile>;

int main()
{
    WraightRef wraight(new Wraight()); // Wraight 객체 생성
    wraight->ReleaseRef(); // Wraight 객체에 대한 참조 해제
    MissileRef missile(new Missile()); // Missile 객체 생성
    missile->ReleaseRef(); // Missile 객체에 대한 참조 해제


    missile->SetTarget(wraight);  // 미사일의 타겟을 Wraight 객체로 설정

    // 레이스가 피격 당함
    wraight->_hp = 0;  // Wraight 객체의 체력을 0으로 설정
    wraight = nullptr;  // Wraight 스마트 포인터를 nullptr로 설정

    while (true)  // 무한 루프
    {
        if (missile)
        {
            if (missile->Update())  // 미사일 업데이트, 종료 조건이 만족되면 미사일 참조를 nullptr로 설정
            {
                missile = nullptr;
            }
        }
    }

    // 미사일 참조를 nullptr로 설정
    missile = nullptr;

    // 프로그램 종료
    return 0;
}

/*------------------------------------------------------------------------------
위의 코드는 'Wraight'와 'Missile'이라는 두 개의 클래스를 생성하고,
각 클래스는 'RefCountable'을 상속받아 참조 카운팅 기능을 활용합니다.
'TSharedPtr'은 스마트 포인터로서, 생성한 객체에 대한 참조를 관리합니다.
'Wraight' 객체를 타겟으로 하는 'Missile' 객체가 생성되며,
무한 루프 내에서 미사일의 'Update' 함수가 호출됩니다.
이 함수 내에서 타겟의 체력이 0이 되면 미사일의 참조를 해제하고 루프를 종료합니다.
--------------------------------------------------------------------------------*/