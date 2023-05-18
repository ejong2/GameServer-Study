class BaseAllocator
{
public:
	static void* Alloc(int32 size); // �⺻ �Ҵ��ڷ� �޸𸮸� �Ҵ��ϴ� �Լ��Դϴ�.
	static void Release(void* ptr); // �⺻ �Ҵ��ڷ� �޸𸮸� �����ϴ� �Լ��Դϴ�.
};

class StompAllocator
{
	enum { PAGE_SIZE = 0x1000 }; // ������ ũ�⸦ ����� �����մϴ�.

public:
	static void* Alloc(int32 size); // �޸𸮸� ������ ������ �Ҵ��ϴ� �Լ��Դϴ�.
	static void Release(void* ptr); // ������ ������ �Ҵ�� �޸𸮸� �����ϴ� �Լ��Դϴ�.
};

class PoolAllocator
{
public:
	static void* Alloc(int32 size); // �޸� Ǯ���� �޸𸮸� �Ҵ��ϴ� �Լ��Դϴ�.
	static void Release(void* ptr); // �޸� Ǯ�� �޸𸮸� ��ȯ�ϴ� �Լ��Դϴ�.
};

template<typename T>
class StlAllocator
{
public:
	using value_type = T; // STL Allocator�� Ÿ���� �����մϴ�.

	StlAllocator() {} // �⺻ �������Դϴ�.

	template<typename Other>
	StlAllocator(const StlAllocator<Other>&) {} // �ٸ� Ÿ���� �Ҵ��ڷκ��� ���� �����ϴ� �������Դϴ�.

	T* allocate(size_t count) // ������ ������ ��ü�� �Ҵ��ϴ� �Լ��Դϴ�.
	{
		const int32 size = static_cast<int32>(count * sizeof(T));
		return static_cast<T*>(Xalloc(size));
	}

	void deallocate(T* ptr, size_t count) // ������ ������ ��ü�� �����ϴ� �Լ��Դϴ�.
	{
		Xrelease(ptr);
	}
};
