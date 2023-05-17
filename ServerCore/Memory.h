#pragma once
#include "Allocator.h"

// Type의 객체를 생성하고, 그 객체에 대한 포인터를 반환하는 함수 템플릿
template<typename Type, typename...Args>
Type* xnew(Args&&... args)
{
    // Type의 객체를 저장할 수 있는 메모리를 할당합니다.
    Type* memory = static_cast<Type*>(BaseAllocator::Alloc(sizeof(Type)));

    // placement new를 이용하여 메모리에 Type의 객체를 생성합니다.
    new(memory)Type(forward<Args>(args)...);
    return memory;
}

// obj가 가리키는 객체를 소멸시키고, 그 객체의 메모리를 해제하는 함수 템플릿
template<typename Type>
void xdelete(Type* obj)
{
    // obj가 가리키는 객체의 소멸자를 호출합니다.
    obj->~Type();
    // 객체의 메모리를 해제합니다.
    BaseAllocator::Release(obj);
}