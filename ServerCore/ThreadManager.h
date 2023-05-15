#pragma once

#include <thread>
#include <functional>

/*--------------------
    ThreadManager
---------------------*/

class ThreadManager
{
public:
    ThreadManager(); // ������
    ~ThreadManager(); // �Ҹ���


    // �����带 �����ϰ� �����ϴ� �Լ��Դϴ�.
    void Launch(function<void(void)> callback);
    // ��� �����带 join�ϴ� �Լ��Դϴ�.
    void Join();

    // Thread Local Storage(TLS)�� �ʱ�ȭ�ϴ� �Լ��Դϴ�.
    static void InitTLS();
    // TLS�� �ı��ϴ� �Լ��Դϴ�.
    static void DestroyTLS();
private:
    Mutex _lock; // ������ ������ ���� ���ؽ�
    vector<thread> _threads; // ��������� �����ϴ� ����
};