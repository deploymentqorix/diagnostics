#pragma once
#include "types.hpp"
#include "INotifier.hpp"
#include <unordered_map>
#include <deque>
#include <mutex>
#include <memory>

namespace qcsidm {

class EventMemory {
public:
    EventMemory(size_t capacity, std::shared_ptr<INotifierHub> notifier)
      : capacity_(capacity), notifier_(notifier) {}

    // Add or update event occurrence: returns true if overflow occurred
    bool pushEvent(const EventId id) {
        std::lock_guard<std::mutex> lk(mx_);
        if (events_.size() >= capacity_) {
            overflow_ = true;
            if (notifier_) notifier_->notifyOverflowOccur((uint32_t)events_.size());
            // drop oldest
            events_.pop_front();
        }
        events_.push_back(id);
        return overflow_;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lk(mx_);
        return events_.size();
    }

    void clearOverflow() {
        std::lock_guard<std::mutex> lk(mx_);
        overflow_ = false;
        if (notifier_) notifier_->notifyOverflowClear();
    }

private:
    size_t capacity_;
    std::deque<EventId> events_;
    bool overflow_{false};
    std::shared_ptr<INotifierHub> notifier_;
    mutable std::mutex mx_;
};

} // namespace qcsidm
