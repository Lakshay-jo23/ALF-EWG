// Sensors.h  --  Units I & II: SensorStation, TelemetryBuffer, ZoneVector, TelemetryAuditor
#ifndef SENSORS_H
#define SENSORS_H

#include <cstddef>
#include <iostream>
#include <string>

namespace DisasterCore {
namespace Sensors {

// ---------------------------------------------------------------------------
// TelemetryBuffer : dynamic memory, parameterized constructors,
// deep-copy constructor, assignment operator, explicit destructor.
// ---------------------------------------------------------------------------
class TelemetryBuffer {
    double*     data_;
    std::size_t size_;
    std::size_t capacity_;
    void grow();
public:
    explicit TelemetryBuffer(std::size_t capacity = 4);      // dynamic constructor
    TelemetryBuffer(std::size_t capacity, double fillValue); // parameterized dynamic constructor
    TelemetryBuffer(const TelemetryBuffer& other);           // deep copy
    TelemetryBuffer& operator=(const TelemetryBuffer& other);
    ~TelemetryBuffer();                                      // no memory leaks

    void push(double value);
    double&       at(std::size_t i);        // return by reference
    const double& at(std::size_t i) const;
    double last() const;
    double average() const;
    std::size_t size() const     { return size_; }
    std::size_t capacity() const { return capacity_; }

    friend std::ostream& operator<<(std::ostream& os, const TelemetryBuffer& b);
};

// ---------------------------------------------------------------------------
// ZoneVector : aggregated readings of a sensor cluster for a regional zone.
//   operator+  : aggregate clusters       operator> : compare hazard
//   operator<< / operator>> : stream formatting (console and files)
// ---------------------------------------------------------------------------
class ZoneVector {
    std::string zone_;
    double rainSum_, slopeSum_, satSum_, vibSum_;
    int    count_;
public:
    ZoneVector(const std::string& zone = "UNKNOWN", double rain = 0, double slope = 0,
               double saturation = 0, double vibration = 0);
    double hazard() const;
    const std::string& zone() const { return zone_; }

    ZoneVector operator+(const ZoneVector& other) const;
    bool       operator>(const ZoneVector& other) const;

    friend std::ostream& operator<<(std::ostream& os, const ZoneVector& z);
    friend std::istream& operator>>(std::istream& is, ZoneVector& z);
};

// ---------------------------------------------------------------------------
// SensorStation : strict data abstraction, const correctness, static members.
// ---------------------------------------------------------------------------
class TelemetryAuditor;

class SensorStation {
    static int activeStations_;          // network-wide count (shared by all instances)

    int          id_;
    std::string  name_;
    double       latitude_, longitude_;
    double       rawMoisture_;           // private raw metrics (no getters)
    double       rawVibration_;
    TelemetryBuffer history_;

    friend class TelemetryAuditor;       // friend class may inspect raw metrics
public:
    SensorStation(int id, const std::string& name, double lat, double lon);
    SensorStation(const SensorStation& other);
    ~SensorStation();

    static int activeStations() { return activeStations_; }   // static member function

    void record(double moisture, double vibration);
    int  id() const                       { return id_; }
    const std::string& name() const       { return name_; }
    double latitude() const               { return latitude_; }
    double longitude() const              { return longitude_; }
    double& latitudeRef()                 { return latitude_; }  // return by reference
    const TelemetryBuffer& history() const { return history_; }

    friend std::ostream& operator<<(std::ostream& os, const SensorStation& s);
};

// ---------------------------------------------------------------------------
// TelemetryAuditor : friend class, reads private raw sensor metrics.
// ---------------------------------------------------------------------------
class TelemetryAuditor {
public:
    void audit(const SensorStation& s) const;
};

} // namespace Sensors
} // namespace DisasterCore

#endif
