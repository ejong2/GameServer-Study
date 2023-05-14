#pragma once

template<typename T>
class LockStack
{
};

template<typename T>
class LockFreeStack
{
	struct Node;

	struct CountedNodePtr
	{
		int32 externalCount = 0; // 외부에서 카운트하는 참조 개수를 저장합니다.
		Node* ptr = nullptr; // 노드 포인터를 저장합니다.
	};

	struct Node
	{
		Node(const T& value) : data(make_shared<T>(value))
		{
			// 노드 생성자입니다. 전달된 값으로 data를 초기화합니다.
		}

		shared_ptr<T> data; // 노드에 저장될 데이터를 저장합니다.
		atomic<int32> internalCount = 0; // 노드 내부에서 카운트하는 참조 개수를 저장합니다.
		CountedNodePtr next; // 다음 노드를 가리키는 포인터입니다.
	};
public:
	void Push(const T& value)
	{
		CountedNodePtr node;
		node.ptr = new Node(value);
		node.externalCount = 1;

		node.ptr->next = _head;
		while (_head.compare_exchange_weak(node.ptr->next, node) == false)
		{
			// _head를 새로운 노드로 바꾸려고 시도합니다. 실패하면 계속 시도합니다.
		}
	}

	shared_ptr<T> TryPop()
	{
		CountedNodePtr oldHead = _head;
		while (true)
		{
			IncreaseHeadCount(oldHead); // 참조 개수를 증가시킵니다.
			Node* ptr = oldHead.ptr;

			if (ptr == nullptr) // 데이터가 없으면 nullptr을 반환합니다.
				return shared_ptr<T>();

			if (_head.compare_exchange_strong(oldHead, ptr->next))
			{
				shared_ptr<T> res;
				res.swap(ptr->data);

				const int32 countIncrease = oldHead.externalCount - 2;

				if (ptr->internalCount.fetch_add(countIncrease) == -countIncrease)
					delete ptr;

				return res;
			}
			else if (ptr->internalCount.fetch_sub(1) == 1)
			{
				delete ptr;
			}
		}
	}
private:
	void IncreaseHeadCount(CountedNodePtr& oldCounter)
	{
		while (true)
		{
			CountedNodePtr newCounter = oldCounter;
			newCounter.externalCount++;

			if (_head.compare_exchange_strong(oldCounter, newCounter))
			{
				oldCounter.externalCount = newCounter.externalCount;
				break;
			}
		}
	}
private:
	atomic<CountedNodePtr> _head;
};

/* 이 코드에서는 락-프리 스택의 3번째 버전입니다. 이전 버전과의 차이점은 노드가 공유 포인터를 사용하지 않고,
CountedNodePtr라는 새로운 구조체를 사용하여 참조 개수를 관리합니다. 이 구조체는 외부에서 카운트하는 참조 개수와 노드 포인터를 저장합니다.
Push 함수에서는 새로운 노드를 생성하고, 외부 카운트를 1로 초기화한 다음, _head를 새로운 노드로 바꾸려고 시도합니다. 실패하면 계속 시도합니다.

TryPop 함수에서는 먼저 참조 개수를 증가시키고, 데이터가 없으면 nullptr을 반환합니다. 참조권은 얻었으나 소유권은 실패한 경우, 뒷수습을 진행합니다. 이전에 미리 증가시킨 외부 카운트를 2만큼 감소시킨 값으로 내부 카운트를 증가시킵니다. 내부 카운트가 -countIncrease와 같으면 노드를 삭제합니다.

IncreaseHeadCount 함수는 _head의 참조 개수를 증가시키는 역할을 합니다. 이 함수에서는 먼저 oldCounter와 같은 값을 가진 새로운 카운터를 생성하고 외부 카운트를 1만큼 증가시킵니다. 이후 _head를 변경하려고 시도하고 성공하면 oldCounter의 외부 카운트를 새로운 카운터의 외부 카운트로 변경하고 반복문을 빠져나옵니다.

LockFreeStack 클래스는 락이 없는 스택 구조를 구현한 클래스입니다. 이 클래스는 atomic 연산을 사용하여 여러 스레드에서 동시에 접근할 때 발생할 수 있는 동기화 문제를 해결하고자 합니다. 이를 통해 스레드가 스택에서 원소를 추가(Push)하거나 제거(TryPop)할 때 락을 걸지 않고도 안전한 동작을 보장합니다. 이로 인해 성능이 개선되며, 다중 스레드 환경에서의 경쟁 조건을 최소화할 수 있습니다. */