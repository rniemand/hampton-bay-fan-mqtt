import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import fan

from . import CC1101FanHub, CONF_CC1101_FAN_ID, CONF_FAN_ID, cc1101_fan_ns

CC1101FanFanOutput = cc1101_fan_ns.class_("CC1101FanFanOutput", cg.Component, fan.Fan)

CONFIG_SCHEMA = fan.fan_schema(CC1101FanFanOutput).extend(
    {
        cv.GenerateID(CONF_CC1101_FAN_ID): cv.use_id(CC1101FanHub),
        cv.Required(CONF_FAN_ID): cv.int_range(min=0, max=15),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = await fan.new_fan(config)
    await cg.register_component(var, config)

    hub = await cg.get_variable(config[CONF_CC1101_FAN_ID])
    cg.add(var.set_hub(hub))
    cg.add(var.set_fan_id(config[CONF_FAN_ID]))
    cg.add(hub.register_fan(config[CONF_FAN_ID], var))
