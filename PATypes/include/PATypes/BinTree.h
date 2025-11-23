#pragma once

#include "ComplexHack.h"
#include "PairTuple.h"
#include "Sequence.h"
#include <ctime>
#include <random>
#include <string>

namespace PATypes {
enum SearchType { KLP, KPL, LPK, LKP, PLK, PKL };
template <class T> class BinaryTreeNode {
    T val;
    int priority;
    bool ltag = false, rtag = false;
    BinaryTreeNode<T> *l = nullptr, *r = nullptr, *parent = nullptr;

  public:
    BinaryTreeNode(T val, int priority, BinaryTreeNode<T> *l = nullptr,
                   BinaryTreeNode<T> *r = nullptr,
                   BinaryTreeNode<T> *parent = nullptr)
        : val(val), priority(priority), l(l), r(r), parent(parent) {};
    BinaryTreeNode(const BinaryTreeNode<T> &node);
    BinaryTreeNode(bool (*f)(T), const BinaryTreeNode<T> &node);
    ~BinaryTreeNode() {
        if (l != nullptr)
            delete l;
        if (r != nullptr)
            delete r;
    }
    void map(T (*f)(T));
    BinaryTreeNode<T> *getLeft() const {
        if (!ltag)
            return l;
        else
            return nullptr;
    }
    BinaryTreeNode<T> *getLeftT() const { return l; }
    BinaryTreeNode<T> *getRight() const {
        if (!rtag)
            return r;
        else
            return nullptr;
    }
    BinaryTreeNode<T> *getRightT() const { return r; }
    BinaryTreeNode<T> **getLeftPtr() {
        if (!ltag)
            return &l;
        else
            return nullptr;
    }
    BinaryTreeNode<T> **getRightPtr() {
        if (!rtag)
            return &r;
        else
            return nullptr;
    }
    BinaryTreeNode<T> *getParent() const { return parent; }
    void setLeft(BinaryTreeNode<T> *n) {
        this->l = n;
        ltag = false;
    }
    void setRight(BinaryTreeNode<T> *n) {
        this->r = n;
        rtag = false;
    }
    void setParent(BinaryTreeNode<T> *n) { this->parent = n; }

    void setLeftT(BinaryTreeNode<T> *n) {
        this->l = n;
        ltag = true;
    }
    void setRightT(BinaryTreeNode<T> *n) {
        this->r = n;
        rtag = true;
    }

    int getPriority() { return priority; }
    T getVal() const { return val; }
};

template <class T> class BinaryTree {
    BinaryTreeNode<T> *root = nullptr;
    std::mt19937 mt;

    BinaryTreeNode<T> *_merge(BinaryTreeNode<T> *l, BinaryTreeNode<T> *r);
    Pair<BinaryTreeNode<T> *, BinaryTreeNode<T> *>
    _split(BinaryTreeNode<T> *root, T key);

    BinaryTreeNode<T> *_search(BinaryTreeNode<T> *node, T key,
                               BinaryTreeNode<T> *parent = nullptr);
    void _erase(BinaryTreeNode<T> **current, BinaryTreeNode<T> *toErase);
    void _insertAll(const BinaryTreeNode<T> *node);
    void _insertAllWhere(std::function<bool(T)> f, const BinaryTreeNode<T> *node);

    size_t _size(BinaryTreeNode<T> *current) {
        if (current == nullptr)
            return 0;
        return 1ll + _size(current->getLeft()) + _size(current->getRight());
    }

    MutableListSequence<T> _KLP(BinaryTreeNode<T> *current);
    MutableListSequence<T> _KPL(BinaryTreeNode<T> *current);
    MutableListSequence<T> _LPK(BinaryTreeNode<T> *current);
    MutableListSequence<T> _LKP(BinaryTreeNode<T> *current);
    MutableListSequence<T> _PLK(BinaryTreeNode<T> *current);
    MutableListSequence<T> _PKL(BinaryTreeNode<T> *current);

    void _threadKLP(BinaryTreeNode<T> *current, BinaryTreeNode<T> *lParent,
                    BinaryTreeNode<T> *rParent);
    void _threadKPL(BinaryTreeNode<T> *current, BinaryTreeNode<T> *lParent,
                    BinaryTreeNode<T> *rParent);
    void _threadLPK(BinaryTreeNode<T> *current, BinaryTreeNode<T> *lParent,
                    BinaryTreeNode<T> *rParent);
    void _threadLKP(BinaryTreeNode<T> *current, BinaryTreeNode<T> *lParent,
                    BinaryTreeNode<T> *rParent);
    void _threadPLK(BinaryTreeNode<T> *current, BinaryTreeNode<T> *lParent,
                    BinaryTreeNode<T> *rParent);
    void _threadPKL(BinaryTreeNode<T> *current, BinaryTreeNode<T> *lParent,
                    BinaryTreeNode<T> *rParent);

  public:
    BinaryTree() : root(nullptr), mt(1337) {};
    BinaryTree(const BinaryTree<T> &tree);
    BinaryTree(const BinaryTreeNode<T> &root);
    BinaryTree(std::function<bool(T)> f, BinaryTree<T> tree);
    ~BinaryTree() { delete root; }
    void merge(BinaryTree<T> tree);
    BinaryTree<T> *map(T (*f)(T));

    void insert(T key);
    void erase(BinaryTreeNode<T> *node);
    BinaryTreeNode<T> *getRoot() { return root; };
    BinaryTreeNode<T> *findElement(T val);
    BinaryTree<T> getSubTree(BinaryTreeNode<T> *current);
    bool treesEqual(BinaryTreeNode<T> *tree1, BinaryTreeNode<T> *tree2);

    size_t getSize() { return _size(root); }

    BinaryTreeNode<T> *subTree(BinaryTree<T> tree);

    // std::string KLP(std::string delimiter);
    // std::string KPL(std::string delimiter);
    // std::string LPK(std::string delimiter);
    // std::string LKP(std::string delimiter);
    // std::string PLK(std::string delimiter);
    // std::string PKL(std::string delimiter);

    Sequence<T> *getSearch(SearchType search);

    std::string toString(std::string search);

    void thread(SearchType type);

    void threadKLP() { _threadKLP(root, nullptr, nullptr); }
    void threadKPL() { _threadKPL(root, nullptr, nullptr); }
    void threadLPK() { _threadLPK(root, nullptr, nullptr); }
    void threadLKP() { _threadLKP(root, nullptr, nullptr); }
    void threadPKL() { _threadPKL(root, nullptr, nullptr); }
    void threadPLK() { _threadPLK(root, nullptr, nullptr); }

    BinaryTree &operator=(const BinaryTree &other) {
        this->root = new BinaryTreeNode<T>(*other.root);
        return *this;
    }
};

template <class T>
BinaryTreeNode<T>::BinaryTreeNode(const BinaryTreeNode<T> &node)
    : val(node.val) {
    priority = node.priority;
    if (node.l != nullptr) {
        this->l = new BinaryTreeNode<T>(*node.getLeft());
    }
    if (node.r != nullptr) {
        this->r = new BinaryTreeNode<T>(*node.getRight());
    }
}

template <class T>
BinaryTreeNode<T>::BinaryTreeNode(bool (*f)(T), const BinaryTreeNode<T> &node) {
    _insertAllWhere(f, &node);
}

template <class T>
void BinaryTree<T>::_insertAll(const BinaryTreeNode<T> *node) {
    if (node != nullptr)
        this->insert(node->getVal());
    if (node->getLeft() != nullptr)
        _insertAll(node->getLeft());
    if (node->getRight() != nullptr)
        _insertAll(node->getRight());
}

template <class T>
void BinaryTree<T>::_insertAllWhere(std::function<bool(T)> f,
                                    const BinaryTreeNode<T> *node) {
    if (node != nullptr && f(node->getVal()))
        this->insert(node->getVal());
    if (node->getLeft())
        _insertAll(node->getLeft());
    if (node->getRight())
        _insertAll(node->getRight());
}

template <class T> void BinaryTreeNode<T>::map(T (*f)(T)) {
    val = f(val);
    if (l != nullptr)
        l->map(f);
    if (r != nullptr)
        r->map(f);
}

template <class T>
BinaryTree<T>::BinaryTree(const BinaryTreeNode<T> &root) : mt(time(nullptr)) {
    this->root = new BinaryTreeNode<T>(root);
}

template <class T>
BinaryTree<T>::BinaryTree(std::function<bool(T)> f, BinaryTree<T> tree)
    : mt(time(nullptr)) {
    this->_insertAllWhere(f, tree.root);
}

template <class T> BinaryTree<T>::BinaryTree(const BinaryTree<T> &tree) {
    root = new BinaryTreeNode<T>(*tree.root);
}

template <class T> BinaryTree<T> *BinaryTree<T>::map(T (*f)(T)) {
    BinaryTree<T> *newTree = new BinaryTree<T>(*root);
    newTree->root->map(f);
    return newTree;
}

template <class T> void BinaryTree<T>::merge(BinaryTree<T> tree) {
    _insertAll(tree.root);
}

template <class T>
BinaryTreeNode<T> *BinaryTree<T>::_merge(BinaryTreeNode<T> *l,
                                         BinaryTreeNode<T> *r) {
    if (l == nullptr)
        return r;
    if (r == nullptr)
        return l;
    if (l->getPriority() > r->getPriority()) {
        l->setRight(_merge(l->getRight(), r));
        return l;
    } else {
        r->setLeft(_merge(l, r->getLeft()));
        return r;
    }
}

template <class T>
Pair<BinaryTreeNode<T> *, BinaryTreeNode<T> *>
BinaryTree<T>::_split(BinaryTreeNode<T> *first, T key) {
    if (first == nullptr) {
        return Pair<BinaryTreeNode<T> *, BinaryTreeNode<T> *>(nullptr, nullptr);
    }
    if (first->getVal() <= key) {
        Pair<BinaryTreeNode<T> *, BinaryTreeNode<T> *> second =
            _split(first->getRight(), key);
        first->setRight(second.getFirst());
        return Pair<BinaryTreeNode<T> *, BinaryTreeNode<T> *>(
            first, second.getSecond());
    } else {
        Pair<BinaryTreeNode<T> *, BinaryTreeNode<T> *> second =
            _split(first->getLeft(), key);
        first->setLeft(second.getSecond());
        return Pair<BinaryTreeNode<T> *, BinaryTreeNode<T> *>(second.getFirst(),
                                                              first);
    }
}

template <class T> void BinaryTree<T>::erase(BinaryTreeNode<T> *node) {
    _erase(&root, node);
    node->setLeft(nullptr);
    node->setRight(nullptr);
    delete node;
}

template <class T>
void BinaryTree<T>::_erase(BinaryTreeNode<T> **current,
                           BinaryTreeNode<T> *toErase) {
    if (*current == nullptr) {
        return;
    }
    if (*current == toErase) {
        *current = _merge(toErase->getLeft(), toErase->getRight());
    } else if ((*current)->getLeft() == toErase) {
        (*current)->setLeft(_merge(toErase->getLeft(), toErase->getRight()));
    } else if ((*current)->getRight() == toErase) {
        (*current)->setRight(_merge(toErase->getLeft(), toErase->getRight()));
    } else {
        _erase(((*current)->getLeftPtr()), toErase);
        _erase(((*current)->getRightPtr()), toErase);
    }
}

template <class T> void BinaryTree<T>::insert(T key) {
    Pair<BinaryTreeNode<T> *, BinaryTreeNode<T> *> second = _split(root, key);
    BinaryTreeNode<T> *newNode = new BinaryTreeNode<T>(key, mt());
    root = _merge(second.getFirst(), _merge(newNode, second.getSecond()));
}

template <class T>
BinaryTreeNode<T> *BinaryTree<T>::_search(BinaryTreeNode<T> *node, T key,
                                          BinaryTreeNode<T> *parent) {
    if (node == nullptr) {
        return node;
    } else if (key == node->getVal()) {
        node->setParent(parent);
        return node;
    }
    if (key < node->getVal()) {
        return _search(node->getLeft(), key, node);
    } else {
        return _search(node->getRight(), key, node);
    }
}

template <class T> BinaryTreeNode<T> *BinaryTree<T>::findElement(T val) {
    return _search(root, val);
}

template <class T>
BinaryTree<T> BinaryTree<T>::getSubTree(BinaryTreeNode<T> *node) {
    return BinaryTreeNode<T>(*node);
}

template <class T>
bool BinaryTree<T>::treesEqual(BinaryTreeNode<T> *tree1,
                               BinaryTreeNode<T> *tree2) {
    if (tree1 == nullptr && tree2 == nullptr)
        return true;
    else if (tree1 == nullptr || tree2 == nullptr)
        return false;
    return tree1->getVal() == tree2->getVal() &&
           treesEqual(tree1->getLeft(), tree2->getLeft()) &&
           treesEqual(tree1->getRight(), tree2->getRight());
}

template <class T>
BinaryTreeNode<T> *BinaryTree<T>::subTree(BinaryTree<T> tree) {
    BinaryTreeNode<T> *index = this->_search(tree.root, tree.root->getVal());
    if (treesEqual(index, tree.root))
        return index;
    else
        return nullptr;
}

template <class T>
MutableListSequence<T> BinaryTree<T>::_KLP(BinaryTreeNode<T> *current) {
    MutableListSequence<T> result(current->getVal());
    if (current->getLeft() != nullptr) {
        MutableListSequence<T> left = _KLP(current->getLeft());
        result.concat(&left);
    }
    if (current->getRight() != nullptr) {
        MutableListSequence<T> right = _KLP(current->getRight());
        result.concat(&right);
    }
    return result;
}

template <class T>
MutableListSequence<T> BinaryTree<T>::_KPL(BinaryTreeNode<T> *current) {
    MutableListSequence<T> result(current->getVal());
    if (current->getRight() != nullptr) {
        MutableListSequence<T> right = _KPL(current->getRight());
        result.concat(&right);
    }
    if (current->getLeft() != nullptr) {
        MutableListSequence<T> left = _KPL(current->getLeft());
        result.concat(&left);
    }
    return result;
}

template <class T>
MutableListSequence<T> BinaryTree<T>::_LPK(BinaryTreeNode<T> *current) {
    MutableListSequence<T> result;
    if (current->getLeft() != nullptr) {
        MutableListSequence<T> left = _LPK(current->getLeft());
        result.concat(&left);
    }
    if (current->getRight() != nullptr) {
        MutableListSequence<T> right = _LPK(current->getRight());
        result.concat(&right);
    }
    result += current->getVal();
    return result;
}

template <class T>
MutableListSequence<T> BinaryTree<T>::_LKP(BinaryTreeNode<T> *current) {
    MutableListSequence<T> result;
    if (current->getLeft() != nullptr) {
        MutableListSequence<T> left = _LKP(current->getLeft());
        result.concat(&left);
    }
    result += current->getVal();
    if (current->getRight() != nullptr) {
        MutableListSequence<T> right = _LKP(current->getRight());
        result.concat(&right);
    }
    return result;
}

template <class T>
MutableListSequence<T> BinaryTree<T>::_PKL(BinaryTreeNode<T> *current) {
    MutableListSequence<T> result;
    if (current->getRight() != nullptr) {
        MutableListSequence<T> right = _PKL(current->getRight());
        result.concat(&right);
    }
    result += current->getVal();
    if (current->getLeft() != nullptr) {
        MutableListSequence<T> left = _PKL(current->getLeft());
        result.concat(&left);
    }
    return result;
}

template <class T>
MutableListSequence<T> BinaryTree<T>::_PLK(BinaryTreeNode<T> *current) {
    MutableListSequence<T> result;
    if (current->getRight() != nullptr) {
        MutableListSequence<T> right = _PLK(current->getRight());
        result.concat(&right);
    }
    if (current->getLeft() != nullptr) {
        MutableListSequence<T> left = _PLK(current->getLeft());
        result.concat(&left);
    }
    result += current->getVal();
    return result;
}

template <class T>
void BinaryTree<T>::_threadKLP(BinaryTreeNode<T> *current,
                               BinaryTreeNode<T> *lParent,
                               BinaryTreeNode<T> *rParent) {
    if (current == nullptr)
        return;
    if (lParent != nullptr && current->getLeftT() == nullptr)
        current->setLeftT(lParent);
    if (rParent != nullptr && current->getRightT() == nullptr)
        current->setRightT(lParent);
    _threadKLP(current->getLeft(), current, nullptr);
    _threadKLP(current->getRight(), nullptr, current);
}

template <class T>
void BinaryTree<T>::_threadKPL(BinaryTreeNode<T> *current,
                               BinaryTreeNode<T> *lParent,
                               BinaryTreeNode<T> *rParent) {
    if (current == nullptr)
        return;
    if (lParent != nullptr && current->getLeftT() == nullptr)
        current->setLeftT(lParent);
    if (rParent != nullptr && current->getRightT() == nullptr)
        current->setRightT(lParent);
    _threadKPL(current->getRight(), nullptr, current);
    _threadKPL(current->getLeft(), current, nullptr);
}

template <class T>
void BinaryTree<T>::_threadLPK(BinaryTreeNode<T> *current,
                               BinaryTreeNode<T> *lParent,
                               BinaryTreeNode<T> *rParent) {
    _threadLPK(current->getLeft(), current, nullptr);
    _threadLPK(current->getRight(), nullptr, current);
    if (current == nullptr)
        return;
    if (lParent != nullptr && current->getLeftT() == nullptr)
        current->setLeftT(lParent);
    if (rParent != nullptr && current->getRightT() == nullptr)
        current->setRightT(lParent);
}

template <class T>
void BinaryTree<T>::_threadLKP(BinaryTreeNode<T> *current,
                               BinaryTreeNode<T> *lParent,
                               BinaryTreeNode<T> *rParent) {
    _threadLKP(current->getLeft(), current, nullptr);
    if (current == nullptr)
        return;
    if (lParent != nullptr && current->getLeftT() == nullptr)
        current->setLeftT(lParent);
    if (rParent != nullptr && current->getRightT() == nullptr)
        current->setRightT(lParent);
    _threadLKP(current->getRight(), nullptr, current);
}

template <class T>
void BinaryTree<T>::_threadPLK(BinaryTreeNode<T> *current,
                               BinaryTreeNode<T> *lParent,
                               BinaryTreeNode<T> *rParent) {
    _threadPLK(current->getRight(), nullptr, current);
    _threadPLK(current->getLeft(), current, nullptr);
    if (current == nullptr)
        return;
    if (lParent != nullptr && current->getLeftT() == nullptr)
        current->setLeftT(lParent);
    if (rParent != nullptr && current->getRightT() == nullptr)
        current->setRightT(lParent);
}

template <class T>
void BinaryTree<T>::_threadPKL(BinaryTreeNode<T> *current,
                               BinaryTreeNode<T> *lParent,
                               BinaryTreeNode<T> *rParent) {
    _threadPLK(current->getRight(), nullptr, current);
    if (current == nullptr)
        return;
    if (lParent != nullptr && current->getLeftT() == nullptr)
        current->setLeftT(lParent);
    if (rParent != nullptr && current->getRightT() == nullptr)
        current->setRightT(lParent);
    _threadPLK(current->getLeft(), current, nullptr);
}

template <class T> void BinaryTree<T>::thread(SearchType type) {
    switch (type) {
    case KLP:
        _threadKLP(root, nullptr, nullptr);
        break;
    case KPL:
        _threadKPL(root, nullptr, nullptr);
        break;
    case LPK:
        _threadLPK(root, nullptr, nullptr);
        break;
    case LKP:
        _threadLKP(root, nullptr, nullptr);
        break;
    case PLK:
        _threadPLK(root, nullptr, nullptr);
        break;
    case PKL:
        _threadPKL(root, nullptr, nullptr);
        break;
    }
}

template <class T> Sequence<T> *BinaryTree<T>::getSearch(SearchType search) {
    MutableListSequence<T> result;
    switch (search) {
    case KLP:
        result = _KLP(root);
        break;
    case KPL:
        result = _KPL(root);
        break;
    case LPK:
        result = _LPK(root);
        break;
    case LKP:
        result = _LKP(root);
        break;
    case PLK:
        result = _PLK(root);
        break;
    case PKL:
        result = _PKL(root);
        break;
    }
    return new ImmutableListSequence<T>(result);
}
} // namespace PATypes