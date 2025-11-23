#pragma once

#include "BinTree.h"
#include "ICollection.h"
#include "PairTuple.h"

namespace PATypes {
template <class T> class Set : public ICollection<T> {
    BinaryTree<T> tree;

  public:
    Set() : tree() {}
    ~Set() {}
    Set(const BinaryTree<T> &tree) : tree(tree) {}
    Set(const Set<T> &set) : tree(set.tree) {}
    // Set(Sequence<T> *sequence) : tree() {
    // }
    Set(std::function<bool(T)> f, Set<T> &set) { tree = BinaryTree<T>(f, tree); }
    void insert(T item) { tree.insert(item); }
    void erase(T item) {
        BinaryTreeNode<T> *element = tree.findElement(item);
        if (element) {
            tree.erase(element);
            return;
        }
        throw std::logic_error(
            "попытка удалить элемент, не лежащий в множестве");
    }
    void map(T (*f)(T)) { tree.map(f); }
    T getByItem(T item) { return (tree.findElement(item))->getVal(); }
    BinaryTreeNode<T> *getNodeByItem(T item) {
        return (tree.findElement(item));
    }
    bool contains(T item) { return tree.findElement(item) != nullptr; }
    virtual T get(size_t index);
    virtual size_t getCount() { return tree.getSize(); };
    void unite(Set<T> &set2) { tree.merge(set2.tree); }
    void intersect(Set<T> &set2) {
        size_t treeSize = getCount();
        for (size_t i = 0; i < treeSize; ++i) {
            T element = get(i);
            if (!set2.contains(element))
                erase(element);
        }
    }
    void difference(Set<T> &set2) {
        size_t treeSize = set2.getCount();
        for (size_t i = 0; i < treeSize; ++i) {
            T element = set2.get(i);
            if (contains(element))
                erase(element);
        }
    }
    bool hasSubSet(Set<T> &set2) {
        bool result = true;
        size_t treeSize = set2.getCount();
        for (size_t i = 0; i < treeSize && result; ++i) {
            T element = set2.get(i);
            result &= contains(element);
        }
        return result;
    }
    bool equal(Set<T> &set2) {
        bool result = (getCount() == set2.getCount());
        size_t treeSize = getCount();
        for (size_t i = 0; result && i < treeSize; ++i) {
            T element = set2.get(i);
            result &= contains(element);
        }
        return result;
    }
    Set<T> &operator=(const Set<T> &other) {
        this->tree = other.tree;
        return *this;
    }
};

template <class T> T Set<T>::get(size_t index) {
    Sequence<T> *result = tree.getSearch(KLP);
    T resultValue = result->get(index);
    delete result;
    return resultValue;
}
}; // namespace PATypes