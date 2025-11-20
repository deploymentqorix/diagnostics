#include "core/DebouncerTime.hpp"

namespace qcsidm {

DebouncerTime::DebouncerTime(const Config& cfg)
    : cfg_(cfg),
      state_(false),
      activeSince_(),
      lastChange_(),
      cb_(nullptr)
{}

void DebouncerTime::sample(bool active) {
    std::lock_guard<std::mutex> lk(mx_);
    auto now = std::chrono::steady_clock::now();

    if (active) {
        if (!state_) {
            if (activeSince_.time_since_epoch().count() == 0)
                activeSince_ = now;

            if (now - activeSince_ >= cfg_.timeAssert) {
                state_ = true;
                if (cb_) cb_(true);
            }
        }
    } else {
        if (state_) {
            if (now - lastChange_ >= cfg_.timeClear) {
                state_ = false;
                if (cb_) cb_(false);
            }
        }
        lastChange_ = now;
    }
}

void DebouncerTime::setCallback(Callback cb) {
    std::lock_guard<std::mutex> lk(mx_);
    cb_ = std::move(cb);
}

} // namespace qcsidm
