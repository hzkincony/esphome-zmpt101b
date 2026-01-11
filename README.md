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
    attenuation: 11db

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
