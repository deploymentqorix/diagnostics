#include "core/EventMemory.hpp"
#include <algorithm>

namespace qcsidm {

EventMemory::EventMemory(std::size_t capacity,
                         std::shared_ptr<INotifierHub> notifier)
    : capacity_(capacity),
      notifier_(std::move(notifier)) 
{
}

bool EventMemory::pushEvent(EventId id) {
    std::lock_guard<std::mutex> lk(mx_);
    if (events_.size() >= capacity_) {
        overflowActive_ = true;
        if (notifier_) notifier_->notifyOverflowOccur(static_cast<std::uint32_t>(events_.size()));
        events_.pop_front();
    }
    events_.push_back(id);
    return overflowActive_;
}

void EventMemory::addOrUpdate(const DtcRecord& rec)
{
    std::lock_guard<std::mutex> lock(mx_);

    auto it = std::find_if(entries_.begin(), entries_.end(),
        [&](const EventMemoryEntry& e) { return e.rec.dtcId == rec.dtcId; });

    if (it != entries_.end()) {
        it->rec = rec;
        return;
    }

    if (entries_.size() >= capacity_) {
        entries_.pop_front();
        entries_.push_back(EventMemoryEntry{rec});
        if (!overflowActive_) {
            overflowActive_ = true;
            if (notifier_)
                notifier_->notifyOverflowOccur(/*count*/ 1);
        }
    } else {
        entries_.push_back(EventMemoryEntry{rec});
        if (overflowActive_ && entries_.size() < capacity_) {
            overflowActive_ = false;
            if (notifier_)
                notifier_->notifyOverflowClear();
        }
    }
}

std::size_t EventMemory::size() const
{
    std::lock_guard<std::mutex> lock(mx_);
    // Pick one: events_ or entries_ (your older header had max of both!)
    return std::max(events_.size(), entries_.size());
}

void EventMemory::clearOverflow() {
    std::lock_guard<std::mutex> lk(mx_);
    overflowActive_ = false;
    if (notifier_) notifier_->notifyOverflowClear();
}

std::vector<EventId> EventMemory::snapshotEvents() const {
    std::lock_guard<std::mutex> lk(mx_);
    return std::vector<EventId>(events_.begin(), events_.end());
}

std::vector<DtcRecord> EventMemory::snapshotRecords() const {
    std::lock_guard<std::mutex> lk(mx_);
    std::vector<DtcRecord> out;
    out.reserve(entries_.size());
    for (const auto &e : entries_) out.push_back(e.rec);
    return out;
}

} // namespace qcsidm