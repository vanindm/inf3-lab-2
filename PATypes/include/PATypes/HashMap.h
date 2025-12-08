#pragma once

#include <functional>
#include <memory>

#include "IMap.h"
#include "Sequence.h"
#include "PairTuple.h"

const size_t HASHMAP_MOD = (1 << 10);

namespace PATypes {
template <class K, class V> class HashMap : IMap<K, V> {
    size_t mod;
    struct HashMapNode {
        K key;
        V value;
        std::shared_ptr<HashMapNode> next;
    };
    DynamicArray<std::shared_ptr<HashMapNode>> storage;

  public:
    HashMap(size_t mod = HASHMAP_MOD) : mod(mod), storage(mod) {}
    template <class V2>
    std::shared_ptr<HashMap<K, V2>> map(std::function<V2(V)> &func) {
        std::shared_ptr<HashMap<K, V2>> newMap =
            std::make_shared<HashMap<K, V2>>(mod);
        std::shared_ptr<Sequence<V>> all = GetAll();
        auto allEnumerator = all->getEnumerator();
        while (allEnumerator->moveNext()) {
            newMap->Set(allEnumerator->current().key,
                        func(allEnumerator->current().value));
        }
        delete allEnumerator;
    }
    virtual void Add(K key, V value) {
        size_t hash = std::hash<K>{}(key) % mod;
        std::shared_ptr<HashMapNode> current = storage.get(hash);
        if (current == nullptr) {
            storage.set(hash,
                        std::make_shared<HashMapNode>(key, value, nullptr));
        } else {
            while (current->next != nullptr) {
                if (current->key == key) {
                    current->value = value;
                    return;
                }
                current = current->next;
            }
            current->next = std::make_shared<HashMapNode>(key, value, nullptr);
        }
    }
    virtual V Get(K key) const {
        size_t hash = std::hash<K>{}(key) % mod;
        std::shared_ptr<HashMapNode> current = storage.get(hash);
        if (current == nullptr)
            throw std::out_of_range(
                "попытка найти элемент, не лежащий в HashMap");
        while (current->next != nullptr) {
            if (current->key == key) {
                return current->value;
            }
            current = current->next;
        }
        if (current->key == key) {
            return current->value;
        }
        throw std::out_of_range("попытка найти элемент, не лежащий в HashMap");
    }
    virtual void Delete(K key) {
        size_t hash = std::hash<K>{}(key) % mod;
        std::shared_ptr<HashMapNode> current = storage.get(hash);
        if (current->key == key) {
            storage.set(hash, current->next);
        }
        std::shared_ptr<HashMapNode> prev = current;
        current = current->next;
        while (current->next != nullptr) {
            if (current->key == key) {
                prev->next = current->next;
            }
            prev = current;
            current = current->next;
        }
    }
    virtual void Clear() {
        storage = DynamicArray<std::shared_ptr<HashMapNode>>(mod);
    }

    virtual std::shared_ptr<Sequence<V>> GetAll() {
        std::shared_ptr<MutableListSequence<V>> result =
            std::make_shared<MutableListSequence<V>>();
        for (size_t i = 0; i < mod; ++i) {
            std::shared_ptr<HashMapNode> current = storage.get(i);
            if (current != nullptr) {
                result->append(current->value);
                while (current->next != nullptr) {
                    current = current->next;
                    result->append(current->value);
                }
            }
        }
        return result;
    }

    virtual std::shared_ptr<Sequence<Pair<K, V>>> GetAllPairs() {
        std::shared_ptr<MutableListSequence<Pair<K, V>>> result =
            std::make_shared<MutableListSequence<Pair<K, V>>>();
        for (size_t i = 0; i < mod; ++i) {
            std::shared_ptr<HashMapNode> current = storage.get(i);
            if (current != nullptr) {
                result->append(Pair<K,V>(current->key, current->value));
                while (current->next != nullptr) {
                    current = current->next;
                    result->append(Pair<K,V>(current->key, current->value));
                }
            }
        }
        return result;
    }
};
} // namespace PATypes