#pragma once

#include <complex>

namespace PATypes {
template <class T>
int operator<=(const std::complex<T> &a, const std::complex<T> &b) {
    return a.imag() * a.imag() + a.real() * a.real() <=
           b.imag() * b.imag() + b.real() * b.real();
}
template <class T>
int operator<(const std::complex<T> &a, const std::complex<T> &b) {
    return a.imag() * a.imag() + a.real() * a.real() <
           b.imag() * b.imag() + b.real() * b.real();
}
} // namespace PATypes