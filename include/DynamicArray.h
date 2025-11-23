#pragma once

#include "SmartPointers/MemorySpan.h"
#include "SmartPointers/ShrdPtr.h"
#include "SmartPointers/SmartPtr.h"

template <class T> class SmartPtrDynamicArray {
    SmartPointers::SmartPtr<T> *pointers;

  public:
    SmartPtrDynamicArray(size_t size, T *list) {
        pointers = new SmartPointers::SmartPtr<T>[size];
        for (size_t i = 0; i < size; ++i) {
            pointers[i] = SmartPointers::SmartPtr<T>(list[i]);
        }
    }
    ~SmartPtrDynamicArray() { delete[] pointers; }

    T &getElement(size_t index) { return *(pointers[index]); }
};

template <class T> class UnqPtrDynamicArray {
    SmartPointers::UnqPtr<T> *pointers;

  public:
    UnqPtrDynamicArray(size_t size, T *list) {
        pointers = new SmartPointers::UnqPtr<T>[size];
        for (size_t i = 0; i < size; ++i) {
            pointers[i] = SmartPointers::UnqPtr<T>(list[i]);
        }
    }
    ~UnqPtrDynamicArray() { delete[] pointers; }

    T &getElement(size_t index) { return *(pointers[index]); }
};

template <class T> class MemorySpanDynamicArray {
    SmartPointers::MemorySpan<T> span;

  public:
    MemorySpanDynamicArray(size_t size, T *list) : span(size, list) {}

    T &getElement(size_t index) { return *(span.Locate(index)); }

    auto begin() { return span.Locate(0); }
};