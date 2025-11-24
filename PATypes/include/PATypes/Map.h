#pragma once

#include "IMap.h"
#include "Set.h"
#include <functional>

namespace PATypes {
template <class K, class V, class H = std::hash<K>> class Map : IMap<K, V> {
    class MapNode {
        size_t key;
        V val;

      public:
        MapNode(K key, V val) : key(std::hash<K>{}(key)), val(val) {}
        MapNode(size_t keyHash, V val, bool workaround) : key(keyHash), val(val) {}
        size_t getKeyHash() { return key; }
        V getValue() { return val; }
        int operator<(const MapNode &b) const { return key < b.key; }
        int operator<=(const MapNode &b) const { return key <= b.key; }
        int operator==(const MapNode &b) const { return key == b.key; }
    };
    Set<MapNode> storage;

  public:
    Map() {}
    virtual V Get(K key) const {
        try {
            return storage.getByItem({H{}(key), {}, 1}).getValue();
        } catch (std::out_of_range &err) {
            throw std::out_of_range(
                "попытка найти элемент, не лежащий в ассоциативном массиве");
        }
    }
    virtual void Add(K index, V value) {
        try {
            storage.erase({H{}(index), {}, 1});
        } catch (std::logic_error &) {
        }
        storage.insert({H{}(index), value, 1});
    }
    virtual void Delete(K key) { storage.erase({H{}(key), {}, 1}); }
    virtual void Clear() { storage = Set<MapNode>(); }
};

}; // namespace PATypes