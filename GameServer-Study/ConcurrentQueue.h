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

    // ���� �����ڿ� ���� �����ڸ� �����Ͽ� LockQueue ��ü�� ������� �ʵ��� �մϴ�.
    LockQueue(const LockQueue&) = delete;
    LockQueue& operator=(const LockQueue&) = delete;

    // ť�� ���� �߰��ϴ� Push �Լ��Դϴ�. ���ؽ��� ����Ͽ� ���ü��� �����մϴ�.
    void Push(T value)
    {
        lock_guard<mutex> lock(_mutex);
        _queue.push(std::move(value));
        _condVar.notify_one();
    }

    // ť���� ���� �������� TryPop �Լ��Դϴ�. ���ؽ��� ����Ͽ� ���ü��� �����մϴ�.
    bool TryPop(T& value)
    {
        lock_guard<mutex> lock(_mutex);
        if (_queue.empty())
        {
            return false;
        }

        // ������� �ʴٸ� ���� �����ɴϴ�.
        value = std::move(_queue.front());
        _queue.pop();

        return true;
    }

    // ���� ������ ������ ����ϴ� WaitPop �Լ��Դϴ�.
    void WaitPop(T& value)
    {
        unique_lock<mutex> lock(_mutex);
        _condVar.wait(lock, [this] { return _queue.empty() == false; });
        value = std::move(_queue.front());
        _queue.pop();
    }

private:
    queue<T> _queue; // ť ��ü
    mutex _mutex; // ���ü��� �����ϱ� ���� ���ؽ� ��ü
    condition_variable _condVar; // ��� ���¸� �����ϱ� ���� condition_variable ��ü
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
    // ��� ����ü�� �����մϴ�.
    struct Node;

    // �ܺο��� ī��Ʈ�ϴ� ���� ������ ��� �����͸� �����ϴ� ����ü�� �����մϴ�.
    struct CountedNodePtr
    {
        int32 externalCount; // ������
        Node* ptr = nullptr;
    };

    // ��� ī���� ����ü�� �����մϴ�.
    struct NodeCounter
    {
        uint32 internalCount : 30; // ������ ��ȯ ����
        uint32 externalCountRemaining : 2; // Push & Pop ���� ������ ����
    };

    // ��� ����ü�� �����մϴ�.
    struct Node
    {
        // ��� �������Դϴ�. ī���� ���� �ʱ�ȭ�մϴ�.
        Node()
        {
            NodeCounter newCount;
            newCount.internalCount = 0;
            newCount.externalCountRemaining = 2;
            count.store(newCount);

            next.ptr = nullptr;
            next.externalCount = 0;
        }

        // ����� ���� ������ ���ҽ�Ű�� �Լ��Դϴ�.
        void ReleaseRef()
        {
            NodeCounter oldCounter = count.load();

            while (true)
            {
                NodeCounter newCounter = oldCounter;
                newCounter.internalCount--;

                // ����� �� ����
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
    // ������ ť �������Դϴ�.
    LockFreeQueue()
    {
        CountedNodePtr node;
        node.ptr = new Node;
        node.externalCount = 1;


        _head.store(node);
        _tail.store(node);
    }

    // ���� �����ڿ� ���� �����ڸ� �����մϴ�.
    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue& operator=(const LockFreeQueue&) = delete;

    // ���� ������ ť�� �߰��ϴ� �Լ��Դϴ�.
    void Push(const T& value)
    {
        unique_ptr<T> newData = make_unique<T>(value);

        CountedNodePtr dummy;
        dummy.ptr = new Node;
        dummy.externalCount = 1;

        CountedNodePtr oldTail = _tail.load(); // ptr = nullptr

        while (true)
        {
            // ������ ȹ�� (externalCount�� ������ ���� +1 �� �ְ� �̱�)
            IncreaseExternalCount(_tail, oldTail);

            // ������ ȹ�� (data�� ���� ��ȯ�� �ְ� �̱�)
            T* oldData = nullptr;
            if (oldTail.ptr->data.compare_exchange_strong(oldData, newData.get()))
            {
                oldTail.ptr->next = dummy;
                oldTail = _tail.exchange(dummy);
                FreeExternalCount(oldTail);

                newData.release(); // �����Ϳ� ���� unique_ptr�� ������ ����
                break;
            }

            // ������ ���� �й�..
            oldTail.ptr->ReleaseRef();
        }
    }

    // ������ ť���� ���� �������� �õ��ϴ� �Լ��Դϴ�.
    shared_ptr<T> TryPop()
    {
        CountedNodePtr oldHead = _head.load();

        while (true)
        {
            // ������ ȹ�� (externalCount�� ������ ���� +1 �� �ְ� �̱�)
            IncreaseExternalCount(_head, oldHead);

            Node* ptr = oldHead.ptr;
            if (ptr == _tail.load().ptr)
            {
                ptr->ReleaseRef();
                return unique_ptr<T>();
            }

            // ������ ȹ�� (head = ptr->next)
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
    // �ܺ� ī��Ʈ�� ������Ű�� �Լ��Դϴ�.
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

    // �ܺ� ī��Ʈ�� �����ϴ� �Լ��Դϴ�.
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