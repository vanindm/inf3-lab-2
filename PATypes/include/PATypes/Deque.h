#pragma once

#include "Sequence.h"

namespace PATypes {
template <class T> class Deque : Sequence<T> {
    MutableListSequence<T> *list;

  public:
    Deque();
    virtual ~Deque();
    void push_back(const T &element);
    void push_front(const T &element);
    T pop_back();
    T pop_front();
    virtual T getFirst();
    virtual T getLast();
    virtual T get(size_t index);
    virtual T get(int index);
    virtual int getLength();
    virtual size_t getCount();
    virtual Sequence<T> *getSubsequence(int startIndex, int endIndex);
    virtual Sequence<T> *append(T item);
    virtual Sequence<T> *insertAt(T item, int index);
    virtual Sequence<T> *concat(Sequence<T> *list);
    virtual Sequence<T> *map(T (*f)(T));
    virtual T reduce(T (*f)(T, T), T c);
    virtual T operator[](int index);
    Sequence<T> &operator+(Sequence<T> &sequence);
    virtual IEnumerator<T> *getEnumerator();
    Sequence<int> *findSequence(Sequence<T> *toFind);
};

template <class T> Deque<T>::Deque() { list = new MutableListSequence<T>(); }

template <class T> Deque<T>::~Deque() { delete list; }

template <class T> T Deque<T>::get(size_t index) { return list->get(index); }

template <class T> size_t Deque<T>::getCount() { return list->getCount(); }

template <class T> T Deque<T>::get(int index) { return list->get(index); }

template <class T> void Deque<T>::push_back(const T &element) {
    list->append(element);
}

template <class T> void Deque<T>::push_front(const T &element) {
    list->insertAt(element, 0);
}

template <class T> T Deque<T>::pop_back() {
    if (list->getLength() == 0)
        throw std::out_of_range(
            "при попытке забрать элемент из Deque Deque пуст");
    T res = list->getLast();
    if (list->getLength() > 1) {
        MutableListSequence<T> *newList =
            (MutableListSequence<T> *)(list->getSubsequence(
                0, list->getLength() - 2));
        delete list;
        list = newList;
    } else {
        delete list;
        list = new MutableListSequence<T>();
    }
    return res;
}

template <class T> T Deque<T>::pop_front() {
    if (list->getLength() == 0)
        throw std::out_of_range(
            "при попытке забрать элемент из Deque Deque пуст");
    T res = list->getFirst();
    if (list->getLength() > 1) {
        MutableListSequence<T> *newList =
            (MutableListSequence<T> *)(list->getSubsequence(
                1, list->getLength() - 1));
        delete list;
        list = newList;
    } else {
        delete list;
        list = new MutableListSequence<T>();
    }
    return res;
}

template <class T> T Deque<T>::getFirst() { return list->getFirst(); }

template <class T> T Deque<T>::getLast() { return list->getLast(); }

template <class T> int Deque<T>::getLength() { return list->getLength(); }

template <class T>
Sequence<T> *Deque<T>::getSubsequence(int startIndex, int endIndex) {
    return list->getSubsequence(startIndex, endIndex);
}

template <class T> Sequence<T> *Deque<T>::append(T item) {
    return list->append(item);
}

template <class T> Sequence<T> *Deque<T>::insertAt(T item, int index) {
    return list->insertAt(item, index);
}

template <class T> Sequence<T> *Deque<T>::concat(Sequence<T> *list) {
    return this->list->concat(list);
}

template <class T> Sequence<T> *Deque<T>::map(T (*f)(T)) {
    return list->map(f);
}

template <class T> T Deque<T>::operator[](int index) {
    return list->get(index);
}

template <class T> Sequence<T> &Deque<T>::operator+(Sequence<T> &sequence) {
    return (*list) + sequence;
}

template <class T> IEnumerator<T> *Deque<T>::getEnumerator() {
    return list->getEnumerator();
}

template <class T> Sequence<int> *Deque<T>::findSequence(Sequence<T> *toFind) {
    int toFindIndex = 0;
    int stackIndex = 0;
    MutableArraySequence<int> *indexes =
        new MutableArraySequence<int>((size_t)0);
    int length = toFind->getLength();
    try {
        while (toFindIndex < length) {
            if (toFind->get(toFindIndex) == this->get(stackIndex)) {
                indexes->append(toFindIndex);
                ++toFindIndex;
            }
            ++stackIndex;
        }
    } catch (std::out_of_range &) {
        throw std::out_of_range("последовательность не содержится");
    }
    return indexes;
}

template <class T> T Deque<T>::reduce(T (*f)(T, T), T c) {
    return list->reduce(f, c);
}

} // namespace PATypes
