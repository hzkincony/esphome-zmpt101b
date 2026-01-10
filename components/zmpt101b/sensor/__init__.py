import esphome.codegen as cg
from esphome.components import sensor, voltage_sampler
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_SENSOR,
    DEVICE_CLASS_VOLTAGE,
    ICON_FLASH,
    STATE_CLASS_MEASUREMENT,
    UNIT_VOLT,
)

CODEOWNERS = ["@hzkincony"]
DEPENDENCIES = []
AUTO_LOAD = ["voltage_sampler"]

zmpt101b_ns = cg.esphome_ns.namespace("zmpt101b")

ZMPT101BSensor = zmpt101b_ns.class_(
    "ZMPT101BSensor",
    sensor.Sensor,
    cg.PollingComponent,
)

CONF_SENSITIVITY = "sensitivity"
CONF_FREQUENCY = "frequency"

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        ZMPT101BSensor,
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=2,
        device_class=DEVICE_CLASS_VOLTAGE,
        state_class=STATE_CLASS_MEASUREMENT,
        icon=ICON_FLASH,
    )
    .extend(
        {
            cv.Required(CONF_SENSOR): cv.use_id(voltage_sampler.VoltageSampler),
            cv.Optional(CONF_SENSITIVITY, default=500.0): cv.float_range(min=1.0, max=10000.0),
            cv.Optional(CONF_FREQUENCY, default=50): cv.int_range(min=50, max=60),
        }
    )
    .extend(cv.polling_component_schema("5s"))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await sensor.register_sensor(var, config)
    await cg.register_component(var, config)

    cg.add(var.set_sensitivity(config[CONF_SENSITIVITY]))
    cg.add(var.set_frequency(config[CONF_FREQUENCY]))

    sens = await cg.get_variable(config[CONF_SENSOR])
    cg.add(var.set_source(sens))
