#pragma once

#include "esphome.h"
#include "esphome/components/sensor/sensor.h"
#include "zmpt101b.h"

namespace esphome {
namespace zmpt101b_ns {

class ZMPT101BSensor : public PollingComponent, public sensor::Sensor {
 public:
  uint8_t adcPin;
  esphome::zmpt101b_ns::ZMPT101B *zmpt101b;
  
  // constructor
  ZMPT101BSensor(uint8_t adcPin, float sensitivity) : PollingComponent(1000) {
    this->adcPin = adcPin;
    this->zmpt101b = new esphome::zmpt101b_ns::ZMPT101B(adcPin);
    this->zmpt101b->setSensitivity(sensitivity);
  }

  float get_setup_priority() const override { return esphome::setup_priority::HARDWARE; }

  void setup() override {
    // Initialize ADC using ESP-IDF API with shared handle
    if (!this->zmpt101b->begin()) {
      ESP_LOGE("zmpt101b", "Failed to initialize ZMPT101B on GPIO%d", this->adcPin);
      this->mark_failed();
      return;
    }
  }

  void update() override {
    float Vrms = this->zmpt101b->getRmsVoltage();
    publish_state(Vrms);
  }
};

}
}
