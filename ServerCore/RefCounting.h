#pragma once

/*----------------
   RefCountable
----------------*/

// ���� ī������ �����ϴ� �⺻ Ŭ����
class RefCountable
{
public:
    // �����ڿ��� ���� ī���͸� 1�� �ʱ�ȭ
    RefCountable() : _refCount(1) {}

    // ���� �Ҹ���
    virtual ~RefCountable() {}

    // ���� ���� ī���͸� �������� �Լ�
    int32 GetRefCount() { return _refCount; }

    // ���� ī���͸� ������Ű�� �Լ�
    int32 AddRef() { return ++_refCount; }

    // ���� ī���͸� ���ҽ�Ű�� �Լ�, ���� ī���Ͱ� 0�� �Ǹ� ��ü�� ����
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
    // ���� ī����, ���� �����忡�� ���ÿ� ������ �� �����Ƿ� atomic Ÿ���� ���
    atomic<int32> _refCount;
};

/*----------------
    SharedPtr
----------------*/

// ����Ʈ ������ Ŭ���� ���ø�
template<typename T>
class TSharedPtr
{
public:
    // �⺻ ������
    TSharedPtr() {}

    // �����͸� ���޹޾� ����Ʈ �����͸� �ʱ�ȭ�ϴ� ������
    TSharedPtr(T* ptr) { Set(ptr); }

    // ���� ������
    TSharedPtr(const TSharedPtr& rhs) { Set(rhs._ptr); }

    // �̵� ������
    TSharedPtr(TSharedPtr&& rhs) { _ptr = rhs._ptr; rhs._ptr = nullptr; }

    // ��� ���� ���� ������
    template<typename U>
    TSharedPtr(const TSharedPtr<U>& rhs) { Set(static_cast<T*>(rhs._ptr)); }

    // �Ҹ���
    ~TSharedPtr() { Release(); }
public:
    // ���� ������ �����ε�
    TSharedPtr& operator=(const TSharedPtr& rhs)
    {
        if (_ptr != rhs._ptr)
        {
            Release();
            Set(rhs._ptr);
        }
        return *this;
    }

    // �̵� ������ �����ε�
    TSharedPtr& operator=(TSharedPtr&& rhs)
    {
        Release();
        _ptr = rhs._ptr;
        rhs._ptr = nullptr;
        return *this;
    }

    // �� �����ڵ� �� ������ ���� �����ڵ�

    bool		operator==(const TSharedPtr& rhs) const { return _ptr == rhs._ptr; }
    bool		operator==(T* ptr) const { return _ptr == ptr; }
    bool		operator!=(const TSharedPtr& rhs) const { return _ptr != rhs._ptr; }
    bool		operator!=(T* ptr) const { return _ptr != ptr; }
    bool		operator<(const TSharedPtr& rhs) const { return _ptr < rhs._ptr; }
    // ���� ���� ������ �����ε�, ����Ʈ �����Ͱ� ����Ű�� ��ü�� ����
    T* operator*() { return _ptr; }
    const T* operator*() const { return _ptr; }

    // Ÿ�� ��ȯ ������ �����ε�, ����Ʈ �����͸� ���� �����ͷ� ��ȯ
    operator T* () const { return _ptr; }

    // ȭ��ǥ ������ �����ε�, ����Ʈ �����Ͱ� ����Ű�� ��ü�� ����� ����
    T* operator->() { return _ptr; }
    const T* operator->() const { return _ptr; }

    // nullptr�� ���ϴ� �Լ�
    bool IsNull() { return _ptr == nullptr; }
private:
    // ����Ʈ �����Ͱ� ��ü�� ����Ű�� �����ϰ� ���� ī��Ʈ�� ������Ű�� �Լ�
    inline void Set(T* ptr)
    {
        _ptr = ptr;
        if (ptr)
            ptr->AddRef();
    }


    // ����Ʈ �����Ͱ� ����Ű�� ��ü�� ������ �����ϰ� �����͸� nullptr�� �����ϴ� �Լ�
    inline void Release()
    {
        if (_ptr != nullptr)
        {
            _ptr->ReleaseRef();
            _ptr = nullptr;
        }
    }
private:
    // ������ ����Ű�� ��ü�� ���� ������
    T* _ptr = nullptr;
};

