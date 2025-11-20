#pragma once
#include <cstdint>
#include <string>
#include <chrono>
#include <vector>

namespace qcsidm {

using EventId = std::uint32_t;
using DtcId   = std::uint32_t;

enum class EventStatus { Inactive, ActivePending, ActiveConfirmed, Cleared };

enum class DtcStatusFlag : std::uint32_t {
    None      = 0,
    Pending   = 1u << 0,
    Confirmed = 1u << 1,
    Cleared   = 1u << 2,
    Aged      = 1u << 3,
    Active    = 1u << 4,
};
inline DtcStatusFlag operator|(DtcStatusFlag a, DtcStatusFlag b) {
    return static_cast<DtcStatusFlag>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

enum class Severity { Info, Warning, Error, Critical };

struct DebounceConfig {
    std::uint32_t counterAssert = 3;
    std::uint32_t counterClear  = 1;
    std::chrono::milliseconds timeAssert{100};
    std::chrono::milliseconds timeClear{100};
};

struct EventConfig {
    EventId id = 0;
    DtcId mappedDtc = 0;
    Severity severity = Severity::Info;
    DebounceConfig debounceCfg;
    std::string name;
};

struct DtcRecord {
    DtcId dtcId = 0;
    EventId eventId = 0;
    DtcStatusFlag status = DtcStatusFlag::None;
    std::uint32_t occurrenceCount = 0;
    std::chrono::system_clock::time_point firstSeen;
    std::chrono::system_clock::time_point lastSeen;
};

} // namespace qcsidm
