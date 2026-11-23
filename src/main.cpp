// ALF-EWG : Autonomous Landslide & Flash-Flood Early Warning Grid
// CIC-211 mini project -- one demo section per syllabus unit.
#include "Alerts.h"
#include "Exceptions.h"
#include "Hazard.h"
#include "Nodes.h"
#include "Persistence.h"
#include "Sensors.h"

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>

using namespace DisasterCore;
using namespace DisasterCore::Sensors;
using namespace DisasterCore::Alerts;

static void banner(const std::string& title) {
    std::cout << "\n==================== " << title << " ====================\n";
}

int main() {
    std::srand(2024);                               // repeatable simulation
    std::remove("incidents.bin");

    // ======================= UNIT I =======================
    banner("UNIT I : Core Foundations & Call Semantics");
    double rain = 72.5, slope = 38.0, sat = 81.0, vib = 14.0;
    std::cout << "Hazard (rain, slope)              = " << computeHazardIndex(rain, slope) << "\n";
    std::cout << "Hazard (rain, slope, saturation)  = " << computeHazardIndex(rain, slope, sat) << "\n";
    std::cout << "Hazard (rain, slope, sat, vibr.)  = " << computeHazardIndex(rain, slope, sat, vib) << "\n";

    double lat = 95.0, lon = 190.5;                 // invalid coordinates
    normalizeCoordinates(lat, lon);                 // call by reference
    std::cout << "Normalized coordinates            = (" << lat << ", " << lon << ")\n";

    double total = 0;
    accumulateReading(total, 10.0) += 0.5;          // return by reference used as lvalue
    accumulateReading(total, 4.0);
    std::cout << "Accumulated telemetry total       = " << total << "\n";

    SensorStation s1(1, "Kedarnath-North", 30.73, 79.07);
    s1.record(64.0, 11.5);
    TelemetryAuditor auditor;
    auditor.audit(s1);                              // friend class

    // ======================= UNIT II =======================
    banner("UNIT II : Encapsulation, Dynamic Memory & Operators");
    std::cout << "Active stations now: " << SensorStation::activeStations() << "\n";
    {
        SensorStation s2(2, "Joshimath-East", 30.55, 79.56);
        SensorStation s3(3, "Chamoli-River", 30.40, 79.32);
        std::cout << "Inside scope       : " << SensorStation::activeStations() << "\n";
    }
    std::cout << "After scope ends   : " << SensorStation::activeStations() << "\n";

    EarlyWarningSiren siren1(1, "Kedarnath"), siren2(2, "Joshimath");
    siren1.trigger(1);
    siren2.trigger(3);
    std::cout << "Sirens installed: " << EarlyWarningSiren::sirenCount()
              << ", global alert level: " << EarlyWarningSiren::globalAlertLevel() << "\n";

    TelemetryBuffer original(2);
    original.push(1.5); original.push(2.5); original.push(3.5);   // forces growth
    TelemetryBuffer copy(original);                 // deep copy constructor
    original.at(0) = 99.9;                          // return by reference
    std::cout << "Original : " << original << "\n";
    std::cout << "Deep copy: " << copy << "  (unchanged)\n";
    TelemetryBuffer assigned;
    assigned = copy;                                // deep copy assignment
    TelemetryBuffer* big = new TelemetryBuffer(5, 0.0);   // dynamic object
    std::cout << "Dynamic buffer: " << *big << "\n";
    delete big;

    ZoneVector a("Kedarnath", 80, 40, 75, 20), b("Joshimath", 40, 30, 60, 10);
    ZoneVector merged = a + b;                      // binary +
    std::cout << a << "\n" << b << "\n" << merged << "\n";
    std::cout << "Kedarnath riskier than Joshimath? " << (a > b ? "yes" : "no") << "\n";
    ZoneVector fromInput;
    std::istringstream fakeConsole("Rudraprayag 55 28 70 12");
    fakeConsole >> fromInput;                       // extraction operator
    std::cout << "Read via >> : " << fromInput << "\n";
    std::ofstream report("zone_report.txt");        // insertion to a file stream
    report << a << "\n" << b << "\n" << merged << "\n" << fromInput << "\n";
    report.close();
    std::cout << "Zone report written to zone_report.txt\n";

    // ======================= UNIT III =======================
    banner("UNIT III : Inheritance, Containment & Polymorphism");
    std::vector<BaseSensorNode*> nodes;
    nodes.push_back(new SoilMoistureNode(101, "Kedarnath"));
    nodes.push_back(new SoilMoistureNode(102, "Joshimath"));
    nodes.push_back(new SeismicNode(103, "Chamoli"));
    nodes.push_back(new AutonomousHybridProbe(104, "Rudraprayag", 42.0));
    nodes.push_back(new SoilMoistureNode(105, "Uttarkashi"));
    nodes.push_back(new SeismicNode(106, "Tehri"));

    nodes[1]->setFaulty(true);                      // dead node
    nodes[4]->power().drain(95.0);                  // almost flat battery
    static_cast<SeismicNode*>(nodes[5])->injectSpike(60.0);   // impossible reading

    AutonomousHybridProbe* probe = static_cast<AutonomousHybridProbe*>(nodes[3]);
    std::cout << "Diamond leaf info: " << probe->deviceInfo() << "\n";

    std::vector<IncidentRecord> incidents;
    std::map<std::string, double> worstPerZone;     // STL map

    for (int cycle = 1; cycle <= 3; ++cycle) {
        std::cout << "\n--- Sweep cycle " << cycle << " ---\n";
        for (std::size_t i = 0; i < nodes.size(); ++i) {
            BaseSensorNode* n = nodes[i];
            try {
                double hazard = n->readTelemetry();             // virtual dispatch
                int level = n->evaluateThreshold();             // virtual dispatch
                std::cout << "  " << n->typeName() << " @" << n->location()
                          << " hazard=" << hazard << " level=" << level << "\n";
                if (level >= 1) {
                    incidents.push_back(makeRecord(n->deviceId(), hazard, level, n->location()));
                    double& worst = worstPerZone[n->location()];
                    if (hazard > worst) worst = hazard;
                }
            } catch (const SensorFailureException& e) {
                std::cout << "  ! " << e.what() << "\n";
            } catch (const ThresholdOverflowException& e) {
                std::cout << "  ! " << e.what() << "\n";
            } catch (const std::exception& e) {
                std::cout << "  ! unexpected: " << e.what() << "\n";
            }
        }
    }

    // ======================= UNIT IV =======================
    banner("UNIT IV : Streams, Persistence, Exceptions & STL");

    std::sort(incidents.begin(), incidents.end(), EvacuationPriority());   // custom functor
    std::cout << "Incidents sorted by evacuation priority:\n" << alertHeader;
    for (std::size_t i = 0; i < incidents.size(); ++i) std::cout << incidents[i] << "\n";
    std::cout << rule;

    EvacuationRouter router = std::for_each(incidents.begin(), incidents.end(), EvacuationRouter());
    std::cout << "Evacuation-level incidents: " << router.count() << "\nRoute order: ";
    for (std::size_t i = 0; i < router.route().size(); ++i)
        std::cout << (i ? " -> " : "") << router.route()[i];
    std::cout << "\n";

    std::cout << "\nWorst hazard per zone (std::map):\n";
    for (std::map<std::string, double>::const_iterator it = worstPerZone.begin();
         it != worstPerZone.end(); ++it)
        std::cout << "  " << Col(14) << it->first << it->second << "\n";

    // ---- binary file logging with transactional rollback ----
    IncidentLogger logger("incidents.bin");
    logger.appendTransaction(incidents);
    std::cout << "\nCommitted " << logger.committedCount() << " incidents to incidents.bin\n";

    std::vector<IncidentRecord> badBatch;
    badBatch.push_back(makeRecord(900, 91.0, 3, "TestZone"));
    IncidentRecord corrupt = makeRecord(901, 85.0, 3, "TestZone");
    corrupt.checksum ^= 0xFFu;                      // simulate transmission error
    badBatch.push_back(corrupt);
    try {
        logger.appendTransaction(badBatch);
    } catch (const TelemetryChecksumMismatch& e) {
        std::cout << "Transaction rolled back: " << e.what() << "\n";
    }
    std::cout << "Committed count after rollback: " << logger.committedCount() << " (unchanged)\n";

    std::vector<IncidentRecord> reloaded = logger.readAll();
    std::cout << "Reloaded " << reloaded.size() << " records from disk, all checksums valid.\n";

    // ---- detect on-disk corruption ----
    {
        std::fstream tamper("incidents.bin", std::ios::in | std::ios::out | std::ios::binary);
        tamper.seekp(IncidentLogger::headerSize() + static_cast<std::streamoff>(offsetof(IncidentRecord, level)));
        char bad = 0x7F;
        tamper.write(&bad, 1);
    }
    try {
        logger.readAll();
    } catch (const TelemetryChecksumMismatch& e) {
        std::cout << "Disk corruption detected: " << e.what() << "\n";
    }

    // ---- dispatch siren alerts and clean up ----
    std::cout << "\nFinal siren dispatch:\n";
    siren1.trigger(3);
    std::cout << "Global alert level: " << levelName(EarlyWarningSiren::globalAlertLevel()) << "\n";

    for (std::size_t i = 0; i < nodes.size(); ++i) delete nodes[i];   // virtual destructors
    std::cout << "\nAll nodes released. Active stations: " << SensorStation::activeStations() << "\n";
    return 0;
}
