#pragma once

#include "IEnumerator.h"
#include "Sequence.h"

namespace PATypes {

template <class T> Sequence<T> *map(T (*f)(T), Sequence<T> *list) {
    return list->map(f);
}

template <class T> Sequence<T> *where(Sequence<T> *list, bool (*f)(T)) {
    IEnumerator<T> *enumerator = list->getEnumerator();
    MutableListSequence<T> result = new ImmutableListSequence<T>();
    bool isNotCompleted = true;
    while (isNotCompleted) {
        T &current = enumerator->current();
        if (f(current)) {
            result.append(current);
        }
        isNotCompleted = enumerator->next();
    }
    return result;
}
}; // namespace PATypes