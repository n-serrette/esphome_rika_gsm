import re
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, time
from esphome.const import (
    CONF_TIME_ID,
    CONF_ID,
    CONF_PIN,
)

DEPENDENCIES = ['uart']

rika_gsm_component_ns = cg.esphome_ns.namespace('rika_gsm')
RikaGSMComponent = rika_gsm_component_ns.class_('RikaGSMComponent', cg.Component)

STOVE_PIN_REGEX = re.compile('\d{4}')

def validate_stove_pin(value):
    val = cv.string(value)
    res = STOVE_PIN_REGEX.match(value)
    if res is None:
        raise Invalid('Invalid stove pin')
    return value


CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(RikaGSMComponent),
    cv.GenerateID(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
    cv.Required(CONF_PIN): validate_stove_pin
}).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    time_ = await cg.get_variable(config[CONF_TIME_ID])
    cg.add(var.set_time(time_))
    cg.add(var.set_pin(config[CONF_PIN]))
    cg.register_component(var, config)
    uart.register_uart_device(var, config)