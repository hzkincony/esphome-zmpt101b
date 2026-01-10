#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/voltage_sampler/voltage_sampler.h"

#define DEFAULT_FREQUENCY 50
#define DEFAULT_SENSITIVITY 500.0f

namespace esphome {
namespace zmpt101b {

class ZMPT101BSensor : public sensor::Sensor, public PollingComponent {
 public:
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override;

  void set_source(voltage_sampler::VoltageSampler *source) { this->source_ = source; }
  void set_sensitivity(float sensitivity) { this->sensitivity_ = sensitivity; }
  void set_frequency(uint16_t frequency) {
    this->frequency_ = frequency;
    this->period_ms_ = 1000.0f / frequency;
  }

  float get_rms_voltage(uint8_t loop_count = 1);

 protected:
  /// The sampling source to read values from (ADC sensor)
  voltage_sampler::VoltageSampler *source_{nullptr};

  /// Sensitivity value (mV/V)
  float sensitivity_{DEFAULT_SENSITIVITY};

  /// AC frequency (Hz)
  uint16_t frequency_{DEFAULT_FREQUENCY};

  /// Period in milliseconds
  float period_ms_{20.0f};  // Default for 50Hz

  /// Calculate zero point (DC offset)
  float get_zero_point();
};

}  // namespace zmpt101b
}  // namespace esphome
