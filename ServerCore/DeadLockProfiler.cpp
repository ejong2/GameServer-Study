#include "pch.h"
#include "DeadLockProfiler.h"

void DeadLockProfiler::PushLock(const char* name)
{
    // 멀티스레드 환경에서 동기화를 위한 lock guard 사용
    LockGuard guard(_lock);

    // 락의 아이디를 찾거나 새로 발급
    int32 lockId = 0;
    auto findIt = _nameToId.find(name);
    if (findIt == _nameToId.end())
    {
        // 새로운 락이면 새로운 아이디를 부여하고, 아이디-이름 매핑 정보를 저장
        lockId = static_cast<int32>(_nameToId.size());
        _nameToId[name] = lockId;
        _idToName[lockId] = name;
    }
    else
    {
        // 이미 존재하는 락이면 아이디를 찾아서 사용
        lockId = findIt->second;
    }

    // 이미 락을 획득한 상태라면
    if (_lockStack.empty() == false)
    {
        // 기존에 발견되지 않은 락 획득 순서라면 데드락 여부를 확인
        const int32 prevId = _lockStack.top();
        if (lockId != prevId)
        {
            set<int32>& history = _lockHistroy[prevId];
            if (history.find(lockId) == history.end())
            {
                // 새로운 락 획득 순서를 기록하고, 데드락을 체크
                history.insert(lockId);
                CheckCycle();
            }
        }
    }

    // 락을 스택에 푸시
    _lockStack.push(lockId);
}

void DeadLockProfiler::PopLock(const char* name)
{
    // 멀티스레드 환경에서 동기화를 위한 lock guard 사용
    LockGuard guard(_lock);

    // 스택이 비어있는데 PopLock이 호출되면 에러 발생
    if (_lockStack.empty())
        CRASH("MULTIPLE_UNLOCK");

    int32 lockId = _nameToId[name];
    // 스택 최상단의 락이 아닌 락을 해제하려는 시도는 잘못된 언락 시도로 판단하고 에러 발생
    if (_lockStack.top() != lockId)
        CRASH("INVALID_UNLOCK");

    // 락을 스택에서 팝
    _lockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
    // 락의 개수를 계산
    const int32 lockCount = static_cast<int32>(_nameToId.size());

    // 노드 발견 순서, 완료 여부, 부모 노드를 초기화
    _discoveredOrder = vector<int32>(lockCount, -1);
    _discoveredCount = 0;
    _finished = vector<bool>(lockCount, false);
    _parent = vector<int32>(lockCount, -1);
    for (int32 lockId = 0; lockId < lockCount; lockId++)
        // 깊이 우선 탐색(DFS)을 시작
        Dfs(lockId);

    // DFS가 끝나면 각종 정보를 초기화
    _discoveredOrder.clear();
    _finished.clear();
    _parent.clear();
}

void DeadLockProfiler::Dfs(int32 here)
{
    // 이미 방문한 노드는 중복 방문하지 않음
    if (_discoveredOrder[here] != -1)
        return;

    // 방문한 노드를 기록
    _discoveredOrder[here] = _discoveredCount++;

    // 모든 인접한 노드를 순회
    auto findIt = _lockHistroy.find(here);
    if (findIt == _lockHistroy.end())
    {
        // 인접한 노드가 없으면 탐색 종료
        _finished[here] = true;
        return;
    }

    set<int32>& nextSet = findIt->second;
    for (int32 there : nextSet)
    {
        // 아직 방문하지 않은 노드면 방문
        if (_discoveredOrder[there] == -1)
        {
            _parent[there] = here;
            Dfs(there);
            continue;
        }

        // here가 there보다 먼저 발견되었다면, there는 here의 후손(순방향 간선)
        if (_discoveredOrder[here] < _discoveredOrder[there])
            continue;

        // 순방향이 아니고, there가 아직 탐색이 종료되지 않았다면, there는 here의 선조(역방향 간선)
        if (_finished[there] == false)
        {
            // 데드락이 발견된 경우 경로를 출력하고 프로그램 종료
            printf("%s -> %s\n", _idToName[here], _idToName[there]);

            int32 now = here;
            while (true)
            {
                printf("%s -> %s\n", _idToName[_parent[now]], _idToName[now]);
                now = _parent[now];
                if (now == there)
                    break;
            }

            CRASH("DEADLOCK_DETECTED");
        }
    }

    // 현재 노드의 탐색이 종료되면 _finished 플래그를 true로 설정
    _finished[here] = true;
}