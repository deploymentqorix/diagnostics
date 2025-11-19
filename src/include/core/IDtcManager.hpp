#pragma once
#include "types.hpp"
#include <vector>

namespace qcsidm {
class IEventMemory;
class INotifierHub;
class IPersistence;
class IDtcManager {
public:
    virtual ~IDtcManager() = default;
    virtual void handleEventChange(EventId id, bool asserted) = 0;
    virtual bool getDtcRecord(DtcId dtcId, DtcRecord& out) const = 0;
    virtual std::vector<DtcRecord> getActiveDtcs() const = 0;
};
} // namespace qcsidm
