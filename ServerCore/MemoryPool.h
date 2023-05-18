#pragma once

enum
{
    SLIST_ALIGNMENT = 16
};

/*-----------------
    MemoryHeader
------------------*/

DECLSPEC_ALIGN(SLIST_ALIGNMENT)
struct MemoryHeader : public SLIST_ENTRY
{
    // [MemoryHeader][Data]
    MemoryHeader(int32 size) : allocSize(size) {} // �������Դϴ�. �Ҵ�� �޸� ũ�⸦ ���ڷ� �޽��ϴ�.

    static void* AttachHeader(MemoryHeader* header, int32 size)
    {
        new(header)MemoryHeader(size); // placement new�� ����� �޸� ����� �Ҵ���� �޸𸮿� �����մϴ�.
        return reinterpret_cast<void*>(++header); // ��� ���� �޸� �ּҸ� ��ȯ�մϴ�.
    }

    static MemoryHeader* DetachHeader(void* ptr)
    {
        MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1; // ����� �ּҸ� ���մϴ�.
        return header; // ����� �ּҸ� ��ȯ�մϴ�.
    }

    int32 allocSize; // �� �޸� ��Ͽ� �Ҵ�� �������� ũ�⸦ �����մϴ�.
    // TODO : �ʿ��� �߰� ����
};

/*-----------------
    MemoryPool
------------------*/

class MemoryPool
{
public:
    MemoryPool(int32 allocSize); // MemoryPool�� �������Դϴ�. �Ҵ��Ϸ��� �޸��� ũ�⸦ ���ڷ� �޽��ϴ�.
    ~MemoryPool(); // �Ҹ����Դϴ�. �޸� Ǯ�� �����ִ� ��� �޸𸮸� �����մϴ�.

    void			Push(MemoryHeader* ptr); // �޸� Ǯ�� �޸𸮸� ��ȯ�ϴ� �Լ��Դϴ�.
    MemoryHeader* Pop(); // �޸� Ǯ���� �޸𸮸� ������ �Լ��Դϴ�.

private:
    SLIST_HEADER    _header;
    int32           _allocSize = 0; // �޸� Ǯ���� �����ϴ� �޸� ����� ũ���Դϴ�.
    atomic<int32>   _allocCount = 0; // ���� �Ҵ�� �޸� ����� �����Դϴ�.

};

