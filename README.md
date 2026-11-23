# ALF-EWG : Autonomous Landslide & Flash-Flood Early Warning Grid (CIC-211)

Plain C++11 (also compiles with C++14/17). No external libraries.

## Build & run
    make        # builds ./alfewg
    make run    # build + run
    make clean

Or without make:
    g++ -std=c++11 -Isrc src/*.cpp -o alfewg && ./alfewg

## Where each syllabus requirement lives
| Requirement | File |
|---|---|
| Namespaces DisasterCore::Sensors / ::Alerts | all headers |
| Inline risk-index functions, overloaded computeHazardIndex() | src/Hazard.h |
| Call by reference / return by reference | src/Hazard.h (normalizeCoordinates, accumulateReading), TelemetryBuffer::at() |
| Friend class | TelemetryAuditor in src/Sensors.h |
| Data abstraction, const correctness | SensorStation, TelemetryBuffer |
| Static members / static functions | SensorStation::activeStations(), EarlyWarningSiren::globalAlertLevel() |
| Dynamic ctor, deep copy, destructor | TelemetryBuffer (src/Sensors.cpp) |
| operator+, operator>, operator<<, operator>> | ZoneVector (src/Sensors.cpp) |
| Abstract base, pure virtual readTelemetry()/evaluateThreshold() | BaseSensorNode (src/Nodes.h) |
| Diamond / virtual base classes | DisasterTelemetryDevice -> SolarPoweredNode, WirelessLoRaTransceiver -> AutonomousHybridProbe |
| Containment | PowerSubsystem inside BaseSensorNode |
| vector<BaseSensorNode*> runtime dispatch | sweep loop in src/main.cpp |
| Custom exceptions with what() | src/Exceptions.h |
| Binary fstream + seekp/seekg transactional rollback | src/Persistence.cpp |
| STL vector/map, sort, for_each, functors | src/Alerts.h, src/main.cpp |
| Custom stream manipulators | rule, alertHeader, Col in src/Alerts.cpp |
