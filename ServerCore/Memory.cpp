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
// 메모리를 할당하는 함수입니다. 

{
    MemoryHeader* header = nullptr;
    // 메모리 헤더를 null로 초기화합니다.

    const int32 allocSize = size + sizeof(MemoryHeader);
    // 할당할 메모리 크기를 계산합니다. 요청된 크기에 메모리 헤더의 크기를 더한 값입니다.

#ifdef _STOMP
    header = reinterpret_cast<MemoryHeader*>(StompAllocator::Alloc(allocSize));
    // _STOMP이 정의되어 있으면, StompAllocator를 사용하여 메모리를 할당하고, 그 주소를 header에 저장합니다.
#else
    if (allocSize > MAX_ALLOC_SIZE)
        // 할당하려는 메모리 크기가 메모리 풀의 최대 크기를 초과하면

    {
        // 메모리 풀링 최대 크기를 벗어나면 일반 할당
        header = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(allocSize, SLIST_ALIGNMENT));
        // 일반 메모리 할당을 수행하고 그 주소를 header에 저장합니다.
    }
    else
        // 할당하려는 메모리 크기가 메모리 풀의 최대 크기 이하이면

    {
        // 메모리 풀에서 꺼내온다
        header = _poolTable[allocSize]->Pop();
        // 메모리 풀에서 메모리를 할당하고 그 주소를 header에 저장합니다.
    }
#endif	

    return MemoryHeader::AttachHeader(header, allocSize);
    // 할당된 메모리에 메모리 헤더를 붙이고, 메모리 헤더가 붙은 메모리의 주소를 반환합니다.
}


void Memory::Release(void* ptr)
// 메모리를 해제하는 함수입니다.

{
    MemoryHeader* header = MemoryHeader::DetachHeader(ptr);
    // 메모리에서 메모리 헤더를 분리하고, 분리된 메모리 헤더의 주소를 header에 저장합니다.

    const int32 allocSize = header->allocSize;
    // 해제할 메모리 크기를 가져옵니다.

    ASSERT_CRASH(allocSize > 0);
    // 메모리 크기가 0보다 큰지 확인합니다. 0이하면 프로그램을 중단시킵니다.

#ifdef _STOMP
    StompAllocator::Release(header);
    // _STOMP이 정의되어 있으면, StompAllocator를 사용하여 메모리를 해제합니다.
#else
    if (allocSize > MAX_ALLOC_SIZE)
        // 해제하려는 메모리 크기가 메모리 풀의 최대 크기를 초과하면

    {
        // 메모리 풀링 최대 크기를 벗어나면 일반 해제
        ::_aligned_free(header);
        // 일반 메모리 해제를 수행합니다.
    }
    else
        // 해제하려는 메모리 크기가 메모리 풀의 최대 크기 이하이면
    {
        // 메모리 풀에 반납한다
        _poolTable[allocSize]->Push(header);
        // 메모리 풀에 메모리를 반환합니다.
    }
#endif	
}