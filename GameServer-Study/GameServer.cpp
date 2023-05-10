#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <future>
#include <windows.h>

// 2차원 배열을 사용하여 캐시에 대한 속도 차이를 확인하는 코드입니다.
int32 buffer[10000][10000];

int main()
{
    memset(buffer, 0, sizeof(buffer));

    // 행 우선 순회 방식으로 배열을 순회하며 합을 구합니다.
    // 이 방식은 캐시 메모리의 공간 지역성(Spatial Locality) 원리에 따라
    // 배열이 메모리에 연속적으로 저장되므로 캐시 효율성이 높습니다.
    {
        uint64 start = GetTickCount64(); // 시작 시간 기록

        int64 sum = 0;

        for (int32 i = 0; i < 10000; i++)
        {
            for (int32 j = 0; j < 10000; j++)
            {
                sum += buffer[i][j];
            }
        }

        uint64 end = GetTickCount64(); // 종료 시간 기록
        cout << "Elapsed Tick " << (end - start) << '\n'; // 경과 시간 출력
    }

    // 열 우선 순회 방식으로 배열을 순회하며 합을 구합니다.
    // 이 방식은 캐시 메모리의 공간 지역성 원리에 부합하지 않아
    // 캐시 미스가 더 자주 발생하며, 따라서 속도가 느려집니다.
    {
        uint64 start = GetTickCount64(); // 시작 시간 기록

        int64 sum = 0;
        for (int32 i = 0; i < 10000; i++)
        {
            for (int32 j = 0; j < 10000; j++)
            {
                sum += buffer[j][i];
            }
        }

        uint64 end = GetTickCount64(); // 종료 시간 기록
        cout << "Elapsed Tick " << (end - start) << '\n'; // 경과 시간 출력
    }
}

/* 이 코드에서 행 우선 방식이 더 빠른 이유는 캐시 메모리의 공간 지역성(Spatial Locality) 원리 때문입니다.
공간 지역성 원리는 프로그램이 메모리에 인접한 위치에 있는 데이터를 참조할 확률이 높다는 것을 의미합니다.
행 우선 방식은 메모리에 연속적으로 저장된 데이터를 순차적으로 접근하기 때문에 캐시 효율성이 높습니다.
반면, 열 우선 방식은 메모리 접근이 불연속적이므로 캐시 미스가 더 자주 발생하고 속도가 느려집니다. */