#include "Nodes.h"
#include "Hazard.h"
#include "Exceptions.h"
#include <cstdlib>

namespace DisasterCore {
namespace Sensors {

// ===== DisasterTelemetryDevice =====
DisasterTelemetryDevice::DisasterTelemetryDevice(int id, const std::string& fw)
    : deviceId_(id), firmware_(fw) {}
DisasterTelemetryDevice::~DisasterTelemetryDevice() {}
std::string DisasterTelemetryDevice::deviceInfo() const {
    return "Device#" + std::to_string(deviceId_) + " fw=" + firmware_;
}

// ===== PowerSubsystem =====
void PowerSubsystem::drain(double amount) {
    batteryPct_ -= amount;
    if (batteryPct_ < 0) batteryPct_ = 0;
}

// ===== BaseSensorNode =====
BaseSensorNode::BaseSensorNode(int id, const std::string& location)
    : DisasterTelemetryDevice(id, "base-1.0"), location_(location),
      power_(100.0), log_(4), faulty_(false) {}
BaseSensorNode::~BaseSensorNode() {}

void BaseSensorNode::checkHealth() const {
    if (faulty_)          throw SensorFailureException(deviceId(), "no response from " + location_);
    if (power_.isLow())   throw SensorFailureException(deviceId(), "battery depleted at " + location_);
}

double BaseSensorNode::store(double hazard) {
    log_.push(hazard);
    power_.drain(2.0);
    return hazard;
}

// ===== SoilMoistureNode =====
SoilMoistureNode::SoilMoistureNode(int id, const std::string& loc)
    : DisasterTelemetryDevice(id, "soil-2.0"), BaseSensorNode(id, loc) {}

double SoilMoistureNode::readTelemetry() {
    checkHealth();
    double saturation = 45 + std::rand() % 55;       // simulated % saturation
    if (saturation > 100.0) throw ThresholdOverflowException(saturation, 100.0);
    return store(saturation);
}
int SoilMoistureNode::evaluateThreshold() { return classifyHazard(log_.last()); }

// ===== SeismicNode =====
SeismicNode::SeismicNode(int id, const std::string& loc)
    : DisasterTelemetryDevice(id, "geo-1.4"), BaseSensorNode(id, loc), spike_(0) {}

double SeismicNode::readTelemetry() {
    checkHealth();
    double hz = 2 + std::rand() % 20 + spike_;       // simulated vibration in Hz
    spike_ = 0;
    if (hz > 50.0) throw ThresholdOverflowException(hz, 50.0);
    return store(hz / 50.0 * 100.0);
}
int SeismicNode::evaluateThreshold() { return classifyHazard(log_.last()); }

// ===== SolarPoweredNode =====
SolarPoweredNode::SolarPoweredNode(int id)
    : DisasterTelemetryDevice(id, "solar-1.1"), batteryVolts_(3.7) {}
void SolarPoweredNode::harvest() {
    batteryVolts_ += 0.05;
    if (batteryVolts_ > 4.2) batteryVolts_ = 4.2;
}
std::string SolarPoweredNode::deviceInfo() const {
    return DisasterTelemetryDevice::deviceInfo() + " [solar " + std::to_string(batteryVolts_) + "V]";
}

// ===== WirelessLoRaTransceiver =====
WirelessLoRaTransceiver::WirelessLoRaTransceiver(int id)
    : DisasterTelemetryDevice(id, "lora-3.0"), signalDbm_(-92) {}
bool WirelessLoRaTransceiver::transmit(const std::string& message) const {
    (void)message;
    return signalDbm_ > -110;
}
std::string WirelessLoRaTransceiver::deviceInfo() const {
    return DisasterTelemetryDevice::deviceInfo() + " [LoRa " + std::to_string(signalDbm_) + "dBm]";
}

// ===== AutonomousHybridProbe =====
// The most-derived class initializes the single shared virtual base.
AutonomousHybridProbe::AutonomousHybridProbe(int id, const std::string& loc, double slopeDeg)
    : DisasterTelemetryDevice(id, "hybrid-2.1"), BaseSensorNode(id, loc),
      SolarPoweredNode(id), WirelessLoRaTransceiver(id), slopeDeg_(slopeDeg) {}

double AutonomousHybridProbe::readTelemetry() {
    checkHealth();
    harvest();
    double rain = std::rand() % 100;
    double saturation = std::rand() % 100;
    double vibration = std::rand() % 30;
    return store(computeHazardIndex(rain, slopeDeg_, saturation, vibration));
}
int AutonomousHybridProbe::evaluateThreshold() { return classifyHazard(log_.last()); }

std::string AutonomousHybridProbe::deviceInfo() const {
    return DisasterTelemetryDevice::deviceInfo() + " [solar " + std::to_string(batteryVolts_) +
           "V, LoRa " + std::to_string(signalDbm_) + "dBm, battery " +
           std::to_string(static_cast<int>(power_.level())) + "%]";
}

} // namespace Sensors
} // namespace DisasterCore
