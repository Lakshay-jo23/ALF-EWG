// Persistence.h  --  Unit IV: binary std::fstream logging with transactional rollback
#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "Alerts.h"
#include <fstream>
#include <string>
#include <vector>

namespace DisasterCore {
namespace Alerts {

// File layout:  [Header: magic, count] [record][record]...
// Only records counted in the header are "committed". A transaction writes
// the new records first and updates the header LAST; on any error we seekp()
// back to where the batch started and the header is never touched (rollback).
class IncidentLogger {
    struct Header { int magic; int count; };
    static const int MAGIC = 0xA1F3;

    std::string  path_;
    std::fstream file_;

    Header readHeader();
    void   writeHeader(const Header& h);
public:
    explicit IncidentLogger(const std::string& path);
    ~IncidentLogger();

    void appendTransaction(const std::vector<IncidentRecord>& batch);
    std::vector<IncidentRecord> readAll();          // verifies checksums
    int  committedCount();
    static std::streamoff headerSize() { return sizeof(Header); }
};

} // namespace Alerts
} // namespace DisasterCore

#endif
