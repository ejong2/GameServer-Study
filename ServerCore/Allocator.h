#pragma once

/*-------------------
    BaseAllocator
-------------------*/

// �⺻ �Ҵ��� Ŭ���� ����
class BaseAllocator
{
public:
    // �޸𸮸� �Ҵ��ϴ� �޼���
    static void*    Alloc(int32 size);
    // �޸𸮸� �����ϴ� �޼���
    static void		Release(void* ptr);
};

/*-------------------
   StompAllocator
-------------------*/

class StompAllocator
{
    enum { PAGE_SIZE = 0x1000 }; // ������ ũ�⸦ ��Ÿ���� ����� �����մϴ�.
public:
    static void* Alloc(int32 size); // ���� �޼ҵ��, ũ�⸦ �Ű������� �޾� ���� �޸𸮸� �Ҵ��մϴ�.
    static void	    Release(void* ptr); // ���� �޼ҵ��, �Ҵ�� �޸𸮸� �����ϴ� ������ �մϴ�.
};

/*
StompAllocator Ŭ������ BaseAllocator Ŭ������ �����ϰ� �޸𸮸� �Ҵ��ϰ� �����ϴ� �޼ҵ带 ������ �ֽ��ϴ�.
�ٸ�, �� Ŭ������ ������ ũ�� ������ �޸𸮸� �����մϴ�.
*/