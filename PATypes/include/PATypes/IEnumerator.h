#pragma once

namespace PATypes {

template <class T> class IEnumerator {
  public:
    virtual ~IEnumerator() {}
    virtual bool moveNext() = 0;
    virtual T &current() = 0;
    virtual void reset() = 0;
};

} // namespace PATypes
