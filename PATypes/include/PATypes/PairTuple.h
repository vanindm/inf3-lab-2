#pragma once

#include "DynamicArray.h"

namespace PATypes {
template <class T, class U> class Pair {
  public:
    Pair(T first, U second) : first(first), second(second) {};
    Pair(Pair &pair) : first(pair.getFirst()), second(pair.getSecond()) {};
    T &getFirst();
    U &getSecond();

  private:
    T first;
    U second;
};

template <class T, class U> T &Pair<T, U>::getFirst() { return first; }

template <class T, class U> U &Pair<T, U>::getSecond() { return second; }

template <class T, size_t size> class Tuple {
  public:
    Tuple(T newItems[size]) : items(newItems, size) {};
    Tuple(Tuple<T, size> &tuple) : items(tuple.items) {};
    T get(int i);

  private:
    DynamicArray<T> items;
};

template <class T, size_t size> T Tuple<T, size>::get(int i) {
    if (i < 0 || i >= size)
        throw std::out_of_range(
            "Попытка обращения к элементу за границами Tuple");
    return items[i];
}
} // namespace PATypes