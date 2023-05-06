#pragma once
#include <mutex>

class User
{
	// TODO
};

class UserManager
{
public:
	// 싱글턴 패턴을 사용하여 UserManager 인스턴스를 가져옵니다.
	static UserManager* Instance()
	{
		static UserManager instance;
		return &instance;
	}

	// 사용자를 가져오는 함수
	User* GetUser(int32 id)
	{
		// 뮤텍스로부터 가드를 생성하여 스레드 안전성을 보장합니다.
		lock_guard<mutex> guard(_mutex);

		// 뭔가 갖고 옴
		return nullptr;
	}

	// 사용자 데이터를 저장하는 함수
	void ProcessSave();
private:
	// 스레드 간 경쟁 조건을 방지하기 위한 뮤텍스
	mutex _mutex;
};