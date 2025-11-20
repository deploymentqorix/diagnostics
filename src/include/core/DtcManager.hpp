#pragma once
#include "types.hpp"
#include "INotifier.hpp"
#include "IDtcManager.hpp"
#include "EventMemory.hpp"
#include "FilePersistence.hpp"
#include <unordered_map>
#include <memory>
#include <mutex>

namespace qcsidm {

class DtcManager : public IDtcManager {
public:
    DtcManager(std::shared_ptr<EventMemory> eventMemory,
               std::shared_ptr<INotifierHub> notifierHub,
               std::shared_ptr<IPersistence> persistence);

    void handleEventChange(EventId id, bool asserted) override;
    bool getDtcRecord(DtcId dtcId, DtcRecord& out) const override;
    std::vector<DtcRecord> getActiveDtcs() const override;

private:
    void persistLocked();

    mutable std::mutex mx_;
    std::unordered_map<DtcId, DtcRecord> dtcs_;
    std::shared_ptr<EventMemory> eventMemory_;
    std::shared_ptr<INotifierHub> notifierHub_;
    std::shared_ptr<IPersistence> persistence_;
};
}