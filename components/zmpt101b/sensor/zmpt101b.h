#pragma once

#include <stdint.h>

#ifdef ESP32
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "hal/adc_types.h"
#endif

#define DEFAULT_FREQUENCY 50
#define DEFAULT_SENSITIVITY 500.0f

#define ADC_SCALE 4095.0f
#define VREF 3.3f

namespace esphome {
namespace zmpt101b_ns {

#ifdef ESP32

class ZMPT101B
{
public:
	ZMPT101B(uint8_t pin, uint16_t frequency = DEFAULT_FREQUENCY);
	
	/// Initialize ADC for this pin (must be called once during setup)
	bool begin();
	
	void setSensitivity(float value);
	float getRmsVoltage(uint8_t loopCount = 1);

private:
	uint8_t  pin_;
	uint32_t period_;
	float 	 sensitivity_ = DEFAULT_SENSITIVITY;
	
	adc_unit_t adc_unit_;
	adc_channel_t adc_channel_;
	bool initialized_ = false;
	
	/// Shared ADC handles for ADC1 and ADC2 (static to be shared across all instances)
	static adc_oneshot_unit_handle_t shared_adc_handles_[2];
	
	/// Convert GPIO pin to ADC unit and channel
	bool pinToAdcChannel(uint8_t pin, adc_unit_t &unit, adc_channel_t &channel);
	
	/// Read ADC value using ESP-IDF API
	int readAdc();
	
	/// Calculate zero point (DC offset)
	int getZeroPoint();
};

#endif  // ESP32

}  // namespace zmpt101b_ns
}  // namespace esphome
