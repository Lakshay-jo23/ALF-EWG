#include "Persistence.h"
#include "Exceptions.h"
#include <stdexcept>

namespace DisasterCore {
namespace Alerts {

IncidentLogger::IncidentLogger(const std::string& path) : path_(path) {
    file_.open(path_.c_str(), std::ios::in | std::ios::out | std::ios::binary);
    if (!file_) {                                   // file does not exist yet: create it
        file_.clear();
        std::ofstream create(path_.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
        Header h = { MAGIC, 0 };
        create.write(reinterpret_cast<const char*>(&h), sizeof(h));
        create.close();
        file_.open(path_.c_str(), std::ios::in | std::ios::out | std::ios::binary);
    }
    if (!file_) throw std::runtime_error("cannot open incident log: " + path_);
}

IncidentLogger::~IncidentLogger() { file_.close(); }

IncidentLogger::Header IncidentLogger::readHeader() {
    Header h = { 0, 0 };
    file_.clear();
    file_.seekg(0, std::ios::beg);
    file_.read(reinterpret_cast<char*>(&h), sizeof(h));
    if (!file_ || h.magic != MAGIC) throw std::runtime_error("incident log is corrupt");
    return h;
}

void IncidentLogger::writeHeader(const Header& h) {
    file_.clear();
    file_.seekp(0, std::ios::beg);
    file_.write(reinterpret_cast<const char*>(&h), sizeof(h));
    file_.flush();
}

int IncidentLogger::committedCount() { return readHeader().count; }

void IncidentLogger::appendTransaction(const std::vector<IncidentRecord>& batch) {
    Header h = readHeader();
    std::streampos start = static_cast<std::streamoff>(sizeof(Header)) +
                           static_cast<std::streamoff>(h.count) * sizeof(IncidentRecord);
    file_.clear();
    file_.seekp(start);                              // begin transaction here
    try {
        for (std::size_t i = 0; i < batch.size(); ++i) {
            unsigned expected = computeChecksum(batch[i]);
            if (expected != batch[i].checksum)
                throw TelemetryChecksumMismatch(expected, batch[i].checksum);
            file_.write(reinterpret_cast<const char*>(&batch[i]), sizeof(IncidentRecord));
            if (!file_) throw std::runtime_error("disk write failed");
        }
        h.count += static_cast<int>(batch.size());   // COMMIT: header updated last
        writeHeader(h);
    } catch (...) {
        file_.clear();
        file_.seekp(start);                          // ROLLBACK: return to batch start
        file_.seekg(start);
        throw;                                       // header unchanged -> batch invisible
    }
}

std::vector<IncidentRecord> IncidentLogger::readAll() {
    Header h = readHeader();
    std::vector<IncidentRecord> out;
    file_.clear();
    file_.seekg(sizeof(Header), std::ios::beg);
    for (int i = 0; i < h.count; ++i) {
        IncidentRecord r;
        file_.read(reinterpret_cast<char*>(&r), sizeof(r));
        if (!file_) throw std::runtime_error("unexpected end of incident log");
        unsigned expected = computeChecksum(r);
        if (expected != r.checksum) throw TelemetryChecksumMismatch(expected, r.checksum);
        out.push_back(r);
    }
    return out;
}

} // namespace Alerts
} // namespace DisasterCore
