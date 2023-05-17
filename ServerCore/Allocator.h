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
    static void*    Alloc(int32 size); // ���� �޼ҵ��, ũ�⸦ �Ű������� �޾� ���� �޸𸮸� �Ҵ��մϴ�.
    static void	    Release(void* ptr); // ���� �޼ҵ��, �Ҵ�� �޸𸮸� �����ϴ� ������ �մϴ�.
};

/*
StompAllocator Ŭ������ BaseAllocator Ŭ������ �����ϰ� �޸𸮸� �Ҵ��ϰ� �����ϴ� �޼ҵ带 ������ �ֽ��ϴ�.
�ٸ�, �� Ŭ������ ������ ũ�� ������ �޸𸮸� �����մϴ�.
*/

/*-------------------
    STL Allocator
-------------------*/

template<typename T>
class StlAllocator
{
public:
    using value_type = T; // Allocator�� �����ϴ� �������� Ÿ���� �����մϴ�.

    StlAllocator() {} // �⺻ �����ڸ� �����մϴ�.

    template<typename Other>
    StlAllocator(const StlAllocator<Other>&) {} // �ٸ� Ÿ���� StlAllocator���� ���� Ÿ���� StlAllocator�� ������ �� �ֵ��� ���� �����ڸ� �����մϴ�.

    T* allocate(size_t count) // count ������ŭ�� T Ÿ�� ��ü�� �Ҵ��ϴ� �Լ��� �����մϴ�.
    {
        const int32 size = static_cast<int32>(count * sizeof(T)); // �Ҵ��� �޸��� ũ�⸦ ����մϴ�.
        return static_cast<T*>(StompAllocator::Alloc(size)); // StompAllocator�� ����� �޸𸮸� �Ҵ��ϰ�, �� �ּҸ� ��ȯ�մϴ�.
    }

    void deallocate(T* ptr, size_t count) // �Ҵ�� �޸𸮸� �����ϴ� �Լ��� �����մϴ�.
    {
        StompAllocator::Release(ptr); // StompAllocator�� ����� �޸𸮸� �����մϴ�.
    }
};

/*
StlAllocator�� STL �����̳ʵ��� ����ϴ� Allocator�� �������̽��� ������ ����� ���� Allocator Ŭ�����Դϴ�.
�� Ŭ������ �����̳ʰ� �޸𸮸� �Ҵ��ϰ� �����ϴ� ����� �����մϴ�.
���⼭�� StompAllocator�� ����� �޸𸮸� �Ҵ��ϰ� �����ϵ��� ���ǵǾ� �ֽ��ϴ�.
�̸� ���� STL �����̳ʵ��� �޸𸮸� �Ҵ��ϰ� �����ϴ� ����� �����ڰ� ���ϴ� ������� ������ �� �ֽ��ϴ�.
*/