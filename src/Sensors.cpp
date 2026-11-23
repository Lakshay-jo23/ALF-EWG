#include "Sensors.h"
#include "Hazard.h"
#include <algorithm>
#include <iomanip>
#include <stdexcept>

namespace DisasterCore {
namespace Sensors {

// ===================== TelemetryBuffer =====================
TelemetryBuffer::TelemetryBuffer(std::size_t capacity)
    : data_(nullptr), size_(0), capacity_(capacity == 0 ? 1 : capacity) {
    data_ = new double[capacity_];
}

TelemetryBuffer::TelemetryBuffer(std::size_t capacity, double fillValue)
    : TelemetryBuffer(capacity) {
    for (std::size_t i = 0; i < capacity_; ++i) data_[i] = fillValue;
    size_ = capacity_;
}

TelemetryBuffer::TelemetryBuffer(const TelemetryBuffer& other)
    : data_(new double[other.capacity_]), size_(other.size_), capacity_(other.capacity_) {
    std::copy(other.data_, other.data_ + other.size_, data_);   // deep copy
}

TelemetryBuffer& TelemetryBuffer::operator=(const TelemetryBuffer& other) {
    if (this != &other) {
        double* fresh = new double[other.capacity_];
        std::copy(other.data_, other.data_ + other.size_, fresh);
        delete[] data_;
        data_ = fresh;
        size_ = other.size_;
        capacity_ = other.capacity_;
    }
    return *this;
}

TelemetryBuffer::~TelemetryBuffer() { delete[] data_; }

void TelemetryBuffer::grow() {
    std::size_t newCap = capacity_ * 2;
    double* bigger = new double[newCap];
    std::copy(data_, data_ + size_, bigger);
    delete[] data_;
    data_ = bigger;
    capacity_ = newCap;
}

void TelemetryBuffer::push(double value) {
    if (size_ == capacity_) grow();
    data_[size_++] = value;
}

double& TelemetryBuffer::at(std::size_t i) {
    if (i >= size_) throw std::out_of_range("TelemetryBuffer index out of range");
    return data_[i];
}
const double& TelemetryBuffer::at(std::size_t i) const {
    if (i >= size_) throw std::out_of_range("TelemetryBuffer index out of range");
    return data_[i];
}

double TelemetryBuffer::last() const {
    if (size_ == 0) throw std::out_of_range("TelemetryBuffer is empty");
    return data_[size_ - 1];
}

double TelemetryBuffer::average() const {
    if (size_ == 0) return 0.0;
    double sum = 0;
    for (std::size_t i = 0; i < size_; ++i) sum += data_[i];
    return sum / size_;
}

std::ostream& operator<<(std::ostream& os, const TelemetryBuffer& b) {
    os << "[";
    for (std::size_t i = 0; i < b.size_; ++i) {
        if (i) os << ", ";
        os << b.data_[i];
    }
    return os << "]";
}

// ===================== ZoneVector =====================
ZoneVector::ZoneVector(const std::string& zone, double rain, double slope,
                       double saturation, double vibration)
    : zone_(zone), rainSum_(rain), slopeSum_(slope), satSum_(saturation),
      vibSum_(vibration), count_(1) {}

double ZoneVector::hazard() const {
    double n = count_;
    return computeHazardIndex(rainSum_ / n, slopeSum_ / n, satSum_ / n, vibSum_ / n);
}

ZoneVector ZoneVector::operator+(const ZoneVector& o) const {
    ZoneVector r(*this);
    r.zone_ = zone_ + "+" + o.zone_;
    r.rainSum_ += o.rainSum_;
    r.slopeSum_ += o.slopeSum_;
    r.satSum_ += o.satSum_;
    r.vibSum_ += o.vibSum_;
    r.count_ += o.count_;
    return r;
}

bool ZoneVector::operator>(const ZoneVector& o) const { return hazard() > o.hazard(); }

std::ostream& operator<<(std::ostream& os, const ZoneVector& z) {
    double n = z.count_;
    os << std::left << std::setw(26) << z.zone_ << std::fixed << std::setprecision(1)
       << " rain=" << z.rainSum_ / n << " slope=" << z.slopeSum_ / n
       << " sat=" << z.satSum_ / n << " vib=" << z.vibSum_ / n
       << " | hazard=" << z.hazard();
    return os;
}

// expected input:  zoneName rain slope saturation vibration
std::istream& operator>>(std::istream& is, ZoneVector& z) {
    std::string name;
    double r, s, sat, v;
    if (is >> name >> r >> s >> sat >> v) z = ZoneVector(name, r, s, sat, v);
    return is;
}

// ===================== SensorStation =====================
int SensorStation::activeStations_ = 0;

SensorStation::SensorStation(int id, const std::string& name, double lat, double lon)
    : id_(id), name_(name), latitude_(lat), longitude_(lon),
      rawMoisture_(0), rawVibration_(0), history_(4) {
    normalizeCoordinates(latitude_, longitude_);   // call by reference
    ++activeStations_;
}

SensorStation::SensorStation(const SensorStation& o)
    : id_(o.id_), name_(o.name_), latitude_(o.latitude_), longitude_(o.longitude_),
      rawMoisture_(o.rawMoisture_), rawVibration_(o.rawVibration_), history_(o.history_) {
    ++activeStations_;
}

SensorStation::~SensorStation() { --activeStations_; }

void SensorStation::record(double moisture, double vibration) {
    rawMoisture_ = moisture;
    rawVibration_ = vibration;
    history_.push(moisture);
}

std::ostream& operator<<(std::ostream& os, const SensorStation& s) {
    return os << "Station#" << s.id_ << " '" << s.name_ << "' @(" << s.latitude_
              << ", " << s.longitude_ << ") history=" << s.history_;
}

// ===================== TelemetryAuditor =====================
void TelemetryAuditor::audit(const SensorStation& s) const {
    std::cout << "  [AUDIT] station " << s.id_ << " raw moisture=" << s.rawMoisture_
              << " raw vibration=" << s.rawVibration_ << " (private data, via friend access)\n";
}

} // namespace Sensors
} // namespace DisasterCore
