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
    MemoryHeader(int32 size) : allocSize(size) {} // 생성자입니다. 할당된 메모리 크기를 인자로 받습니다.

    static void* AttachHeader(MemoryHeader* header, int32 size)
    {
        new(header)MemoryHeader(size); // placement new를 사용해 메모리 헤더를 할당받은 메모리에 생성합니다.
        return reinterpret_cast<void*>(++header); // 헤더 뒤의 메모리 주소를 반환합니다.
    }

    static MemoryHeader* DetachHeader(void* ptr)
    {
        MemoryHeader* header = reinterpret_cast<MemoryHeader*>(ptr) - 1; // 헤더의 주소를 구합니다.
        return header; // 헤더의 주소를 반환합니다.
    }

    int32 allocSize; // 이 메모리 블록에 할당된 데이터의 크기를 저장합니다.
    // TODO : 필요한 추가 정보
};

/*-----------------
    MemoryPool
------------------*/

class MemoryPool
{
public:
    MemoryPool(int32 allocSize); // MemoryPool의 생성자입니다. 할당하려는 메모리의 크기를 인자로 받습니다.
    ~MemoryPool(); // 소멸자입니다. 메모리 풀에 남아있는 모든 메모리를 해제합니다.

    void			Push(MemoryHeader* ptr); // 메모리 풀에 메모리를 반환하는 함수입니다.
    MemoryHeader* Pop(); // 메모리 풀에서 메모리를 꺼내는 함수입니다.

private:
    SLIST_HEADER    _header;
    int32           _allocSize = 0; // 메모리 풀에서 관리하는 메모리 블록의 크기입니다.
    atomic<int32>   _allocCount = 0; // 현재 할당된 메모리 블록의 개수입니다.

};

