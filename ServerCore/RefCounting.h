#pragma once

/*----------------
   RefCountable
----------------*/

// 참조 카운팅을 지원하는 기본 클래스
class RefCountable
{
public:
    // 생성자에서 참조 카운터를 1로 초기화
    RefCountable() : _refCount(1) {}

    // 가상 소멸자
    virtual ~RefCountable() {}

    // 현재 참조 카운터를 가져오는 함수
    int32 GetRefCount() { return _refCount; }

    // 참조 카운터를 증가시키는 함수
    int32 AddRef() { return ++_refCount; }

    // 참조 카운터를 감소시키는 함수, 참조 카운터가 0이 되면 객체를 삭제
    int32 ReleaseRef()
    {
        int32 refCount = --_refCount;
        if (refCount == 0)
        {
            delete this;
        }
        return refCount;
    }
protected:
    // 참조 카운터, 여러 스레드에서 동시에 접근할 수 있으므로 atomic 타입을 사용
    atomic<int32> _refCount;
};

/*----------------
    SharedPtr
----------------*/

// 스마트 포인터 클래스 템플릿
template<typename T>
class TSharedPtr
{
public:
    // 기본 생성자
    TSharedPtr() {}

    // 포인터를 전달받아 스마트 포인터를 초기화하는 생성자
    TSharedPtr(T* ptr) { Set(ptr); }

    // 복사 생성자
    TSharedPtr(const TSharedPtr& rhs) { Set(rhs._ptr); }

    // 이동 생성자
    TSharedPtr(TSharedPtr&& rhs) { _ptr = rhs._ptr; rhs._ptr = nullptr; }

    // 상속 관계 복사 생성자
    template<typename U>
    TSharedPtr(const TSharedPtr<U>& rhs) { Set(static_cast<T*>(rhs._ptr)); }

    // 소멸자
    ~TSharedPtr() { Release(); }
public:
    // 복사 연산자 오버로딩
    TSharedPtr& operator=(const TSharedPtr& rhs)
    {
        if (_ptr != rhs._ptr)
        {
            Release();
            Set(rhs._ptr);
        }
        return *this;
    }

    // 이동 연산자 오버로딩
    TSharedPtr& operator=(TSharedPtr&& rhs)
    {
        Release();
        _ptr = rhs._ptr;
        rhs._ptr = nullptr;
        return *this;
    }

    // 비교 연산자들 및 포인터 접근 연산자들

    bool		operator==(const TSharedPtr& rhs) const { return _ptr == rhs._ptr; }
    bool		operator==(T* ptr) const { return _ptr == ptr; }
    bool		operator!=(const TSharedPtr& rhs) const { return _ptr != rhs._ptr; }
    bool		operator!=(T* ptr) const { return _ptr != ptr; }
    bool		operator<(const TSharedPtr& rhs) const { return _ptr < rhs._ptr; }
    // 복합 대입 연산자 오버로딩, 스마트 포인터가 가리키는 객체를 접근
    T* operator*() { return _ptr; }
    const T* operator*() const { return _ptr; }

    // 타입 변환 연산자 오버로딩, 스마트 포인터를 원시 포인터로 변환
    operator T* () const { return _ptr; }

    // 화살표 연산자 오버로딩, 스마트 포인터가 가리키는 객체의 멤버를 접근
    T* operator->() { return _ptr; }
    const T* operator->() const { return _ptr; }

    // nullptr과 비교하는 함수
    bool IsNull() { return _ptr == nullptr; }
private:
    // 스마트 포인터가 객체를 가리키게 설정하고 참조 카운트를 증가시키는 함수
    inline void Set(T* ptr)
    {
        _ptr = ptr;
        if (ptr)
            ptr->AddRef();
    }


    // 스마트 포인터가 가리키는 객체의 참조를 해제하고 포인터를 nullptr로 설정하는 함수
    inline void Release()
    {
        if (_ptr != nullptr)
        {
            _ptr->ReleaseRef();
            _ptr = nullptr;
        }
    }
private:
    // 실제로 가리키는 객체에 대한 포인터
    T* _ptr = nullptr;
};

