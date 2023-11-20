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

CONF_PHONE_NUMBER = "phone_number";

DEFAULT_PHONE_NUMBER = "+33685967412"

rika_gsm_component_ns = cg.esphome_ns.namespace('rika_gsm')
RikaGSMComponent = rika_gsm_component_ns.class_('RikaGSMComponent', cg.Component)

STOVE_PIN_REGEX = re.compile('^\d{4}$')
PHONE_NUMBER_REGEX = re.compile('^\+[1-9]{1}[0-9]{3,14}$')

def validate_stove_pin(value):
    val = cv.string(value)
    res = STOVE_PIN_REGEX.match(value)
    if res is None:
        raise Invalid('Invalid stove pin')
    return value


def validate_phone_number(value):
    val = cv.string(value)
    res = PHONE_NUMBER_REGEX.match(value)
    if res is None:
        raise Invalid('Invalid phone number')
    return value


CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(RikaGSMComponent),
    cv.GenerateID(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
    cv.Required(CONF_PIN): validate_stove_pin,
    cv.Optional(CONF_PHONE_NUMBER, default=DEFAULT_PHONE_NUMBER): validate_phone_number,
}).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    time_ = await cg.get_variable(config[CONF_TIME_ID])
    cg.add(var.set_time(time_))
    cg.add(var.set_pin(config[CONF_PIN]))
    cg.add(var.set_phone_number(config[CONF_PHONE_NUMBER]))
    cg.register_component(var, config)
    uart.register_uart_device(var, config)