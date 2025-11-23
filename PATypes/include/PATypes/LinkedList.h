#pragma once

namespace PATypes {

template <class T> class LinkedListNode {
  public:
    LinkedListNode(T val, LinkedListNode *next = nullptr);
    LinkedListNode *getNext();
    ~LinkedListNode();
    T &get();
    void set(T value);
    void setNext(LinkedListNode *newNext);
    LinkedListNode<T> &operator=(const LinkedListNode<T> &node);

  private:
    T value;
    LinkedListNode *next;
};

template <class T> class LinkedList : IEnumerable<T> {
  public:
    LinkedList(T *items, int count);
    LinkedList();
    LinkedList(LinkedListNode<T> *start, int count);
    LinkedList(const LinkedList<T> &list);
    virtual ~LinkedList();
    T getFirst();
    T getLast();
    T get(int index);
    LinkedList<T> *getSubList(int startIndex, int endIndex);
    int getLength();
    void append(T item);
    void prepend(T item);
    void insertAt(T item, int index);
    void removeAt(int index);
    LinkedList<T> *concat(LinkedList<T> *list);
    void map(T (*f)(T));
    PATypes::LinkedList<T> &operator=(const LinkedList<T> &array);
    IEnumerator<T> *getEnumerator() { return new Enumerator(*this); }

  private:
    class Enumerator : public IEnumerator<T> {
        bool isFirst = true;

      public:
        Enumerator(LinkedList<T> &parent) : parent(parent), ptr(parent.head) {}
        Enumerator(LinkedList<T> &parent, LinkedListNode<T> *ptr)
            : parent(parent), ptr(ptr) {}
        virtual ~Enumerator() {}

        virtual bool moveNext() {
            if (ptr->getNext() == nullptr) {
                return 0;
            }
            if (isFirst) {
                isFirst = false;
            } else {
                ptr = ptr->getNext();
            }
            return 1;
        }

        virtual T &current() { return (ptr->get()); }

        virtual void reset() {
            isFirst = true;
            ptr = parent.head;
        }

      private:
        LinkedList<T> &parent;
        LinkedListNode<T> *ptr;
    };
    int size;
    LinkedListNode<T> *head;
    LinkedListNode<T> &getNode(int index);
    LinkedListNode<T> &getFirstNode();
    LinkedListNode<T> &getLastNode();
};

}; // namespace PATypes

template <class T>
PATypes::LinkedListNode<T>::LinkedListNode(T val, LinkedListNode *next)
    : value(val), next(next) {}

template <class T> PATypes::LinkedListNode<T>::~LinkedListNode() {
    if (this->next)
        delete this->next;
}

template <class T>
PATypes::LinkedListNode<T> *PATypes::LinkedListNode<T>::getNext() {
    return this->next;
}

template <class T> T &PATypes::LinkedListNode<T>::get() { return this->value; }

template <class T> void PATypes::LinkedListNode<T>::set(T value) {
    this->value = value;
}

template <class T>
void PATypes::LinkedListNode<T>::setNext(PATypes::LinkedListNode<T> *newNext) {
    this->next = newNext;
}

template <class T>
PATypes::LinkedListNode<T> &
PATypes::LinkedListNode<T>::operator=(const PATypes::LinkedListNode<T> &node) {
    this->value = node.value;
    return *this;
}

template <class T>
PATypes::LinkedList<T>::LinkedList(T *items, int count) : size(count) {
    this->head = new PATypes::LinkedListNode<T>(items[0]);
    PATypes::LinkedListNode<T> *current = this->head;
    PATypes::LinkedListNode<T> *intermediate = nullptr;
    for (int i = 1; i < count; ++i) {
        intermediate = new PATypes::LinkedListNode<T>(items[i]);
        current->setNext(intermediate);
        current = current->getNext();
    }
}

template <class T> PATypes::LinkedList<T>::LinkedList() {
    this->size = 0;
    this->head = nullptr;
}

