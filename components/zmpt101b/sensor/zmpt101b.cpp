#include "zmpt101b.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include <cmath>

namespace esphome {
namespace zmpt101b {

static const char *const TAG = "zmpt101b";

float ZMPT101BSensor::get_setup_priority() const { return setup_priority::DATA; }

void ZMPT101BSensor::update() {
  float vrms = this->get_rms_voltage();
  this->publish_state(vrms);
}

void ZMPT101BSensor::dump_config() {
  LOG_SENSOR(TAG, "ZMPT101B Sensor", this);
  ESP_LOGCONFIG(TAG, "  Sensitivity: %.1f mV/V", this->sensitivity_);
  ESP_LOGCONFIG(TAG, "  Frequency: %u Hz", this->frequency_);
  LOG_UPDATE_INTERVAL(this);
}

/// @brief Calculate zero point (DC offset) by averaging samples over one period
/// @return Average voltage (zero point)
float ZMPT101BSensor::get_zero_point() {
  if (this->source_ == nullptr) {
    ESP_LOGE(TAG, "No voltage source configured");
    return 0.0f;
  }

  float sum = 0.0f;
  uint32_t count = 0;
  uint32_t start_time = millis();

  // Sample for one complete AC period
  while ((millis() - start_time) < this->period_ms_) {
    float voltage = this->source_->sample();
    sum += voltage;
    count++;
    delayMicroseconds(100);  // Small delay between samples
  }

  if (count == 0) {
    ESP_LOGW(TAG, "No samples collected for zero point");
    return 0.0f;
  }

  return sum / count;
}

/// @brief Calculate root mean square (RMS) of AC voltage
/// @param loop_count Number of measurement cycles to average
/// @return RMS voltage in volts
float ZMPT101BSensor::get_rms_voltage(uint8_t loop_count) {
  if (this->source_ == nullptr) {
    ESP_LOGE(TAG, "No voltage source configured");
    return 0.0f;
  }

  double total_rms = 0.0;

  for (uint8_t i = 0; i < loop_count; i++) {
    // Get the DC offset for this cycle
    float zero_point = this->get_zero_point();

    // Now measure RMS over one period
    double sum_squares = 0.0;
    uint32_t count = 0;
    uint32_t start_time = millis();

    while ((millis() - start_time) < this->period_ms_) {
      float voltage = this->source_->sample();
      float ac_voltage = voltage - zero_point;
      sum_squares += (ac_voltage * ac_voltage);
      count++;
      delayMicroseconds(100);  // Small delay between samples
    }

    if (count > 0) {
      // Calculate RMS and apply sensitivity calibration
      float rms = std::sqrt(sum_squares / count);
      total_rms += rms * this->sensitivity_;
    }
  }

  return static_cast<float>(total_rms / loop_count);
}

}  // namespace zmpt101b
}  // namespace esphome
