// Nodes.h  --  Unit III: abstract class, diamond problem (virtual bases),
//              containment (PowerSubsystem inside BaseSensorNode), polymorphism
#ifndef NODES_H
#define NODES_H

#include "Sensors.h"
#include <string>

namespace DisasterCore {
namespace Sensors {

// ---- Root of the diamond ----
class DisasterTelemetryDevice {
protected:
    int         deviceId_;
    std::string firmware_;
public:
    explicit DisasterTelemetryDevice(int id = 0, const std::string& fw = "generic-1.0");
    virtual ~DisasterTelemetryDevice();
    int deviceId() const { return deviceId_; }
    virtual std::string deviceInfo() const;
};

// ---- Independent class used for containment (has-a), NOT inheritance ----
class PowerSubsystem {
    double batteryPct_;
public:
    explicit PowerSubsystem(double pct = 100.0) : batteryPct_(pct) {}
    void   drain(double amount);
    double level() const { return batteryPct_; }
    bool   isLow() const { return batteryPct_ < 10.0; }
};

// ---- Abstract base class (pure virtual functions) ----
class BaseSensorNode : public virtual DisasterTelemetryDevice {
protected:
    std::string     location_;
    PowerSubsystem  power_;        // containment / aggregation
    TelemetryBuffer log_;
    bool            faulty_;
    void   checkHealth() const;    // throws SensorFailureException
    double store(double hazard);   // log + drain battery
public:
    BaseSensorNode(int id, const std::string& location);
    virtual ~BaseSensorNode();

    virtual double readTelemetry() = 0;      // returns hazard value 0..100
    virtual int    evaluateThreshold() = 0;  // returns alert level 0..3
    virtual std::string typeName() const = 0;

    const std::string& location() const { return location_; }
    PowerSubsystem& power()             { return power_; }
    void setFaulty(bool f)              { faulty_ = f; }
};

// ---- Concrete sensor nodes ----
class SoilMoistureNode : public BaseSensorNode {
public:
    SoilMoistureNode(int id, const std::string& loc);
    double readTelemetry() override;
    int    evaluateThreshold() override;
    std::string typeName() const override { return "SoilMoisture"; }
};

class SeismicNode : public BaseSensorNode {
    double spike_;    // injected disturbance, used to demonstrate overflow
public:
    SeismicNode(int id, const std::string& loc);
    void injectSpike(double hz) { spike_ = hz; }
    double readTelemetry() override;
    int    evaluateThreshold() override;
    std::string typeName() const override { return "Seismic"; }
};

// ---- Intermediate derivations (virtually inherit the root) ----
class SolarPoweredNode : public virtual DisasterTelemetryDevice {
protected:
    double batteryVolts_;
public:
    explicit SolarPoweredNode(int id = 0);
    void   harvest();
    double batteryVolts() const { return batteryVolts_; }
    std::string deviceInfo() const override;
};

class WirelessLoRaTransceiver : public virtual DisasterTelemetryDevice {
protected:
    int signalDbm_;
public:
    explicit WirelessLoRaTransceiver(int id = 0);
    bool transmit(const std::string& message) const;
    std::string deviceInfo() const override;
};

// ---- Derived leaf: one root object shared thanks to virtual inheritance ----
class AutonomousHybridProbe : public BaseSensorNode,
                              public SolarPoweredNode,
                              public WirelessLoRaTransceiver {
    double slopeDeg_;
public:
    AutonomousHybridProbe(int id, const std::string& loc, double slopeDeg);
    double readTelemetry() override;
    int    evaluateThreshold() override;
    std::string typeName() const override { return "HybridProbe"; }
    std::string deviceInfo() const override;   // resolves ambiguity of two overriders
};

} // namespace Sensors
} // namespace DisasterCore

#endif
