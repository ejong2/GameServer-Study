#include "pch.h"
#include "UserManager.h"
#include "AccountManager.h"

// 사용자 데이터 저장 함수 구현
void UserManager::ProcessSave()
{
	// AccountManager 인스턴스로부터 특정 계정을 가져옵니다.
	Account* account = AccountManager::Instance()->GetAccount(100);

	// userLock을 걸어줍니다.
	lock_guard<mutex> guard(_mutex);

	//Account* account = AccountManager::Instance()->GetAccount(100); -> 데드락 발생 (데드락 순서X)

	// TODO
}