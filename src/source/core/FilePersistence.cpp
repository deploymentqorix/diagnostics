#include "core/FilePersistence.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <stdexcept>

namespace qcsidm {

namespace {
    // helper: time_point -> unix seconds (int64)
    inline std::int64_t tp_to_epoch(const std::chrono::system_clock::time_point& tp) {
        return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
    }

    // helper: unix seconds -> time_point
    inline std::chrono::system_clock::time_point epoch_to_tp(std::int64_t s) {
        return std::chrono::system_clock::time_point(std::chrono::seconds(s));
    }

    // Escape CSV simple (no quotes inside expected fields here, but keep safe)
    std::string csv_escape(const std::string& in) {
        std::string out = in;
        if (out.find_first_of(",\"\n") != std::string::npos) {
            std::ostringstream ss;
            ss << '"';
            for (char c : out) {
                if (c == '"') ss << "\"\"";
                else ss << c;
            }
            ss << '"';
            return ss.str();
        }
        return out;
    }
}

// ctor
FilePersistence::FilePersistence(std::string path)
    : path_(std::move(path))
{}

// Save: write header + rows
bool FilePersistence::save(const std::vector<DtcRecord>& records) {
    std::lock_guard<std::mutex> lk(mx_);
    try {
        std::ofstream os(path_, std::ios::out | std::ios::trunc);
        if (!os.is_open()) return false;

        // header: dtcId,eventId,status,occurrenceCount,firstSeenEpoch,lastSeenEpoch
        os << "dtcId,eventId,status,occurrenceCount,firstSeen,lastSeen\n";

        for (const auto& r : records) {
            auto first = tp_to_epoch(r.firstSeen);
            auto last  = tp_to_epoch(r.lastSeen);

            os << r.dtcId << ','
               << r.eventId << ','
               << static_cast<std::underlying_type_t<DtcStatusFlag>>(r.status) << ','
               << r.occurrenceCount << ','
               << first << ','
               << last << '\n';
        }
        os.flush();
        return os.good();
    } catch (...) {
        return false;
    }
}

// Load: read CSV, skip unknown/malformed rows
bool FilePersistence::load(std::vector<DtcRecord>& outRecords) {
    std::lock_guard<std::mutex> lk(mx_);
    outRecords.clear();

    std::ifstream is(path_, std::ios::in);
    if (!is.is_open()) return false;

    std::string line;
    // read header
    if (!std::getline(is, line)) return false;

    while (std::getline(is, line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string token;

        DtcRecord rec{};
        // dtcId
        if (!std::getline(ss, token, ',')) continue;
        try { rec.dtcId = static_cast<DtcId>(std::stoul(token)); } catch(...) { continue; }

        // eventId
        if (!std::getline(ss, token, ',')) continue;
        try { rec.eventId = static_cast<EventId>(std::stoul(token)); } catch(...) { continue; }

        // status (stored as integer)
        if (!std::getline(ss, token, ',')) continue;
        try {
            auto v = static_cast<int>(std::stoi(token));
            rec.status = static_cast<DtcStatusFlag>(v);
        } catch(...) { rec.status = DtcStatusFlag::None; }

        // occurrenceCount
        if (!std::getline(ss, token, ',')) continue;
        try { rec.occurrenceCount = static_cast<std::uint32_t>(std::stoul(token)); } catch(...) { rec.occurrenceCount = 0; }

        // firstSeen
        if (!std::getline(ss, token, ',')) continue;
        try { rec.firstSeen = epoch_to_tp(static_cast<std::int64_t>(std::stoll(token))); } catch(...) { rec.firstSeen = std::chrono::system_clock::time_point{}; }

        // lastSeen (rest of the line)
        if (!std::getline(ss, token, ',')) {
            // maybe the last field wasn't separated by comma (fallback)
            token.clear();
            if (!std::getline(ss, token)) {}
        }
        try { rec.lastSeen = epoch_to_tp(static_cast<std::int64_t>(std::stoll(token))); } catch(...) { rec.lastSeen = rec.firstSeen; }

        outRecords.push_back(std::move(rec));
    }

    return true;
}

} // namespace qcsidm
