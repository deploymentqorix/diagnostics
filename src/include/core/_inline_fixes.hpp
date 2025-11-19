#pragma once
#include "DebouncerTime.hpp"
#include "DebouncerCounter.hpp"

namespace qcsidm {

// Inline/trivial implementations (to ensure symbol availability)
inline DebouncerCounter::DebouncerCounter(const Config& cfg) : cfg_(cfg), counter_(0), state_(false) {}
inline void DebouncerCounter::sample(bool) {}
inline void DebouncerCounter::setCallback(Callback) {}

inline DebouncerTime::DebouncerTime(const Config& cfg) : cfg_(cfg), state_(false) {}
inline void DebouncerTime::sample(bool) {}
inline void DebouncerTime::setCallback(Callback) {}

} // namespace qcsidm
