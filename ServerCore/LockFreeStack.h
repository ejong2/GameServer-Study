#pragma once

// -------------------
//		1차 시도
// -------------------
/*
struct SListEntry
{
	SListEntry* next;
};

struct SListHeader
{
	SListEntry* next = nullptr;
};

void InitializeHead(SListHeader* header);
void PushEntrySList(SListHeader* header, SListEntry* entry);
SListEntry* PopEntrySList(SListHeader* header);
*/

// -------------------
//		2차 시도
// -------------------

//struct SListEntry
//{
//	SListEntry* next;
//};
//
//struct SListHeader
//{
//	SListEntry* next = nullptr;
//};
//
//void InitializeHead(SListHeader* header);
//void PushEntrySList(SListHeader* header, SListEntry* entry);
//SListEntry* PopEntrySList(SListHeader* header);

// -------------------
//		3차 시도
// -------------------

DECLSPEC_ALIGN(16)
struct SListEntry
{
	SListEntry* next;
};

// SListEntry를 16바이트 정렬로 정의합니다. 
// 각 엔트리는 다음 엔트리를 가리키는 포인터를 포함합니다.

DECLSPEC_ALIGN(16)
struct SListHeader
{
	SListHeader()
	{
		alignment = 0;
		region = 0;
	}

	union
	{
		struct
		{
			uint64 alignment;
			uint64 region;
		} DUMMYSTRUCTNAME;
		struct
		{
			uint64 depth : 16;
			uint64 sequence : 48;
			uint64 reserved : 4;
			uint64 next : 60;
		} HeaderX64;
	};
};

// SListHeader를 16바이트 정렬로 정의합니다.
// 헤더는 리스트의 깊이, 시퀀스 번호, 예약된 영역, 다음 포인터를 가진 64비트 구조체를 사용합니다.

void InitializeHead(SListHeader* header);
void PushEntrySList(SListHeader* header, SListEntry* entry);
SListEntry* PopEntrySList(SListHeader* header);