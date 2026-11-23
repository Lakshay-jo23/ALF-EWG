// Hazard.h  --  Unit I: inline functions, overloading, call/return by reference
#ifndef HAZARD_H
#define HAZARD_H

namespace DisasterCore {
namespace Sensors {

// ---- inline helper functions (expanded at call site for real-time speed) ----
inline double clampPercent(double v) { return v < 0 ? 0 : (v > 100 ? 100 : v); }

// ---- OVERLOADED computeHazardIndex(): different telemetry combinations ----
// (1) rainfall rate (mm/hr) + slope angle (degrees)
inline double computeHazardIndex(double rainfallMmHr, double slopeDeg) {
    return clampPercent(100.0 * (0.6 * (rainfallMmHr / 100.0) + 0.4 * (slopeDeg / 90.0)));
}
// (2) rainfall + slope + soil saturation (%)
inline double computeHazardIndex(double rainfallMmHr, double slopeDeg, double saturationPct) {
    return clampPercent(0.6 * computeHazardIndex(rainfallMmHr, slopeDeg) + 0.4 * saturationPct);
}
// (3) rainfall + slope + soil saturation + vibration frequency (Hz)
inline double computeHazardIndex(double rainfallMmHr, double slopeDeg,
                                 double saturationPct, double vibrationHz) {
    double vib = clampPercent(vibrationHz / 50.0 * 100.0);
    return clampPercent(0.7 * computeHazardIndex(rainfallMmHr, slopeDeg, saturationPct) + 0.3 * vib);
}

// Map a 0-100 hazard value to an alert level: 0 normal, 1 watch, 2 warning, 3 evacuate
inline int classifyHazard(double hazard) {
    if (hazard >= 80.0) return 3;
    if (hazard >= 55.0) return 2;
    if (hazard >= 30.0) return 1;
    return 0;
}

// ---- CALL BY REFERENCE: modifies the caller's coordinates in place ----
inline void normalizeCoordinates(double& lat, double& lon) {
    if (lat > 90.0)  lat = 90.0;
    if (lat < -90.0) lat = -90.0;
    while (lon >= 180.0) lon -= 360.0;
    while (lon < -180.0) lon += 360.0;
}

// ---- RETURN BY REFERENCE: returns the caller's own variable (usable as an lvalue) ----
inline double& accumulateReading(double& total, double reading) {
    total += reading;
    return total;
}

} // namespace Sensors
} // namespace DisasterCore

#endif
