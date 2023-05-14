#pragma once

template<typename T>
class LockStack
{
	// 락을 사용하는 스택 클래스입니다.
};

template<typename T>
class LockFreeStack
{
	// 락을 사용하지 않는 락-프리 스택 클래스입니다.
	struct Node
	{
		Node(const T& value) : data(value)
		{
			// 노드 생성자입니다. 전달된 값으로 data를 초기화합니다.
		}

		T data;
		Node* next;
	};
public:


	void Push(const T& value)
	{
		// 새 노드를 생성하고, 스택의 head에 추가합니다.
		Node* node = new Node(value);
		node->next = _head;

		// compare_exchange_weak를 사용하여 head를 변경하는 작업이 원자적으로 수행되도록 합니다.
		while (_head.compare_exchange_weak(node->next, node) == false)
		{

		}
	}

	// 1) head 읽기
	// 2) head->next 읽기
	// 3) head = head->next
	// 4) data 추출해서 반환
	// 5) 추출한 노드를 삭제
	bool TryPop(T& value)
	{
		// 스택에서 노드를 추출하여 값을 반환하는 함수입니다.
		Node* oldHead = _head;

		// compare_exchange_weak를 사용하여 head를 변경하는 작업이 원자적으로 수행되도록 합니다.
		while (oldHead && _head.compare_exchange_weak(oldHead, oldHead->next) == false)
		{

		}

		if (oldHead == nullptr)
			return false; // 노드가 없는 경우, false를 반환합니다.

		value = oldHead->data; // 노드의 값을 반환합니다.

		return true;
	}
private:
	atomic<Node*> _head; // 스택의 head를 원자적으로 관리합니다.
};

/* 이 코드에서는 락-프리 스택을 구현했습니다. 락-프리 자료구조는 락을 사용하지 않고 동시성을 보장하는 자료구조입니다.
이 경우에는 원자적 연산인 compare_exchange_weak를 사용하여 락-프리로 스택의 노드를 추가하고 추출하는 기능을 구현했습니다.
락-프리 자료구조는 락을 사용하지 않기 때문에 경쟁 상태로 인한 성능 저하가 발생하지 않으며, 데드락이 발생하지 않는 이점이 있습니다. */