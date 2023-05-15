#pragma once
#include <mutex>

class ConcurrentQueue
{
};

template<typename T>
class LockQueue
{
public:
    LockQueue() {}

    // 복사 생성자와 대입 연산자를 삭제하여 LockQueue 객체가 복사되지 않도록 합니다.
    LockQueue(const LockQueue&) = delete;
    LockQueue& operator=(const LockQueue&) = delete;

    // 큐에 값을 추가하는 Push 함수입니다. 뮤텍스를 사용하여 동시성을 보장합니다.
    void Push(T value)
    {
        lock_guard<mutex> lock(_mutex);
        _queue.push(std::move(value));
        _condVar.notify_one();
    }

    // 큐에서 값을 꺼내오는 TryPop 함수입니다. 뮤텍스를 사용하여 동시성을 보장합니다.
    bool TryPop(T& value)
    {
        lock_guard<mutex> lock(_mutex);
        if (_queue.empty())
        {
            return false;
        }

        // 비어있지 않다면 값을 가져옵니다.
        value = std::move(_queue.front());
        _queue.pop();

        return true;
    }

    // 값을 가져올 때까지 대기하는 WaitPop 함수입니다.
    void WaitPop(T& value)
    {
        unique_lock<mutex> lock(_mutex);
        _condVar.wait(lock, [this] { return _queue.empty() == false; });
        value = std::move(_queue.front());
        _queue.pop();
    }

private:
    queue<T> _queue; // 큐 객체
    mutex _mutex; // 동시성을 보장하기 위한 뮤텍스 객체
    condition_variable _condVar; // 대기 상태를 관리하기 위한 condition_variable 객체
};

//template<typename T>
//class LockFreeQueue
//{
//    struct Node
//    {
//        shared_ptr<T> data;
//        Node* next = nullptr;
//    };
//
//public:
//    LockFreeQueue() : _head(new Node), _tail(_head)
//    {
//
//    }
//
//    LockFreeQueue(const LockFreeQueue&) = delete;
//    LockFreeQueue& operator=(const LockFreeQueue&) = delete;
//
//    void Push(const T& value)
//    {
//        shared_ptr<T> newData = make_shared<T>(value);
//
//        Node* dummy = new Node();
//
//        Node* oldTail = _tail;
//        oldTail->data.swap(newData);
//        oldTail->next = dummy;
//
//        _tail = dummy;
//    }
//
//    shared_ptr<T> TryPop()
//    {
//        Node* oldHead = PopHead();
//        if (oldHead == nullptr)
//            return shared_ptr<T>();
//
//        shared_ptr<T> res(oldHead->data);
//        delete res;
//    }
//
//private:
//    Node* PopHead()
//    {
//        Node* oldHead = _head;
//        if (oldHead == _tail)
//            return nullptr;
//
//        _head = oldHead->next;
//        return oldHead;
//    }
//
//private:
//
//    Node* _head = nullptr;
//    Node * _tail = nullptr;
//};

template<typename T>
class LockFreeQueue
{
    // 노드 구조체를 선언합니다.
    struct Node;

    // 외부에서 카운트하는 참조 개수와 노드 포인터를 저장하는 구조체를 선언합니다.
    struct CountedNodePtr
    {
        int32 externalCount; // 참조권
        Node* ptr = nullptr;
    };

    // 노드 카운터 구조체를 선언합니다.
    struct NodeCounter
    {
        uint32 internalCount : 30; // 참조권 반환 관련
        uint32 externalCountRemaining : 2; // Push & Pop 다중 참조권 관련
    };

    // 노드 구조체를 선언합니다.
    struct Node
    {
        // 노드 생성자입니다. 카운터 값을 초기화합니다.
        Node()
        {
            NodeCounter newCount;
            newCount.internalCount = 0;
            newCount.externalCountRemaining = 2;
            count.store(newCount);

            next.ptr = nullptr;
            next.externalCount = 0;
        }

