# ZMPT101B AC Voltage Sensor Component for ESPHome

ESPHome custom component for ZMPT101B AC voltage sensor module.

## Usage Example

```yaml
esphome:
  name: voltage-monitor

esp32:
  board: esp32dev
  framework:
    type: arduino

external_components:
  - source:
      type: git
      url: https://github.com/hzkincony/esphome-zmpt101b
      ref: v2.1.0

logger:

sensor:
  # Define ADC sensor (set update_interval to never)
  - platform: adc
    pin: 34
    id: adc34
    update_interval: never
    attenuation: 12db

  # ZMPT101B sensor
  - platform: zmpt101b
    name: "AC Voltage"
    sensor: adc34
    sensitivity: 538.0
    frequency: 50
    update_interval: 5s
```

## Configuration Options

- **sensor** (Required): ID of the ADC sensor
- **sensitivity** (Optional): Calibration value in mV/V. Default: `500.0`
- **frequency** (Optional): AC frequency, `50` or `60` Hz. Default: `50`
- **update_interval** (Optional): Update interval. Default: `5s`

## How to Calculate Sensitivity

The `sensitivity` parameter calibrates the sensor for accurate voltage readings.

**Steps:**

1. Measure the actual AC voltage with a multimeter (e.g., 220V)
2. Check the voltage reported by the sensor in ESPHome (e.g., 210V)
3. Calculate the new sensitivity:
   ```
   new_sensitivity = current_sensitivity × (actual_voltage / measured_voltage)
   ```

**Example:**

- Current sensitivity: `500.0`
- Actual voltage (multimeter): `220V`
- Measured voltage (sensor): `210V`
- New sensitivity: `500.0 × (220 / 210) = 523.8`

Update your configuration with the new sensitivity value and test again. Repeat if needed for better accuracy.