template <class T>
PATypes::LinkedList<T>::LinkedList(PATypes::LinkedListNode<T> *start, int count)
    : size(count) {
    this->head = new PATypes::LinkedListNode<T>(start->get());
    PATypes::LinkedListNode<T> *current = this->head;
    PATypes::LinkedListNode<T> *currentSource = start;
    PATypes::LinkedListNode<T> *intermediate = nullptr;
    for (int i = 1; i < count; ++i) {
        if (current == nullptr)
            throw std::out_of_range("Выход за границы при попытке создания "
                                    "LinkedList из LinkedListNode");
        intermediate =
            new PATypes::LinkedListNode<T>(currentSource->getNext()->get());
        current->setNext(intermediate);
        current = current->getNext();
    }
}

template <class T>
PATypes::LinkedList<T>::LinkedList(const LinkedList<T> &list)
    : size(list.size) {
    if (list.head == nullptr) {
        this->head = nullptr;
        return;
    }
    this->head = new PATypes::LinkedListNode<T>(list.head->get());
    PATypes::LinkedListNode<T> *current = this->head;
    PATypes::LinkedListNode<T> *intermediate = nullptr;
    PATypes::LinkedListNode<T> *currentSource = list.head;
    while (currentSource->getNext() != nullptr) {
        intermediate =
            new PATypes::LinkedListNode<T>(currentSource->getNext()->get());
        current->setNext(intermediate);
        currentSource = currentSource->getNext();
        current = current->getNext();
    }
}

template <class T> PATypes::LinkedList<T>::~LinkedList() { delete this->head; }

template <class T>
PATypes::LinkedListNode<T> &PATypes::LinkedList<T>::getNode(int index) {
    if (index < 0 || index >= this->size)
        throw std::out_of_range(
            "Выход за границы при попытке получения LinkedListNode по индексу");
    PATypes::LinkedListNode<T> *current = this->head;
    for (int i = 0; i < index; i++) {
        current = current->getNext();
        if (current == nullptr)
            throw std::out_of_range("Выход за границы при попытке получения "
                                    "LinkedListNode по индексу");
    }
    return *current;
}

template <class T>
PATypes::LinkedListNode<T> &PATypes::LinkedList<T>::getFirstNode() {
    if (this->head == nullptr)
        throw std::out_of_range(
            "при попытке получения начала списка LinkedList пуст");
    return *this->head;
}

template <class T>
PATypes::LinkedListNode<T> &PATypes::LinkedList<T>::getLastNode() {
    PATypes::LinkedListNode<T> *current = this->head;
    if (current == nullptr)
        throw std::out_of_range(
            "при попытке получения конца списка LinkedList пуст");
    while (current->getNext() != nullptr) {
        current = current->getNext();
    }
    return *current;
}

template <class T> T PATypes::LinkedList<T>::getFirst() {
    return this->getFirstNode().get();
}

template <class T> T PATypes::LinkedList<T>::getLast() {
    return this->getLastNode().get();
}

template <class T> T PATypes::LinkedList<T>::get(int index) {
    try {
        return this->getNode(index).get();
    } catch (std::out_of_range &) {
        throw std::out_of_range("Выход за границы при попытке получения "
                                "элемента LinkedList по индексу");
    }
}

template <class T>
PATypes::LinkedList<T> *PATypes::LinkedList<T>::getSubList(int startIndex,
                                                           int endIndex) {
    if (startIndex < 0 || startIndex >= this->size || endIndex < 0 ||
        endIndex >= this->size)
        throw std::out_of_range(
            "Выход за границы при попытке получения subList LinkedList");
    PATypes::LinkedList<T> *newList = new PATypes::LinkedList<T>();
    PATypes::LinkedListNode<T> *current = this->head;
    for (int i = 0; i < startIndex; i++) {
        current = current->getNext();
        if (current == nullptr)
            throw std::out_of_range(
                "Выход за границы при попытке получения subList LinkedList");
    }
    for (int i = startIndex; i <= endIndex; i++) {
        if (current == nullptr)
            throw std::out_of_range(
                "Выход за границы при попытке получения subList LinkedList");
        newList->append(current->get());
        current = current->getNext();
    }
    return newList;
}

