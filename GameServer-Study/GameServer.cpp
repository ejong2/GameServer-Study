#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <atomic>
#include <mutex>
#include <windows.h>
#include <future>
#include "ThreadManager.h"

#include "RefCounting.h"
#include "Memory.h"
#include "Allocator.h"

class Player
{
public:
	Player() {}
	virtual ~Player() {}
};

class Knight : public Player
{
public:
	Knight()
	{
		cout << "Knight()" << endl;
	}

	Knight(int32 hp) : _hp(hp)
	{
		cout << "Knight(hp)" << endl;
	}

	~Knight()
	{
		cout << "~Knight()" << endl;
	}

	int32 _hp = 100;
	int32 _mp = 10;
};

int main()
{
	Vector<Knight> v(100); // 사용자 정의 Allocator를 사용하는 Vector를 생성하고, Knight 객체 100개를 저장하는 공간을 할당합니다.

	Map<int32, Knight> m; // 사용자 정의 Allocator를 사용하는 Map을 생성합니다.
	m[100] = Knight(); // Map에 키 100에 대응하는 Knight 객체를 저장합니다.
}