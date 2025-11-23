#pragma once

#include <memory>

namespace PATypes {

template <class T> class ICollection {
  public:
    virtual T get(size_t index) = 0;
    virtual size_t getCount() = 0;
};
} // namespace PATypes
