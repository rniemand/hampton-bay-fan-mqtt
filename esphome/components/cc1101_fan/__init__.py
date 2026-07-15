import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.const import CONF_ID, CONF_NUMBER

CODEOWNERS = ["@niemand"]

cc1101_fan_ns = cg.esphome_ns.namespace("cc1101_fan")
CC1101FanHub = cc1101_fan_ns.class_("CC1101FanHub", cg.Component)

CONF_CC1101_FAN_ID = "cc1101_fan_id"
CONF_FAN_ID = "fan_id"
CONF_RX_PIN = "rx_pin"
CONF_TX_PIN = "tx_pin"
CONF_FREQUENCY = "frequency"
CONF_PROTOCOL = "protocol"
CONF_REPEATS = "repeats"
CONF_PULSE_LENGTH = "pulse_length"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(CC1101FanHub),
        cv.Required(CONF_RX_PIN): pins.internal_gpio_input_pin_schema,
        cv.Required(CONF_TX_PIN): pins.internal_gpio_output_pin_schema,
        cv.Required(CONF_FREQUENCY): cv.float_,
        cv.Optional(CONF_PROTOCOL, default=11): cv.int_,
        cv.Optional(CONF_REPEATS, default=8): cv.int_,
        cv.Optional(CONF_PULSE_LENGTH, default=320): cv.int_,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_rx_pin(config[CONF_RX_PIN][CONF_NUMBER]))
    cg.add(var.set_tx_pin(config[CONF_TX_PIN][CONF_NUMBER]))
    cg.add(var.set_frequency(config[CONF_FREQUENCY]))
    cg.add(var.set_protocol(config[CONF_PROTOCOL]))
    cg.add(var.set_repeats(config[CONF_REPEATS]))
    cg.add(var.set_pulse_length(config[CONF_PULSE_LENGTH]))

    cg.add_library("SPI", None)
    cg.add_library("SmartRC-CC1101-Driver-Lib", None)
    cg.add_library("rc-switch", None)
