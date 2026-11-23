#include "Alerts.h"
#include <algorithm>
#include <cstring>
#include <sstream>

namespace DisasterCore {
namespace Alerts {

unsigned computeChecksum(const IncidentRecord& r) {
    unsigned c = 17;
    c = c * 31 + static_cast<unsigned>(r.nodeId);
    c = c * 31 + static_cast<unsigned>(r.hazard * 100.0);
    c = c * 31 + static_cast<unsigned>(r.level);
    for (std::size_t i = 0; i < sizeof(r.zone) && r.zone[i] != '\0'; ++i)
        c = c * 31 + static_cast<unsigned char>(r.zone[i]);
    return c;
}

IncidentRecord makeRecord(int nodeId, double hazard, int level, const std::string& zone) {
    IncidentRecord r;
    std::memset(&r, 0, sizeof(r));
    r.nodeId = nodeId;
    r.hazard = hazard;
    r.level = level;
    std::strncpy(r.zone, zone.c_str(), sizeof(r.zone) - 1);
    r.checksum = computeChecksum(r);
    return r;
}

std::string levelName(int level) {
    switch (level) {
        case 0:  return "NORMAL";
        case 1:  return "WATCH";
        case 2:  return "WARNING";
        default: return "EVACUATE";
    }
}

// ===== EarlyWarningSiren =====
int EarlyWarningSiren::globalAlertLevel_ = 0;
int EarlyWarningSiren::sirenCount_ = 0;

EarlyWarningSiren::EarlyWarningSiren(int id, const std::string& zone)
    : id_(id), zone_(zone), sounding_(false) { ++sirenCount_; }
EarlyWarningSiren::~EarlyWarningSiren() { --sirenCount_; }

void EarlyWarningSiren::trigger(int level) {
    sounding_ = (level >= 2);
    if (level > globalAlertLevel_) globalAlertLevel_ = level;   // network-wide
    std::cout << "  Siren " << id_ << " (" << zone_ << ") level " << level
              << " -> " << (sounding_ ? "SOUNDING" : "silent") << "\n";
}

// ===== manipulators =====
std::ostream& rule(std::ostream& os) { return os << std::string(58, '-') << '\n'; }

std::ostream& alertHeader(std::ostream& os) {
    os << rule;
    os << std::left << std::setw(8) << "Node" << std::setw(14) << "Zone"
       << std::setw(10) << "Hazard" << std::setw(10) << "Level" << "Status\n";
    return os << rule;
}

std::ostream& operator<<(std::ostream& os, const Col& c) {
    return os << std::left << std::setw(c.width);
}

std::ostream& operator<<(std::ostream& os, const IncidentRecord& r) {
    std::ostringstream h;
    h << std::fixed << std::setprecision(1) << r.hazard;
    return os << Col(8) << r.nodeId << Col(14) << r.zone << Col(10) << h.str()
              << Col(10) << r.level << levelName(r.level);
}

} // namespace Alerts
} // namespace DisasterCore
