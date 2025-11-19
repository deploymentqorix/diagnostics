#pragma once
#include "types.hpp"
#include <string>
#include <vector>
#include <mutex>

namespace qcsidm {

class IPersistence {
public:
    virtual ~IPersistence() = default;
    virtual bool save(const std::vector<DtcRecord>& records) = 0;
    virtual bool load(std::vector<DtcRecord>& outRecords) = 0;
};

class FilePersistence : public IPersistence {
public:
    explicit FilePersistence(std::string path);
    ~FilePersistence() override = default;

    bool save(const std::vector<DtcRecord>& records) override;
    bool load(std::vector<DtcRecord>& outRecords) override;

private:
    std::string path_;
    std::mutex mx_;
};

} // namespace qcsidm
