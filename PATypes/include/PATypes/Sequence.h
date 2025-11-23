#pragma once

#include "DynamicArray.h"
#include "ICollection.h"
#include "LinkedList.h"

namespace PATypes {
template <class T> class Sequence : public ICollection<T>, IEnumerable<T> {
  public:
    virtual ~Sequence() {};
    virtual T getFirst() = 0;
    virtual T getLast() = 0;
    virtual int getLength() = 0;
    virtual Sequence<T> *getSubsequence(int startIndex, int endIndex) = 0;
    virtual Sequence<T> *append(T item) = 0;
    virtual Sequence<T> *insertAt(T item, int index) = 0;
    virtual Sequence<T> *concat(Sequence<T> *list) = 0;
    virtual Sequence<T> *map(T (*f)(T)) = 0;
    virtual T reduce(T (*f)(T, T), T c) = 0;
    virtual T operator[](int index) = 0;
    Sequence<T> &operator+(Sequence<T> &sequence);
    Sequence<T> &operator+(T item);
    Sequence<T> &operator+=(T item);
    virtual IEnumerator<T> *getEnumerator() = 0;
};

template <class T> T _reduce(T (*f)(T, T), Sequence<T> *list, T c) {
    if (list->getLength() == 1) {
        return (*f)(list->getFirst(), c);
    } else {
        auto subSequence = list->getSubsequence(0, list->getLength() - 2);
        T result = (*f)(list->getLast(), _reduce(f, subSequence, c));
        delete subSequence;
        return result;
    }
}

template <class T> Sequence<T> &Sequence<T>::operator+(Sequence<T> &sequence) {
    return *(this->concat(&sequence));
}

template <class T> Sequence<T> &Sequence<T>::operator+(T item) {
    return *(this->append(item));
}

template <class T> Sequence<T> &Sequence<T>::operator+=(T item) {
    return *(this->append(item));
}

template <class T> class ArraySequence : public Sequence<T> {
  public:
    ArraySequence(T *items, int count) : array(items, count) {}
    ArraySequence() : array(0) {};
    ArraySequence(const ArraySequence &arraySequence)
        : array(arraySequence.array) {};
    ArraySequence(Sequence<T> &sequence);
    ArraySequence(T item) : array(&item, 1) {}
    ArraySequence(size_t size) : array(size) {}
    virtual ~ArraySequence() {};
    virtual T getFirst();
    virtual T getLast();
    virtual T get(int index);
    virtual T get(size_t index);
    virtual size_t getCount();
    virtual int getLength();
    virtual Sequence<T> *getSubsequence(int startIndex, int endIndex) = 0;
    virtual Sequence<T> *append(T item);
    virtual Sequence<T> *insertAt(T item, int index);
    virtual Sequence<T> *concat(Sequence<T> *list);
    virtual Sequence<T> *map(T (*f)(T));
    virtual T reduce(T (*f)(T, T), T c) {
        return _reduce(f, (Sequence<T> *)this, c);
    }
    T operator[](int index);
    ArraySequence<T> &operator=(const ArraySequence<T> &other);
    ArraySequence<T> &operator=(const ArraySequence<T> &&other);
    virtual IEnumerator<T> *getEnumerator() { return array.getEnumerator(); }

  protected:
    DynamicArray<T> array;
    virtual ArraySequence<T> *Instance() = 0;
    virtual ArraySequence<T> *Clone() = 0;
};

template <class T> T ArraySequence<T>::getFirst() {
    if (array.getSize() < 1)
        throw std::out_of_range(
            "при попытке получения первого элемента ArraySequence пустой");
    return array[0];
}

template <class T> T ArraySequence<T>::getLast() {
    if (array.getSize() < 1)
        throw std::out_of_range(
            "при попытке получения последнего элемента ArraySequence пустой");
    return array[array.getSize() - 1];
}

template <class T> T ArraySequence<T>::get(int index) {
    if (index < 0 || index >= array.getSize())
        throw std::out_of_range("при попытке получения элемента по индексу "
                                "ArraySequence индекс за границами");
    return array[index];
}

template <class T> int ArraySequence<T>::getLength() { return array.getSize(); }

template <class T> Sequence<T> *ArraySequence<T>::append(T item) {
    ArraySequence<T> *current = Instance();
    current->array.resize(array.getSize() + 1);
    current->array.set(array.getSize() - 1, item);
    return current;
}

template <class T> Sequence<T> *ArraySequence<T>::insertAt(T item, int index) {
    if (index < 0 || index >= array.getSize())
        throw std::out_of_range("при попытке вставки элемента по индексу "
                                "ArraySequence индекс за границами");
    ArraySequence<T> *current = Instance();
    current->array.resize(array.getSize() + 1);
    for (int i = array.getSize() - 1; i > index; --i) {
        current->array[i] = current->array[i - 1];
    }
    current->array[index] = item;
    return current;
}

template <class T> Sequence<T> *ArraySequence<T>::concat(Sequence<T> *list) {
    ArraySequence<T> *current = Instance();
    int previousLength = current->getLength();
    DynamicArray<T> *array =
        new DynamicArray<T>(previousLength + list->getLength(), this->array);
    current->array = *array;
    delete array;
    for (int i = 0; i < list->getLength(); ++i)
        current->array.set(i + previousLength, list->get(i));
    return current;
}

template <class T> T ArraySequence<T>::operator[](int index) {
    return get(index);
}

template <class T> T ArraySequence<T>::get(size_t index) {
    return get((int)index);
}

template <class T> size_t ArraySequence<T>::getCount() {
    return (size_t)array.getSize();
}

template <class T> Sequence<T> *ArraySequence<T>::map(T (*f)(T)) {
    ArraySequence<T> *current = Instance();
    for (int i = 0; i < current->getLength(); ++i)
        current->array.set(i, f(current->array.get(i)));
    return current;
}

template <class T>
ArraySequence<T>::ArraySequence(Sequence<T> &sequence)
    : array(sequence.getLength()) {
    for (int i = 0; i < sequence.getLength(); ++i) {
        array.set(i, sequence.get(i));
    }
}

template <class T>
ArraySequence<T> &ArraySequence<T>::operator=(const ArraySequence<T> &other) {
    this->array = DynamicArray<T>(other.array);
    return *this;
}

template <class T>
ArraySequence<T> &ArraySequence<T>::operator=(const ArraySequence<T> &&other) {
    this->array = DynamicArray<T>(other.array);
    return *this;
}

template <class T> class ImmutableArraySequence : public ArraySequence<T> {
  public:
    ImmutableArraySequence(T *items, int count)
        : ArraySequence<T>(items, count) {}
    ImmutableArraySequence() : ArraySequence<T>() {};
    ImmutableArraySequence(Sequence<T> &sequence)
        : ArraySequence<T>(sequence) {}
    ImmutableArraySequence(size_t size) : ArraySequence<T>(size) {};
    ImmutableArraySequence(T item) : ArraySequence<T>(item) {}
    virtual PATypes::Sequence<T> *getSubsequence(int startIndex, int endIndex);

  protected:
    ArraySequence<T> *Instance();
    ArraySequence<T> *Clone();
};

template <class T>
Sequence<T> *ImmutableArraySequence<T>::getSubsequence(int startIndex,
                                                       int endIndex) {
    if (startIndex < 0 || startIndex >= this->getLength() || endIndex < 0 ||
        endIndex >= this->getLength())
        throw std::out_of_range("Индекс за границами при попытке получения "
                                "подпоследовательности ImmutableArraySequence");
    ImmutableArraySequence<T> *current =
        new ImmutableArraySequence<T>((size_t)endIndex - startIndex + 1);
    for (int i = startIndex; i <= endIndex; ++i) {
        current->array.set(i, this->array.get(i));
    }
    return current;
}

template <class T> ArraySequence<T> *ImmutableArraySequence<T>::Instance() {
    return new ImmutableArraySequence(*this);
}

template <class T> ArraySequence<T> *ImmutableArraySequence<T>::Clone() {
    return new ImmutableArraySequence(*this);
}

template <class T> class MutableArraySequence : public ArraySequence<T> {
  public:
    MutableArraySequence(T *items, int count)
        : ArraySequence<T>(items, count) {}
    MutableArraySequence() : ArraySequence<T>() {};
    MutableArraySequence(Sequence<T> &sequence) : ArraySequence<T>(sequence) {}
    MutableArraySequence(size_t size) : ArraySequence<T>(size) {}
    MutableArraySequence(T item) : ArraySequence<T>(item) {}
    PATypes::Sequence<T> *getSubsequence(int startIndex, int endIndex);

  protected:
    ArraySequence<T> *Instance();
    ArraySequence<T> *Clone();
};

template <class T>
Sequence<T> *MutableArraySequence<T>::getSubsequence(int startIndex,
                                                     int endIndex) {
    if (startIndex < 0 || startIndex >= this->getLength() || endIndex < 0 ||
        endIndex >= this->getLength())
        throw std::out_of_range("Индекс за границами при попытке получения "
                                "подпоследовательности ImmutableArraySequence");
    MutableArraySequence<T> *current =
        new MutableArraySequence<T>((size_t)endIndex - startIndex + 1);
    for (int i = startIndex; i <= endIndex; ++i) {
        current->array.set(i, this->array.get(i));
    }
    return current;
}

template <class T> ArraySequence<T> *MutableArraySequence<T>::Instance() {
    return this;
}

template <class T> ArraySequence<T> *MutableArraySequence<T>::Clone() {
    return new MutableArraySequence(*this);
}

template <class T> class ListSequence : public Sequence<T> {
  public:
    ListSequence(T *items, int count) : list(items, count) {};
    ListSequence() : list() {};
    ListSequence(const ListSequence<T> &listSequence)
        : list(listSequence.list) {};
    ListSequence(Sequence<T> &sequence);
    ListSequence(T item) : list(&item, 1) {};
    virtual ~ListSequence() {};
    T getFirst();
    T getLast();
    T get(int index);
    int getLength();
    virtual T get(size_t index);
    virtual size_t getCount();
    virtual Sequence<T> *getSubsequence(int startIndex, int endIndex) = 0;
    virtual Sequence<T> *append(T item);
    virtual Sequence<T> *insertAt(T item, int index);
    virtual Sequence<T> *concat(Sequence<T> *list) = 0;
    Sequence<T> *concat(ListSequence<T> *list);
    virtual Sequence<T> *map(T (*f)(T));
    virtual T reduce(T (*f)(T, T), T c) { return _reduce(f, Instance(), c); }
    T operator[](int index);
    ListSequence<T> &operator=(const ListSequence<T> &other);
    ListSequence<T> &operator=(const ListSequence<T> &&other);
    IEnumerator<T> *getEnumerator() { return list.getEnumerator(); }

  protected:
    LinkedList<T> list;
    virtual ListSequence<T> *Instance() = 0;
};

template <class T> T ListSequence<T>::getFirst() { return list.getFirst(); }

template <class T> T ListSequence<T>::getLast() { return list.getLast(); }
template <class T> T ListSequence<T>::get(int index) { return list.get(index); }
template <class T> Sequence<T> *ListSequence<T>::append(T item) {
    ListSequence<T> *current = Instance();
    current->list.append(item);
    return current;
}

template <class T> int ListSequence<T>::getLength() { return list.getLength(); }

template <class T> Sequence<T> *ListSequence<T>::insertAt(T item, int index) {
    if (index < 0 || index > list.getLength())
        throw std::out_of_range(
            "при попытке вставить в ListSequence индекс за границами");
    ListSequence<T> *current = Instance();
    try {
        current->list.insertAt(item, index);
        return current;
    } catch (std::out_of_range &) {
        throw std::out_of_range(
            "при попытке вставить в ListSequence индекс за границами");
    }
    return current;
}

template <class T> Sequence<T> *ListSequence<T>::concat(ListSequence<T> *list) {
    ListSequence<T> *current = Instance();
    list = current->list.concat(list);
    return current;
}

template <class T> T ListSequence<T>::operator[](int index) {
    return get(index);
}

template <class T> T ListSequence<T>::get(size_t index) {
    return get((int)index);
}

template <class T> size_t ListSequence<T>::getCount() {
    return (size_t)getLength();
}

template <class T> Sequence<T> *ListSequence<T>::map(T (*f)(T)) {
    ListSequence<T> *current = Instance();
    current->list.map(f);
    return current;
}
template <class T>
ListSequence<T>::ListSequence(Sequence<T> &sequence) : list() {
    for (int i = 0; i < sequence.getLength(); ++i) {
        list.append(sequence.get(i));
    }
}

template <class T>
ListSequence<T> &ListSequence<T>::operator=(const ListSequence<T> &other) {
    this->list = LinkedList<T>(other.list);
    return *this;
}

template <class T>
ListSequence<T> &ListSequence<T>::operator=(const ListSequence<T> &&other) {
    this->list = LinkedList<T>(other.list);
    return *this;
}

template <class T> class ImmutableListSequence : public ListSequence<T> {
  public:
    ImmutableListSequence(T *items, int count)
        : ListSequence<T>(items, count) {}
    ImmutableListSequence(Sequence<T> &sequence) : ListSequence<T>(sequence) {}
    ImmutableListSequence() : ListSequence<T>() {}
    ImmutableListSequence(T item) : ListSequence<T>(item) {}
    virtual Sequence<T> *concat(Sequence<T> *list);
    virtual PATypes::Sequence<T> *getSubsequence(int startIndex, int endIndex);

  protected:
    virtual ListSequence<T> *Instance();
};

template <class T> ListSequence<T> *ImmutableListSequence<T>::Instance() {
    return new ImmutableListSequence(*this);
}

template <class T>
Sequence<T> *ImmutableListSequence<T>::getSubsequence(int startIndex,
                                                      int endIndex) {
    if (startIndex < 0 || startIndex >= this->getLength() || endIndex < 0 ||
        endIndex >= this->getLength())
        throw std::out_of_range("Индекс за границами при попытке получения "
                                "подпоследовательности ImmutableListSequence");
    ImmutableListSequence<T> *current = new ImmutableListSequence<T>();
    auto sublist = this->list.getSubList(startIndex, endIndex);
    current->list = *sublist;
    delete sublist;
    return current;
}

template <class T>
Sequence<T> *ImmutableListSequence<T>::concat(Sequence<T> *list) {
    Sequence<T> *current = Instance();
    Sequence<T> *old;
    for (int i = 0; i < list->getLength(); ++i) {
        old = current;
        current = current->append(list->get(i));
        delete old;
    }
    return current;
}

template <class T> class MutableListSequence : public ListSequence<T> {
  public:
    MutableListSequence(T *items, int count) : ListSequence<T>(items, count) {};
    MutableListSequence(Sequence<T> &sequence) : ListSequence<T>(sequence) {};
    MutableListSequence() : ListSequence<T>() {};
    MutableListSequence(T item) : ListSequence<T>(item) {};
    virtual Sequence<T> *concat(Sequence<T> *list);
    virtual PATypes::Sequence<T> *getSubsequence(int startIndex, int endIndex);

  protected:
    virtual ListSequence<T> *Instance();
};

template <class T> ListSequence<T> *MutableListSequence<T>::Instance() {
    return this;
}

template <class T>
Sequence<T> *MutableListSequence<T>::getSubsequence(int startIndex,
                                                    int endIndex) {
    if (startIndex < 0 || startIndex >= this->getLength() || endIndex < 0 ||
        endIndex >= this->getLength())
        throw std::out_of_range("Индекс за границами при попытке получения "
                                "подпоследовательности MutableListSequence");
    MutableListSequence<T> *current = new MutableListSequence<T>();
    auto sublist = this->list.getSubList(startIndex, endIndex);
    current->list = *sublist;
    delete sublist;
    return current;
}

template <class T>
Sequence<T> *MutableListSequence<T>::concat(Sequence<T> *list) {
    Sequence<T> *current = Instance();
    for (int i = 0; i < list->getLength(); ++i) {
        current = current->append(list->get(i));
    }
    return current;
}

}; // namespace PATypes
