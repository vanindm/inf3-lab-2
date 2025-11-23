#pragma once

#include "IEnumerator.h"

namespace PATypes {

template <class T> class IEnumerable {
  public:
    virtual IEnumerator<T> *getEnumerator() = 0;
};

} // namespace PATypes
