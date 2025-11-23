#pragma once

#include <exception>
#include <functional>
#include <optional>

#include "PATypes/Sequence.h"
#include "PATypes/Set.h"

namespace PATypes {
class Cardinal {
    size_t a;
    bool infinite;
    Cardinal(bool inf) : a(0), infinite(inf) {}

  public:
    Cardinal(size_t a) : a(a), infinite(false) {}
    Cardinal(const Cardinal &other) : a(other.a), infinite(other.a) {}
    Cardinal() : a(0), infinite(true) {}
    std::optional<size_t> Get() {
        if (infinite)
            return std::nullopt;
        else
            return a;
    }
    static Cardinal Infinity() { return Cardinal(); }
    bool operator==(const Cardinal &other) const {
        if (infinite && other.infinite)
            return true;
        if (infinite || other.infinite)
            return false;
        return a == other.a;
    }

    bool operator<(const Cardinal &other) const {
        if (infinite)
            return false;
        if (other.infinite)
            return true;
        return a < other.a;
    }
};

static Cardinal Infinity = Cardinal();

template <class T> class LazySequence {

    class LazyStorage {
        int index;
        T val;
        friend LazySequence;

      public:
        LazyStorage(int index, T val) : index(index), val(val) {}
        int getIndex() { return index; }
        T getValue() { return val; }
        int operator<(const LazyStorage &b) const { return index < b.index; }
        int operator<=(const LazyStorage &b) const { return index <= b.index; }
        int operator==(const LazyStorage &b) const { return index == b.index; }
    };
    std::function<T(Sequence<T> *)> rule;
    // На самом деле словарь (map)
    Set<LazyStorage> storage;

    bool IsInfinite = false;
    size_t stride;
    int leftmost_index;
    int rightmost_index;
    void _clearStorage(int index) {
        std::function<bool(LazyStorage)> clearFunction = [index](const LazyStorage &a) {
            return a.index < index;
        };
        storage = Set<LazyStorage>(clearFunction, storage);
    }
    bool IsCalculated(int index) { return storage.contains({index, T()}); }
    T Calculate(int index) {
        if (IsCalculated(index)) {
            return storage.getByItem({index, 0}).getValue();
        } else {
            if (!IsInfinite &&
                (index < leftmost_index || index > rightmost_index)) {
                throw std::out_of_range("outside of finite sequence");
            }
            MutableListSequence<T> toCalculate;
            for (size_t i = 0; i < stride; ++i) {
                toCalculate.append(Calculate(index - i - 1));
            }
            T newValue = rule(&toCalculate);
            storage.insert({index, newValue});
            return newValue;
        }
    }

    void _Append(T item) {
        ++rightmost_index;
        storage.insert({rightmost_index, item});
    }

    void _Prepend(T item) {
        --leftmost_index;
        storage.insert({leftmost_index, item});
    }

  public:
    LazySequence()
        : rule(nullptr), storage(), leftmost_index(0), rightmost_index(0) {}

    LazySequence(const T *items, int count)
        : rule(nullptr), storage(), leftmost_index(0),
          rightmost_index(count - 1) {
        for (int i = 0; i < count; ++i) {
            storage.insert({i, items[i]});
        }
    }

    LazySequence(Sequence<T> *seq)
        : rule(nullptr), leftmost_index(0),
          rightmost_index(seq->getLength() - 1) {
        auto *enumerator = seq->getEnumerator();
        int i = 0;
        while (enumerator->moveNext()) {
            storage.insert({i, enumerator->current()});
            ++i;
        }
        delete enumerator;
    }

    LazySequence(T (*rule)(Sequence<T> *), Sequence<T> *initial, size_t count)
        : rule(rule), IsInfinite(true), stride(count), leftmost_index(0),
          rightmost_index(0) {
        auto *enumerator = initial->getEnumerator();
        int i = 0;
        while (enumerator->moveNext()) {
            storage.insert({i, enumerator->current()});
            ++i;
        }
        delete enumerator;
    }
    LazySequence(const LazySequence<T> &copy)
        : rule(copy.rule), storage(copy.storage), IsInfinite(copy.IsInfinite),
          stride(copy.stride), leftmost_index(copy.leftmost_index),
          rightmost_index(copy.rightmost_index) {}
    T GetFirst() { return storage.getByItem({leftmost_index, 0}).getValue(); }
    T GetLast() {
        if (GetLength().Get() == std::nullopt) {
            throw std::out_of_range(
                "attempt to get last element of infinite sequence");
        } else {
            return storage.getByItem({rightmost_index, 0}).getValue();
        }
    }
    T Get(int index) { return Calculate(index); }
    LazySequence<T> *GetSubSequence(int startindex, int endindex);
    Cardinal GetLength() const {
        if (IsInfinite) {
            return Cardinal();
        } else {
            return Cardinal((size_t)rightmost_index - leftmost_index + 1);
        }
    }
    size_t GetMaterializedCount() const { return storage.GetLength(); }

    int GetLeftmostIndex() const {
        return leftmost_index;
    }

    int GetRightmostIndex() const {
        return rightmost_index;
    }
    
    LazySequence<T> *Append(T item) const {
        LazySequence<T> *newSeq = new LazySequence<T>(*this);
        ++(newSeq->rightmost_index);
        newSeq->storage.insert({newSeq->rightmost_index, item});
        return newSeq;
    }
    LazySequence<T> *Prepend(T item) const {
        LazySequence<T> *newSeq = new LazySequence<T>(*this);
        --(newSeq->leftmost_index);
        newSeq->storage.insert({newSeq->leftmost_index, item});
        return newSeq;
    }
    LazySequence<T> *InsertAt(T item, int index) const {
        LazySequence<T> *newSeq = new LazySequence<T>(*this);
        if (rule != nullptr)
            newSeq->_clearStorage(index);
        newSeq->storage.erase({index, T(0)});
        newSeq->storage.insert({index, item});
        return newSeq;
    }
    LazySequence<T> *Concat(LazySequence<T> &list) {
        LazySequence<T> *newSequence;
        if (this->IsInfinite)
            newSequence = new LazySequence<T>(*this);
        else {
            newSequence = new LazySequence<T>(*this);
            newSequence->rule = list.rule;
            newSequence->stride = list.stride;
            newSequence->IsInfinite = list.IsInfinite;
            if (list.IsInfinite) {
                for (int i = list.leftmost_index;
                     i < list.leftmost_index + list.stride; ++i) {
                    newSequence->_Append(list.Get(i));
                }
            } else {
                for (int i = list.leftmost_index; i < list.rightmost_index;
                     ++i) {
                    newSequence->_Append(list.Get(i));
                }
            }
        }
        return newSequence;
    }

    LazySequence<T> *Map(T (*func)(T)) {
        LazySequence<T> *seq;
        if (IsInfinite) {
            std::function<T(Sequence<T> *)> newRule = [&](Sequence<T> *in) {
                return func(rule(in));
            };
            seq = new LazySequence<T>(*this);
            seq->storage = Set<LazyStorage>();
            for (int i = leftmost_index; i <= leftmost_index + stride; ++i) {
                seq->storage.insert({i, func(storage.getByItem({i, T(0)}))});
            }
        } else {
            seq = new LazySequence<T>(this);
            for (int i = leftmost_index; i <= rightmost_index; ++i) {
                seq->storage.insert({i, func(storage.getByItem({i, T(0)}))});
            }
        }
        return seq;
    }

    template <class T2> T2 Reduce(T2 (*func)(T2, T), T2 C) {
        if (IsInfinite)
            throw std::logic_error("unable to reduce infinite sequence");
        T2 res = func(C, Get(leftmost_index));
        for (int i = leftmost_index + 1; i <= rightmost_index; ++i) {
            res = func(res, Get(i));
        }

        return res;
    }

    LazySequence<T> &operator=(const LazySequence<T> &other) {
        this->storage = other.storage;
        this->rule = other.rule;
        this->IsInfinite = other.IsInfinite;
        this->stride = other.stride;
        this->leftmost_index = other.leftmost_index;
        this->rightmost_index = other.rightmost_index;
        return *this;
    }
};
} // namespace PATypes