template <class T> int PATypes::LinkedList<T>::getLength() {
    return this->size;
}

template <class T> void PATypes::LinkedList<T>::append(T item) {
    this->size++;
    if (this->head)
        this->getLastNode().setNext(new LinkedListNode<T>(item));
    else
        this->head = new LinkedListNode<T>(item);
}

template <class T> void PATypes::LinkedList<T>::prepend(T item) {
    this->size++;
    PATypes::LinkedListNode<T> *newHead = new PATypes::LinkedListNode<T>(item);
    newHead->setNext(this->head);
    this->head = newHead;
}

template <class T> void PATypes::LinkedList<T>::insertAt(T item, int index) {
    if (getLength() == 0) {
        this->append(item);
    } else if (index == getLength()) {
        PATypes::LinkedListNode<T> *newNode =
            new PATypes::LinkedListNode<T>(item);
        this->getNode(index - 1).setNext(newNode);
        ++this->size;
    } else {
        try {
            PATypes::LinkedListNode<T> *newNode =
                new PATypes::LinkedListNode<T>(item);
            newNode->setNext(&this->getNode(index));
            if (index == 0)
                this->head = newNode;
            if (index > 0)
                this->getNode(index - 1).setNext(newNode);
            ++this->size;
        } catch (std::out_of_range &) {
            throw std::out_of_range("Выход за границы при попытке получения "
                                    "добавить к LinkedList элемент по индексу");
        }
    }
}

template <class T> void PATypes::LinkedList<T>::removeAt(int index) {
    if (index < 0 || index >= this->size)
        throw std::out_of_range(
            "Выход за границы при попытке удаления элемента LinkedList");
    LinkedListNode<T> *node = this->head;
    for (int i = 0; i < index - 1; ++i) {
        node = node->getNext();
    }
    node->setNext(node->getNext()->getNext());
    --this->size;
    delete node;
    return;
}

template <class T>
PATypes::LinkedList<T> *
PATypes::LinkedList<T>::concat(PATypes::LinkedList<T> *list) {
    PATypes::LinkedListNode<T> *current = &(this->getLastNode());
    PATypes::LinkedListNode<T> *currentOriginal = &(list->getFirstNode());
    current->setNext(new PATypes::LinkedListNode<T>(currentOriginal->get()));
    current = current->getNext();
    while (currentOriginal->getNext()) {
        current->setNext(
            new PATypes::LinkedListNode<T>(currentOriginal->getNext()->get()));
        current = current->getNext();
        currentOriginal = currentOriginal->getNext();
    }
    this->size += list->size;
    return this;
}

template <class T> void PATypes::LinkedList<T>::map(T (*f)(T)) {
    PATypes::LinkedListNode<T> *current = &getFirstNode();
    current->set(f(current->get()));
    current = current->getNext();
    while (current) {
        current->set(f(current->get()));
        current = current->getNext();
    }
}

template <class T>
PATypes::LinkedList<T> &
PATypes::LinkedList<T>::operator=(const PATypes::LinkedList<T> &list) {
    if (list.head != nullptr)
        this->head = new PATypes::LinkedListNode<T>(list.head->get());
    this->size = list.size;
    PATypes::LinkedListNode<T> *current = this->head;
    PATypes::LinkedListNode<T> *intermediate = nullptr;
    PATypes::LinkedListNode<T> *currentSource = list.head;
    while (currentSource != nullptr && currentSource->getNext() != nullptr) {
        intermediate =
            new PATypes::LinkedListNode<T>(currentSource->getNext()->get());
        current->setNext(intermediate);
        current = current->getNext();
        currentSource = currentSource->getNext();
    }
    return *this;
}