        // 노드의 참조 개수를 감소시키는 함수입니다.
        void ReleaseRef()
        {
            NodeCounter oldCounter = count.load();

            while (true)
            {
                NodeCounter newCounter = oldCounter;
                newCounter.internalCount--;

                // 끼어들 수 있음
                if (count.compare_exchange_strong(oldCounter, newCounter))
                {
                    if (newCounter.internalCount == 0 && newCounter.externalCountRemaining == 0)
                        delete this;

                    break;
                }
            }
        }

        atomic<T*> data;
        atomic<NodeCounter> count;
        CountedNodePtr next;
    };
public:
    // 락프리 큐 생성자입니다.
    LockFreeQueue()
    {
        CountedNodePtr node;
        node.ptr = new Node;
        node.externalCount = 1;


        _head.store(node);
        _tail.store(node);
    }

    // 복사 생성자와 대입 연산자를 삭제합니다.
    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue& operator=(const LockFreeQueue&) = delete;

    // 값을 락프리 큐에 추가하는 함수입니다.
    void Push(const T& value)
    {
        unique_ptr<T> newData = make_unique<T>(value);

        CountedNodePtr dummy;
        dummy.ptr = new Node;
        dummy.externalCount = 1;

        CountedNodePtr oldTail = _tail.load(); // ptr = nullptr

        while (true)
        {
            // 참조권 획득 (externalCount를 현시점 기준 +1 한 애가 이김)
            IncreaseExternalCount(_tail, oldTail);

            // 소유권 획득 (data를 먼저 교환한 애가 이김)
            T* oldData = nullptr;
            if (oldTail.ptr->data.compare_exchange_strong(oldData, newData.get()))
            {
                oldTail.ptr->next = dummy;
                oldTail = _tail.exchange(dummy);
                FreeExternalCount(oldTail);

                newData.release(); // 데이터에 대한 unique_ptr의 소유권 포기
                break;
            }

            // 소유권 경쟁 패배..
            oldTail.ptr->ReleaseRef();
        }
    }

    // 락프리 큐에서 값을 꺼내려고 시도하는 함수입니다.
    shared_ptr<T> TryPop()
    {
        CountedNodePtr oldHead = _head.load();

        while (true)
        {
            // 참조권 획득 (externalCount를 현시점 기준 +1 한 애가 이김)
            IncreaseExternalCount(_head, oldHead);

            Node* ptr = oldHead.ptr;
            if (ptr == _tail.load().ptr)
            {
                ptr->ReleaseRef();
                return unique_ptr<T>();
            }

            // 소유권 획득 (head = ptr->next)
            if (_head.compare_exchange_strong(oldHead, ptr->next))
            {
                T* res = ptr->data.exchange(nullptr);
                FreeExternalCount(oldHead);
                return shared_ptr<T>(res);
            }

            ptr->ReleaseRef();
        }
    }
private:
    // 외부 카운트를 증가시키는 함수입니다.
    static void IncreaseExternalCount(atomic<CountedNodePtr>& counter, CountedNodePtr& oldCounter)
    {
        while (true)
        {
            CountedNodePtr newCounter = oldCounter;
            newCounter.externalCount++;


            if (counter.compare_exchange_weak(oldCounter, newCounter))
            {
                oldCounter.externalCount = newCounter.externalCount;
                break;
            }
        }
    }

    // 외부 카운트를 해제하는 함수입니다.
    static void FreeExternalCount(CountedNodePtr& oldNodePtr)
    {
        Node* ptr = oldNodePtr.ptr;
        const int32 countIncrease = oldNodePtr.externalCount - 2;

        NodeCounter oldCounter = ptr->count.load();

        while (true)
        {
            NodeCounter newCounter = oldCounter;
            newCounter.externalCountRemaining--; // TODO
            newCounter.internalCount += countIncrease;

            if (ptr->count.compare_exchange_strong(oldCounter, newCounter))
            {
                if (newCounter.internalCount == 0 && newCounter.externalCountRemaining == 0)
                    delete ptr;

                break;
            }
        }
    }
private:


    atomic<CountedNodePtr> _head;
    atomic<CountedNodePtr> _tail;
};