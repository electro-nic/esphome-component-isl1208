import esphome.config_validation as cv
import esphome.codegen as cg
from esphome import automation
from esphome.components import i2c, time
from esphome.const import CONF_ID


CODEOWNERS = ["@badbadc0ffee"]
DEPENDENCIES = ["i2c"]
isl1208_ns = cg.esphome_ns.namespace("isl1208")
ISL1208Component = isl1208_ns.class_("ISL1208Component", time.RealTimeClock, i2c.I2CDevice)
WriteAction = isl1208_ns.class_("WriteAction", automation.Action)
ReadAction = isl1208_ns.class_("ReadAction", automation.Action)


CONFIG_SCHEMA = time.TIME_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(ISL1208Component),
    }
).extend(i2c.i2c_device_schema(0x6F))


@automation.register_action(
    "isl1208.write_time",
    WriteAction,
    cv.Schema(
        {
            cv.GenerateID(): cv.use_id(ISL1208Component),
        }
    ),
)
async def isl1208_write_time_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


@automation.register_action(
    "isl1208.read_time",
    ReadAction,
    automation.maybe_simple_id(
        {
            cv.GenerateID(): cv.use_id(isl1208Component),
        }
    ),
)
async def isl1208_read_time_to_code(config, action_id, template_arg, args):
    var = cg.new_Pvariable(action_id, template_arg)
    await cg.register_parented(var, config[CONF_ID])
    return var


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    await time.register_time(var, config)
