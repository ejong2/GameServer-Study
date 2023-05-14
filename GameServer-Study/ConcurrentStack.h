#pragma once

template<typename T>
class LockStack
{
};

template<typename T>
class LockFreeStack
{
	struct Node
	{
		Node(const T& value) : data(value), next(nullptr)
		{
			// 노드 생성자입니다. 전달된 값으로 data를 초기화하고, next를 nullptr로 초기화합니다.
		}

		T data;
		Node* next;
	};
public:


	void Push(const T& value)
	{
		Node* node = new Node(value);
		node->next = _head;

		while (_head.compare_exchange_weak(node->next, node) == false)
		{

		}
	}

	bool TryPop(T& value)
	{
		++_popCount; // Pop을 실행하는 스레드의 개수를 증가시킵니다.

		Node* oldHead = _head;

		while (oldHead && _head.compare_exchange_weak(oldHead, oldHead->next) == false)
		{

		}

		if (oldHead == nullptr)
		{
			--_popCount;
			return false;
		}
		value = oldHead->data;
		TryDelete(oldHead); // 추출한 노드를 삭제하려고 시도합니다.
		return true;
	}

	void TryDelete(Node* oldHead)
	{
		if (_popCount == 1)
		{
			Node* node = _pendingList.exchange(nullptr);

			if (--_popCount == 0)
			{
				DeleteNodes(node);
			}
			else
			{
				ChainPendingNodeList(node);
			}

			delete oldHead;
		}
		else
		{
			ChainPendingNode(oldHead);
			--_popCount;
		}
	}

	void ChainPendingNodeList(Node* first, Node* last)
	{
		last->next = _pendingList;

		while (_pendingList.compare_exchange_weak(last->next, first) == false)
		{

		}
	}

	void ChainPendingNodeList(Node* node)
	{
		Node* last = node;
		while (last->next)
			last = last->next;

		ChainPendingNodeList(node, last);
	}

	void ChainPendingNode(Node* node)
	{
		ChainPendingNodeList(node, node);
	}

	static void DeleteNodes(Node* node)
	{
		while (node)
		{
			Node* next = node->next;
			delete node;
			node = next;
		}
	}
private:
	atomic<Node*> _head;


	atomic<uint32> _popCount = 0; // Pop을 실행중인 쓰레드 개수를 저장하는 변수입니다.
	atomic<Node*> _pendingList; // 삭제 되어야 할 노드들의 리스트를 저장하는 변수입니다.
};

/* 이 코드에서는 삭제해야 하는 노드들을 관리하는 방법이 추가되었습니다.
Pop을 실행하는 스레드의 개수를 카운트하고, TryDelete 함수를 통해 삭제 대상 노드를 관리합니다.
이를 통해 락-프리 스택에서 노드를 안전하게 삭제할 수 있습니다. 또한, ChainPendingNodeList와 ChainPendingNode 함수를 사용해
삭제 대상 노드들을 연결하고 관리할 수 있습니다. 이를 통해 락-프리 스택에서 노드의 삭제를 효율적으로 처리할 수 있습니다. */