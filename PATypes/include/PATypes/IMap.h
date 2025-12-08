#pragma once

#include <memory>
#include "Sequence.h"

namespace PATypes {
template <class K, class V> class IMap {
  public:
    virtual V Get(K key) const = 0;
    // why
    virtual void Add(K key, V value) = 0;
    virtual void Clear() = 0;
    virtual void Delete(K key) = 0;
};
}; // namespace PATypes