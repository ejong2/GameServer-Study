#include "pch.h"
#include "UserManager.h"
#include "AccountManager.h"

// 사용자 데이터 저장 함수 구현
void AccountManager::ProcessLogin()
{
	// userLock을 걸어줍니다.
	lock_guard<mutex> guard(_mutex);

	// AccountManager 인스턴스로부터 특정 계정을 가져옵니다.
	User* user = UserManager::Instance()->GetUser(100);

	// TODO
}