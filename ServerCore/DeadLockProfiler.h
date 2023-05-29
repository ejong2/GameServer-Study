#pragma once
#include <stack>
#include <map>
#include <vector>

class DeadLockProfiler
{
public:
    // 락을 획득하려 할 때 호출하는 함수입니다.
    void PushLock(const char* name);

    // 락을 해제하려 할 때 호출하는 함수입니다.
    void PopLock(const char* name);

    // 데드락 가능성을 체크하는 함수입니다.
    void CheckCycle();

private:
    // 깊이 우선 탐색을 수행하는 함수입니다.
    void Dfs(int32 index);

private:
    // 락의 이름을 키로 하고, 락의 아이디를 값으로 갖는 해시 맵입니다.
    unordered_map<const char*, int32>	_nameToId;

    // 락의 아이디를 키로 하고, 락의 이름을 값으로 갖는 해시 맵입니다.
    unordered_map<int32, const char*>	_idToName;

    // 현재 획득한 락들을 저장하는 스택입니다.

    // 락들 간의 획득 순서를 기록하는 맵입니다.
    map<int32, set<int32>>				_lockHistroy;

    // 멀티스레드 환경에서 동기화를 위한 뮤텍스입니다.
    Mutex _lock;

private:
    // 노드가 발견된 순서를 기록하는 배열입니다.
    vector<int32>	_discoveredOrder;

    // 노드가 발견된 순서를 카운팅하는 변수입니다.
    int32			_discoveredCount = 0;

    // 각 노드에 대해 DFS가 완료되었는지 여부를 저장하는 배열입니다.
    vector<bool>	_finished;

    // 각 노드의 부모 노드 아이디를 저장하는 배열입니다.
    vector<int32>	_parent;
};