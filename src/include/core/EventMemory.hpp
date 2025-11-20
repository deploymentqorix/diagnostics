#pragma once
#include "types.hpp"
#include "INotifier.hpp"
#include <deque>
#include <mutex>
#include <memory>
#include <algorithm>
#include <optional>
#include <vector>

namespace qcsidm {

struct EventMemoryEntry {
    DtcRecord rec;
    EventMemoryEntry() = default;
    explicit EventMemoryEntry(const DtcRecord &r) : rec(r) {}
};

class EventMemory {
public:
    EventMemory(std::size_t capacity, std::shared_ptr<INotifierHub> notifier);

    // Only declarations—NO function bodies!
    bool pushEvent(EventId id);

    void addOrUpdate(const DtcRecord &rec);

    std::size_t size() const;

    void clearOverflow();

    std::vector<EventId> snapshotEvents() const;

    std::vector<DtcRecord> snapshotRecords() const;

private:
    std::size_t capacity_;
    std::deque<EventId> events_;
    std::deque<EventMemoryEntry> entries_;
    bool overflowActive_{false};

    std::shared_ptr<INotifierHub> notifier_;

    mutable std::mutex mx_;
};

} // namespace qcsidm