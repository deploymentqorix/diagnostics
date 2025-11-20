#include "core/DtcManager.hpp"
#include <chrono>

namespace qcsidm {

DtcManager::DtcManager(std::shared_ptr<EventMemory> eventMemory,
                       std::shared_ptr<INotifierHub> notifierHub,
                       std::shared_ptr<IPersistence> persistence)
    : eventMemory_(std::move(eventMemory)),
      notifierHub_(std::move(notifierHub)),
      persistence_(std::move(persistence))
{
}

void DtcManager::persistLocked() {
    if (!persistence_) return;
    std::vector<DtcRecord> list;
    for (auto &p : dtcs_) list.push_back(p.second);
    // best-effort, ignore return
    persistence_->save(list);
}

void DtcManager::handleEventChange(EventId eventId, bool asserted)
{
    std::lock_guard<std::mutex> lk(mx_);

    auto now = std::chrono::system_clock::now();

    if (asserted)
    {
        // New DTC or update existing one
        // NOTE: using derived DTC id (example)
        DtcId d = static_cast<DtcId>(0x1000 + eventId);
        auto it = dtcs_.find(d);

        if (it == dtcs_.end())
        {
            // Create a new record
            DtcRecord rec{};
            rec.dtcId = d;
            rec.eventId = eventId;
            rec.status = DtcStatusFlag::Active;
            rec.occurrenceCount = 1;
            rec.firstSeen = now;
            rec.lastSeen = now;

            dtcs_[d] = rec;

            // Notify creation
            if (notifierHub_)
                notifierHub_->notifyDtcCreated(static_cast<std::uint32_t>(d));
        }
        else
        {
            // Update existing record
            DtcRecord &rec = it->second;
            rec.status = DtcStatusFlag::Active;
            rec.occurrenceCount += 1;
            rec.lastSeen = now;
        }

        if (eventMemory_) eventMemory_->pushEvent(eventId);

    }
    else
    {
        // Clear (or delete) the DTC by eventId
        for (auto it = dtcs_.begin(); it != dtcs_.end(); ++it) {
            if (it->second.eventId == eventId) {
                DtcId d = it->first;
                if (notifierHub_) notifierHub_->notifyDtcCleared(static_cast<std::uint32_t>(d));
                dtcs_.erase(it);
                break;
            }
        }
    }

    // Persist all active DTCs (best effort)
    persistLocked();
}

bool DtcManager::getDtcRecord(DtcId dtcId, DtcRecord &out) const
{
    std::lock_guard<std::mutex> lk(mx_);
    auto it = dtcs_.find(dtcId);
    if (it == dtcs_.end())
        return false;
    out = it->second;
    return true;
}

std::vector<DtcRecord> DtcManager::getActiveDtcs() const
{
    std::lock_guard<std::mutex> lk(mx_);
    std::vector<DtcRecord> result;
    result.reserve(dtcs_.size());
    for (auto &kv : dtcs_) {
        result.push_back(kv.second);
    }
    return result;
}

} // namespace qcsidm
