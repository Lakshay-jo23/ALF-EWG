// Alerts.h  --  EarlyWarningSiren, STL functors, custom stream manipulators
#ifndef ALERTS_H
#define ALERTS_H

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace DisasterCore {
namespace Alerts {

// ---------------------------------------------------------------------------
// Persistent incident record (plain struct so it can be written in binary)
// ---------------------------------------------------------------------------
struct IncidentRecord {
    int      nodeId;
    double   hazard;
    int      level;
    char     zone[16];
    unsigned checksum;
};

unsigned       computeChecksum(const IncidentRecord& r);
IncidentRecord makeRecord(int nodeId, double hazard, int level, const std::string& zone);
std::string    levelName(int level);

// ---------------------------------------------------------------------------
// EarlyWarningSiren : static members track global emergency alert level
// ---------------------------------------------------------------------------
class EarlyWarningSiren {
    static int globalAlertLevel_;
    static int sirenCount_;

    int         id_;
    std::string zone_;
    bool        sounding_;

    EarlyWarningSiren(const EarlyWarningSiren&);             // non-copyable (C++03 style)
    EarlyWarningSiren& operator=(const EarlyWarningSiren&);
public:
    EarlyWarningSiren(int id, const std::string& zone);
    ~EarlyWarningSiren();

    void trigger(int level);
    bool isSounding() const { return sounding_; }

    static int  globalAlertLevel() { return globalAlertLevel_; }
    static int  sirenCount()       { return sirenCount_; }
    static void resetGlobalLevel() { globalAlertLevel_ = 0; }
};

// ---------------------------------------------------------------------------
// Custom functors
// ---------------------------------------------------------------------------
// Sort priority: higher level first, then higher hazard, then lower node id.
struct EvacuationPriority {
    bool operator()(const IncidentRecord& a, const IncidentRecord& b) const {
        if (a.level != b.level)   return a.level > b.level;
        if (a.hazard != b.hazard) return a.hazard > b.hazard;
        return a.nodeId < b.nodeId;
    }
};

// Stateful functor for std::for_each: collects the evacuation route in order.
class EvacuationRouter {
    std::vector<std::string> route_;
    int evacuateCount_;
public:
    EvacuationRouter() : evacuateCount_(0) {}
    void operator()(const IncidentRecord& r) {
        if (r.level >= 3) {
            ++evacuateCount_;
            route_.push_back(r.zone);
        }
    }
    int count() const { return evacuateCount_; }
    const std::vector<std::string>& route() const { return route_; }
};

// ---------------------------------------------------------------------------
// Custom stream manipulators for tabular alert dispatch
// ---------------------------------------------------------------------------
std::ostream& rule(std::ostream& os);          // horizontal line
std::ostream& alertHeader(std::ostream& os);   // column titles

struct Col {                                   // parameterized manipulator: Col(12)
    int width;
    explicit Col(int w) : width(w) {}
};
std::ostream& operator<<(std::ostream& os, const Col& c);
std::ostream& operator<<(std::ostream& os, const IncidentRecord& r);   // one table row

} // namespace Alerts
} // namespace DisasterCore

#endif
