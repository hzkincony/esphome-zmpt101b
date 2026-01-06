#include "zmpt101b.h"
#include <cmath>

#ifdef ESP32
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "zmpt101b";

namespace esphome {
namespace zmpt101b_ns {

// Initialize static shared ADC handles
adc_oneshot_unit_handle_t ZMPT101B::shared_adc_handles_[2] = {nullptr, nullptr};

/// @brief ZMPT101B constructor
/// @param pin analog pin that ZMPT101B connected to.
/// @param frequency AC system frequency
ZMPT101B::ZMPT101B(uint8_t pin, uint16_t frequency)
{
	this->pin_ = pin;
	this->period_ = 1000000 / frequency;
}

/// @brief Convert GPIO pin number to ADC unit and channel
/// @param pin GPIO pin number
/// @param unit Output ADC unit (ADC_UNIT_1 or ADC_UNIT_2)
/// @param channel Output ADC channel
/// @return true if conversion successful, false if pin is not an ADC pin
bool ZMPT101B::pinToAdcChannel(uint8_t pin, adc_unit_t &unit, adc_channel_t &channel)
{
	// Use ESP-IDF helper function to convert GPIO to ADC channel
	if (adc_oneshot_io_to_channel(pin, &unit, &channel) == ESP_OK) {
		return true;
	}
	
	ESP_LOGE(TAG, "GPIO%d is not a valid ADC pin", pin);
	return false;
}

/// @brief Initialize ADC for this sensor
/// @return true if initialization successful
bool ZMPT101B::begin()
{
	if (this->initialized_) {
		return true;
	}
	
	// Convert pin to ADC unit and channel
	if (!pinToAdcChannel(this->pin_, this->adc_unit_, this->adc_channel_)) {
		return false;
	}
	
	ESP_LOGI(TAG, "GPIO%d -> ADC%d Channel %d", this->pin_, this->adc_unit_ + 1, this->adc_channel_);
	
	// Check if this ADC unit is already initialized (shared handle)
	if (shared_adc_handles_[this->adc_unit_] == nullptr) {
		// First sensor using this ADC unit - create the handle
		adc_oneshot_unit_init_cfg_t init_config = {};
		init_config.unit_id = this->adc_unit_;
		init_config.ulp_mode = ADC_ULP_MODE_DISABLE;
		
		esp_err_t err = adc_oneshot_new_unit(&init_config, &shared_adc_handles_[this->adc_unit_]);
		if (err != ESP_OK) {
			ESP_LOGE(TAG, "Failed to initialize ADC%d unit: %s", this->adc_unit_ + 1, esp_err_to_name(err));
			return false;
		}
		ESP_LOGI(TAG, "ADC%d unit initialized", this->adc_unit_ + 1);
	} else {
		ESP_LOGI(TAG, "ADC%d unit already initialized, reusing handle", this->adc_unit_ + 1);
	}
	
	// Configure this channel with 12dB attenuation for 0-3.3V range
	adc_oneshot_chan_cfg_t chan_config = {
		.atten = ADC_ATTEN_DB_12,
		.bitwidth = ADC_BITWIDTH_DEFAULT,
	};
	
	esp_err_t err = adc_oneshot_config_channel(shared_adc_handles_[this->adc_unit_], this->adc_channel_, &chan_config);
	if (err != ESP_OK) {
		ESP_LOGE(TAG, "Failed to configure ADC channel: %s", esp_err_to_name(err));
		return false;
	}
	
	this->initialized_ = true;
	ESP_LOGI(TAG, "ZMPT101B on GPIO%d initialized successfully", this->pin_);
	return true;
}

/// @brief Read ADC value using ESP-IDF oneshot API
/// @return ADC raw value (0-4095) or -1 on error
int ZMPT101B::readAdc()
{
	if (!this->initialized_) {
		ESP_LOGE(TAG, "ADC not initialized");
		return -1;
	}
	
	int raw_value = 0;
	esp_err_t err = adc_oneshot_read(shared_adc_handles_[this->adc_unit_], this->adc_channel_, &raw_value);
	if (err != ESP_OK) {
		ESP_LOGW(TAG, "ADC read failed: %s", esp_err_to_name(err));
		return -1;
	}
	
	return raw_value;
}

/// @brief Set sensitivity
/// @param value Sensitivity value
void ZMPT101B::setSensitivity(float value)
{
	this->sensitivity_ = value;
}

/// @brief Calculate zero point (DC offset)
/// @return zero / center value
int ZMPT101B::getZeroPoint()
{
	uint32_t Vsum = 0;
	uint32_t measurements_count = 0;
	int64_t t_start = esp_timer_get_time();
	
	while ((esp_timer_get_time() - t_start) < this->period_)
	{
		int reading = readAdc();
		if (reading >= 0) {
			Vsum += reading;
			measurements_count++;
		}
	}
	
	if (measurements_count == 0) {
		return 2048; // Return midpoint as fallback
	}
	
	return Vsum / measurements_count;
}

/// @brief Calculate root mean square (RMS) of AC voltage
/// @param loopCount Loop count to calculate (for averaging)
/// @return root mean square (RMS) of AC voltage
float ZMPT101B::getRmsVoltage(uint8_t loopCount)
{
	double readingVoltage = 0.0;

	for (uint8_t i = 0; i < loopCount; i++)
	{
		int zeroPoint = this->getZeroPoint();

		int32_t Vnow = 0;
		uint32_t Vsum = 0;
		uint32_t measurements_count = 0;
		int64_t t_start = esp_timer_get_time();

		while ((esp_timer_get_time() - t_start) < this->period_)
		{
			int reading = readAdc();
			if (reading >= 0) {
				Vnow = reading - zeroPoint;
				Vsum += (Vnow * Vnow);
				measurements_count++;
			}
		}
		
		if (measurements_count > 0) {
			readingVoltage += std::sqrt((double)Vsum / measurements_count) / ADC_SCALE * VREF * this->sensitivity_;
		}
	}

	return static_cast<float>(readingVoltage / loopCount);
}

}  // namespace zmpt101b_ns
}  // namespace esphome

#endif  // ESP32
