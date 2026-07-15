import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.const import CONF_OUTPUT_ID

from . import CC1101FanHub, CONF_CC1101_FAN_ID, CONF_FAN_ID, cc1101_fan_ns

CC1101FanLightOutput = cc1101_fan_ns.class_("CC1101FanLightOutput", light.LightOutput)

CONFIG_SCHEMA = light.BINARY_LIGHT_SCHEMA.extend(
    {
        cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(CC1101FanLightOutput),
        cv.GenerateID(CONF_CC1101_FAN_ID): cv.use_id(CC1101FanHub),
        cv.Required(CONF_FAN_ID): cv.int_range(min=0, max=15),
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await light.register_light(var, config)

    hub = await cg.get_variable(config[CONF_CC1101_FAN_ID])
    cg.add(var.set_hub(hub))
    cg.add(var.set_fan_id(config[CONF_FAN_ID]))
    cg.add(hub.register_light(config[CONF_FAN_ID], var))
