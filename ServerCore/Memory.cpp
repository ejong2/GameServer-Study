#include "pch.h"
#include "Memory.h"
#include "MemoryPool.h"

/*-------------
    Memory
---------------*/

Memory::Memory()
{
    int32 size = 0;
    int32 tableIndex = 0;

    for (size = 32; size <= 1024; size += 32)
    {
        MemoryPool* pool = new MemoryPool(size);
        _pools.push_back(pool);

        while (tableIndex <= size)
        {
            _poolTable[tableIndex] = pool;
            tableIndex++;
        }
    }

    for (; size <= 2048; size += 128)
    {
        MemoryPool* pool = new MemoryPool(size);
        _pools.push_back(pool);

        while (tableIndex <= size)
        {
            _poolTable[tableIndex] = pool;
            tableIndex++;
        }
    }

    for (; size <= 4096; size += 256)
    {
        MemoryPool* pool = new MemoryPool(size);
        _pools.push_back(pool);

        while (tableIndex <= size)
        {
            _poolTable[tableIndex] = pool;
            tableIndex++;
        }
    }
}

Memory::~Memory()
{
    for (MemoryPool* pool : _pools)
        delete pool;

    _pools.clear();
}

void* Memory::Allocate(int32 size)
// �޸𸮸� �Ҵ��ϴ� �Լ��Դϴ�. 

{
    MemoryHeader* header = nullptr;
    // �޸� ����� null�� �ʱ�ȭ�մϴ�.

    const int32 allocSize = size + sizeof(MemoryHeader);
    // �Ҵ��� �޸� ũ�⸦ ����մϴ�. ��û�� ũ�⿡ �޸� ����� ũ�⸦ ���� ���Դϴ�.

#ifdef _STOMP
    header = reinterpret_cast<MemoryHeader*>(StompAllocator::Alloc(allocSize));
    // _STOMP�� ���ǵǾ� ������, StompAllocator�� ����Ͽ� �޸𸮸� �Ҵ��ϰ�, �� �ּҸ� header�� �����մϴ�.
#else
    if (allocSize > MAX_ALLOC_SIZE)
        // �Ҵ��Ϸ��� �޸� ũ�Ⱑ �޸� Ǯ�� �ִ� ũ�⸦ �ʰ��ϸ�

    {
        // �޸� Ǯ�� �ִ� ũ�⸦ ����� �Ϲ� �Ҵ�
        header = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(allocSize, SLIST_ALIGNMENT));
        // �Ϲ� �޸� �Ҵ��� �����ϰ� �� �ּҸ� header�� �����մϴ�.
    }
    else
        // �Ҵ��Ϸ��� �޸� ũ�Ⱑ �޸� Ǯ�� �ִ� ũ�� �����̸�

    {
        // �޸� Ǯ���� �����´�
        header = _poolTable[allocSize]->Pop();
        // �޸� Ǯ���� �޸𸮸� �Ҵ��ϰ� �� �ּҸ� header�� �����մϴ�.
    }
#endif	

    return MemoryHeader::AttachHeader(header, allocSize);
    // �Ҵ�� �޸𸮿� �޸� ����� ���̰�, �޸� ����� ���� �޸��� �ּҸ� ��ȯ�մϴ�.
}


void Memory::Release(void* ptr)
// �޸𸮸� �����ϴ� �Լ��Դϴ�.

{
    MemoryHeader* header = MemoryHeader::DetachHeader(ptr);
    // �޸𸮿��� �޸� ����� �и��ϰ�, �и��� �޸� ����� �ּҸ� header�� �����մϴ�.

    const int32 allocSize = header->allocSize;
    // ������ �޸� ũ�⸦ �����ɴϴ�.

    ASSERT_CRASH(allocSize > 0);
    // �޸� ũ�Ⱑ 0���� ū�� Ȯ���մϴ�. 0���ϸ� ���α׷��� �ߴܽ�ŵ�ϴ�.

#ifdef _STOMP
    StompAllocator::Release(header);
    // _STOMP�� ���ǵǾ� ������, StompAllocator�� ����Ͽ� �޸𸮸� �����մϴ�.
#else
    if (allocSize > MAX_ALLOC_SIZE)
        // �����Ϸ��� �޸� ũ�Ⱑ �޸� Ǯ�� �ִ� ũ�⸦ �ʰ��ϸ�

    {
        // �޸� Ǯ�� �ִ� ũ�⸦ ����� �Ϲ� ����
        ::_aligned_free(header);
        // �Ϲ� �޸� ������ �����մϴ�.
    }
    else
        // �����Ϸ��� �޸� ũ�Ⱑ �޸� Ǯ�� �ִ� ũ�� �����̸�
    {
        // �޸� Ǯ�� �ݳ��Ѵ�
        _poolTable[allocSize]->Push(header);
        // �޸� Ǯ�� �޸𸮸� ��ȯ�մϴ�.
    }
#endif	
}