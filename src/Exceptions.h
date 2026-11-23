// Exceptions.h  --  Unit IV: custom exception classes derived from std::exception
#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <string>

namespace DisasterCore {

class SensorFailureException : public std::exception {
    std::string msg_;
public:
    SensorFailureException(int nodeId, const std::string& reason)
        : msg_("SensorFailureException: node " + std::to_string(nodeId) + " -> " + reason) {}
    const char* what() const noexcept override { return msg_.c_str(); }
};

class TelemetryChecksumMismatch : public std::exception {
    std::string msg_;
public:
    TelemetryChecksumMismatch(unsigned expected, unsigned actual)
        : msg_("TelemetryChecksumMismatch: expected " + std::to_string(expected) +
               " but record has " + std::to_string(actual)) {}
    const char* what() const noexcept override { return msg_.c_str(); }
};

class ThresholdOverflowException : public std::exception {
    std::string msg_;
public:
    ThresholdOverflowException(double value, double limit)
        : msg_("ThresholdOverflowException: reading " + std::to_string(value) +
               " exceeds physical limit " + std::to_string(limit)) {}
    const char* what() const noexcept override { return msg_.c_str(); }
};

} // namespace DisasterCore

#endif
