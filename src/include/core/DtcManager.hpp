#pragma once
#include "types.hpp"
#include "INotifier.hpp"
#include "EventMemory.hpp"
#include "FilePersistence.hpp"
#include <unordered_map>
#include <memory>
#include <mutex>

namespace qcsidm {

class DtcManager {
public:
    DtcManager(std::shared_ptr<EventMemory> mem,
               std::shared_ptr<INotifierHub> notifier,
               std::shared_ptr<IPersistence> persistence)
      : mem_(std::move(mem)), notifier_(std::move(notifier)), persistence_(std::move(persistence)) {}

    // Simple handler: when an event is asserted, create a DTC record; when cleared, clear it
    void handleEventChange(EventId id, bool asserted) {
        std::lock_guard<std::mutex> lk(mx_);
        if (asserted) {
            DtcId d = 0x1000 + id;
            auto &rec = dtcs_[d];
            if (rec.occurrenceCount == 0) rec.firstSeen = std::chrono::system_clock::now();
            rec.dtcId = d;
            rec.eventId = id;
            rec.occurrenceCount += 1;
            rec.lastSeen = std::chrono::system_clock::now();
            if (notifier_) notifier_->notifyDtcCreated(d);
            if (mem_) mem_->pushEvent(id);
            // persist (best-effort)
            persistLocked();
        } else {
            // find dtc by event id and clear
            for (auto it = dtcs_.begin(); it != dtcs_.end(); ++it) {
                if (it->second.eventId == id) {
                    if (notifier_) notifier_->notifyDtcCleared(it->first);
                    dtcs_.erase(it);
                    persistLocked();
                    break;
                }
            }
        }
    }

    bool getDtcRecord(DtcId dtcId, DtcRecord& out) const {
        std::lock_guard<std::mutex> lk(mx_);
        auto it = dtcs_.find(dtcId);
        if (it == dtcs_.end()) return false;
        out = it->second;
        return true;
    }

    std::vector<DtcRecord> getActiveDtcs() const {
        std::lock_guard<std::mutex> lk(mx_);
        std::vector<DtcRecord> out;
        out.reserve(dtcs_.size());
        for (auto &p : dtcs_) out.push_back(p.second);
        return out;
    }

private:
    void persistLocked() {
        if (!persistence_) return;
        std::vector<DtcRecord> list;
        for (auto &p : dtcs_) list.push_back(p.second);
        // best-effort, ignore return
        persistence_->save(list);
    }

    mutable std::mutex mx_;
    std::unordered_map<DtcId, DtcRecord> dtcs_;
    std::shared_ptr<EventMemory> mem_;
    std::shared_ptr<INotifierHub> notifier_;
    std::shared_ptr<IPersistence> persistence_;
};

} // namespace qcsidm
