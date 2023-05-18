class BaseAllocator
{
public:
	static void* Alloc(int32 size); // 기본 할당자로 메모리를 할당하는 함수입니다.
	static void Release(void* ptr); // 기본 할당자로 메모리를 해제하는 함수입니다.
};

class StompAllocator
{
	enum { PAGE_SIZE = 0x1000 }; // 페이지 크기를 상수로 정의합니다.

public:
	static void* Alloc(int32 size); // 메모리를 페이지 단위로 할당하는 함수입니다.
	static void Release(void* ptr); // 페이지 단위로 할당된 메모리를 해제하는 함수입니다.
};

class PoolAllocator
{
public:
	static void* Alloc(int32 size); // 메모리 풀에서 메모리를 할당하는 함수입니다.
	static void Release(void* ptr); // 메모리 풀로 메모리를 반환하는 함수입니다.
};

template<typename T>
class StlAllocator
{
public:
	using value_type = T; // STL Allocator의 타입을 정의합니다.

	StlAllocator() {} // 기본 생성자입니다.

	template<typename Other>
	StlAllocator(const StlAllocator<Other>&) {} // 다른 타입의 할당자로부터 복사 생성하는 생성자입니다.

	T* allocate(size_t count) // 지정된 개수의 객체를 할당하는 함수입니다.
	{
		const int32 size = static_cast<int32>(count * sizeof(T));
		return static_cast<T*>(Xalloc(size));
	}

	void deallocate(T* ptr, size_t count) // 지정된 개수의 객체를 해제하는 함수입니다.
	{
		Xrelease(ptr);
	}
};
