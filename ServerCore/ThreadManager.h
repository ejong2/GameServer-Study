#pragma once

#include <thread>
#include <functional>

/*--------------------
    ThreadManager
---------------------*/

class ThreadManager
{
public:
    ThreadManager(); // 생성자
    ~ThreadManager(); // 소멸자


    // 쓰레드를 생성하고 실행하는 함수입니다.
    void Launch(function<void(void)> callback);
    // 모든 쓰레드를 join하는 함수입니다.
    void Join();

    // Thread Local Storage(TLS)를 초기화하는 함수입니다.
    static void InitTLS();
    // TLS를 파괴하는 함수입니다.
    static void DestroyTLS();
private:
    Mutex _lock; // 쓰레드 관리를 위한 뮤텍스
    vector<thread> _threads; // 쓰레드들을 저장하는 벡터
};