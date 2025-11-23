#pragma once

#include <optional>

#include "LazySequence.h"

namespace PATypes {
template <class A, size_t n> class MultiStripTuringMachine {
  protected:
    LazySequence<A>* strip[n];
    int pointer[n];
    bool accepted = false;
    virtual void d() = 0;

  public:
    MultiStripTuringMachine(Sequence<LazySequence<A>> *initial) {
        for (size_t i = 0; i < n; ++i) {
            strip[i] = new LazySequence<A>(initial->get(i));
            pointer[i] = 0;
        }
    }
    ~MultiStripTuringMachine() {
        for (size_t i = 0; i < n; ++i) {
            delete strip[i];
        }
    }
    bool step() { 
        try {
            d(); 
            return true;
        } catch (std::out_of_range &) {
            return false;
        }
    }
    // deprecated
    bool hasFinished() const {
        for (size_t i = 0; i < n; ++i) {
            try {
                strip[i]->get(pointer[i]);
            } catch (std::out_of_range &err) {
                return true;
            }
            return false;
        }
    }
    LazySequence<A> *getStrip(size_t i) {
        if (i < n) {
            return new LazySequence<A>(*strip[i]);
        } else {
            throw std::logic_error("attempt to return nonexisting strip");
        }
    }
};
} // namespace PATypes