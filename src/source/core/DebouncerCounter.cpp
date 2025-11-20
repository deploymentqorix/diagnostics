#include "../../include/core/DebouncerCounter.hpp"
#include <limits>

namespace qcsidm {

DebouncerCounter::DebouncerCounter(const Config& cfg) : cfg_(cfg) {}

void DebouncerCounter::sample(bool active) {
    std::lock_guard<std::mutex> lk(mx_);
    if (active) {
        if (counter_ < std::numeric_limits<std::uint32_t>::max()) ++counter_;
        if (!state_ && counter_ >= cfg_.assertThreshold) {
            state_ = true;
            if (cb_) cb_(true);
        }
    } else {
        if (counter_ > 0) --counter_;
        if (state_ && counter_ <= cfg_.clearThreshold) {
            state_ = false;
            if (cb_) cb_(false);
        }
    }
}

void DebouncerCounter::setCallback(Callback cb) {
    std::lock_guard<std::mutex> lk(mx_);
    cb_ = std::move(cb);
}

} // namespace qcsidm