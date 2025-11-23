#pragma once

#include "DynamicArray.h"
#include "LinkedList.h"

namespace PATypes {
template <class T> class SegmentedList : IEnumerable<T> {
    int size;
    LinkedListNode<DynamicArray<T>> *head;
    LinkedListNode<DynamicArray<T>> &getNode(int index);
    LinkedListNode<DynamicArray<T>> &getFirstNode();
    LinkedListNode<DynamicArray<T>> &getLastNode();

  public:
    SegmentedList(T *items, int count);
    SegmentedList();
    SegmentedList(LinkedListNode<DynamicArray<T>> *start, int count);
    SegmentedList(const SegmentedList<T> &list);
    ~SegmentedList();
    T getFirst();
    T getLast();
    T get(int index);
    SegmentedList<T> *getSubList(int startIndex, int endIndex);
    int getLength();
    void append(T item);
    void prepend(T item);
    void insertAt(T item, int index);
    SegmentedList<T> *concat(SegmentedList<T> *list);
    void map(T (*f)(T));
    SegmentedList<T> &operator=(const SegmentedList<T> &array);
    IEnumerator<T> *getEnumerator() { return new Enumerator(*this); }
};

template <class T>
SegmentedList<T>::SegmentedList(T *items, int count) : size(count) {
    if (count >= 21) {
        this->head = new LinkedListNode<DynamicArray<T>>(DynamicArray(T, 21));
        LinkedListNode<DynamicArray<T>> *current = this->head;
        LinkedListNode<DynamicArray<T>> *intermediate = nullptr;

        for (int i = 1; i < count / 21; ++i) {
            if (count - i * 21 >= 21)
                intermediate = new LinkedListNode<DynamicArray<T>>(
                    DynamicArray(T + i * 21), 21);
            else
                intermediate = new LinkedListNode<DynamicArray<T>>(
                    DynamicArray(T + i * 21), count - i * 21);
            current->setNext(intermediate);
            current = current->getNext();
        }
    } else
        this->head =
            new LinkedListNode<DynamicArray<T>>(DynamicArray(T, count));
}

template <class T> SegmentedList<T>::SegmentedList() {
    this->size = 0;
    this->head = nullptr;
}

template <class T>
SegmentedList<T>::SegmentedList(LinkedListNode<DynamicArray<T>> *start,
                                int count)
    : size(count) {
    this->head = new LinkedListNode<DynamicArray<T>>(start->get());
    LinkedListNode<DynamicArray<T>> *current = this->head;
    LinkedListNode<DynamicArray<T>> *currentSource = start;
    LinkedListNode<DynamicArray<T>> *intermediate = nullptr;
    for (int i = 1; i < count; ++i) {
        if (current == nullptr)
            throw std::out_of_range("Выход за границы при попытке создания "
                                    "LinkedList из LinkedListNode");
        intermediate = new LinkedListNode<DynamicArray<T>>(
            currentSource->getNext()->get());
        current->setNext(intermediate);
        current = current->getNext();
    }
}

template <class T>
SegmentedList<T>::SegmentedList(const SegmentedList<T> &list)
    : size(list.size) {
    this->head = new LinkedListNode<DynamicArray<T>>(list.head->get());
    LinkedListNode<DynamicArray<T>> *current = this->head;
    LinkedListNode<DynamicArray<T>> *intermediate = nullptr;
    LinkedListNode<DynamicArray<T>> *currentSource = list.head;
    while (currentSource->getNext() != nullptr) {
        intermediate = new LinkedListNode<DynamicArray<T>>(
            currentSource->getNext()->get());
        current->setNext(intermediate);
        currentSource = currentSource->getNext();
        current = current->getNext();
    }
}

template <class T> SegmentedList<T>::~SegmentedList() { delete this->head; }

template <class T>
LinkedListNode<DynamicArray<T>> &SegmentedList<T>::getNode(int index) {
    if (index < 0 || index >= this->size)
        throw std::out_of_range(
            "Выход за границы при попытке получения LinkedListNode по индексу");
    LinkedListNode<DynamicArray<T>> *current = this->head;
    for (int i = 0; i < index; i++) {
        current = current->getNext();
        if (current == nullptr)
            throw std::out_of_range("Выход за границы при попытке получения "
                                    "LinkedListNode по индексу");
    }
    return *current;
}

template <class T>
LinkedListNode<DynamicArray<T>> &SegmentedList<T>::getFirstNode() {
    if (this->head == nullptr)
        throw std::out_of_range(
            "при попытке получения начала списка LinkedList пуст");
    return *this->head;
}

template <class T>
LinkedListNode<DynamicArray<T>> &SegmentedList<T>::getLastNode() {
    LinkedListNode<DynamicArray<T>> *current = this->head;
    if (current == nullptr)
        throw std::out_of_range(
            "при попытке получения конца списка LinkedList пуст");
    while (current->getNext() != nullptr) {
        current = current->getNext();
    }
    return *current;
}

template <class T> T SegmentedList<T>::getFirst() {
    return this->getFirstNode().get();
}

template <class T> T SegmentedList<T>::getLast() {
    return this->getLastNode().get();
}

template <class T> T SegmentedList<T>::get(int index) {
    try {
        return this->getNode(index).get();
    } catch (std::out_of_range &) {
        throw std::out_of_range("Выход за границы при попытке получения "
                                "элемента LinkedList по индексу");
    }
}

template <class T>
SegmentedList<T> *SegmentedList<T>::getSubList(int startIndex, int endIndex) {}

template <class T> int SegmentedList<T>::getLength() { return this->size; }

template <class T> void SegmentedList<T>::append(T item) {
    this->size++;
    if (this->head)
        this->getLastNode().setNext(new LinkedListNode<T>(item));
    else
        this->head = new LinkedListNode<T>(item);
}

template <class T> void SegmentedList<T>::prepend(T item) {
    this->size++;
    LinkedListNode<T> *newHead = new LinkedListNode<T>(item);
    newHead->setNext(this->head);
    this->head = newHead;
}

template <class T> void SegmentedList<T>::insertAt(T item, int index) {
    try {
        LinkedListNode<T> &newNode(item);
        newNode->setNext(this->getNode(index));
        if (index > 0)
            this->getNode(index - 1).setNext(newNode);
    } catch (std::out_of_range &) {
        throw std::out_of_range("Выход за границы при попытке получения "
                                "добавить к LinkedList элемент по индексу");
    }
}

template <class T>
SegmentedList<T> *SegmentedList<T>::concat(SegmentedList<T> *list) {
    LinkedListNode<T> *current = &(this->getLastNode());
    LinkedListNode<T> *currentOriginal = &(list->getFirstNode());
    current->setNext(new LinkedListNode<T>(currentOriginal->get()));
    current = current->getNext();
    while (currentOriginal->getNext()) {
        current->setNext(
            new LinkedListNode<T>(currentOriginal->getNext()->get()));
        current = current->getNext();
        currentOriginal = currentOriginal->getNext();
    }
    this->size += list->size;
    return this;
}

template <class T> void SegmentedList<T>::map(T (*f)(T)) {}

template <class T>
SegmentedList<T> &SegmentedList<T>::operator=(const SegmentedList<T> &list) {
    this->head = new LinkedListNode<T>(list.head->get());
    this->size = list.size;
    LinkedListNode<T> *current = this->head;
    LinkedListNode<T> *intermediate = nullptr;
    LinkedListNode<T> *currentSource = list.head;
    while (currentSource->getNext() != nullptr) {
        intermediate = new LinkedListNode<T>(currentSource->getNext()->get());
        current->setNext(intermediate);
        current = current->getNext();
        currentSource = currentSource->getNext();
    }
    return *this;
}

} // namespace PATypes