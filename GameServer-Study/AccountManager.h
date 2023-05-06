#pragma once
#include <mutex>

class Account
{
	// TODO
};

class AccountManager
{
public:
	// 싱글턴 패턴을 사용하여 AccountManager 인스턴스를 가져옵니다.
	static AccountManager* Instance()
	{
		static AccountManager instance;
		return &instance;
	}


	// 계정을 가져오는 함수
	Account* GetAccount(int32 id)
	{
		// 뮤텍스로부터 가드를 생성하여 스레드 안전성을 보장합니다.
		lock_guard<mutex> guard(_mutex);

		return nullptr;
	}
	// 로그인 처리를 하는 함수
	void ProcessLogin();
private:
	// 스레드 간 경쟁 조건을 방지하기 위한 뮤텍스
	mutex _mutex;
};