import re
from esphome import automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, time
from esphome.const import (
    CONF_ID,
    CONF_PIN,
)

DEPENDENCIES = ['uart']

CONF_RIKA_GSM_ID = "rika_gsm_id"
CONF_PHONE_NUMBER = "phone_number"
CONF_COMMAND = "command"

DEFAULT_PHONE_NUMBER = "+33685967412"

rika_gsm_component_ns = cg.esphome_ns.namespace('rika_gsm')
RikaGSMComponent = rika_gsm_component_ns.class_('RikaGSMComponent', cg.Component)

# Action
RikaGsmSendCommandAction = rika_gsm_component_ns.class_("RikaGsmSendCommandAction", automation.Action)

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
    cv.Required(CONF_PIN): validate_stove_pin,
    cv.Optional(CONF_PHONE_NUMBER, default=DEFAULT_PHONE_NUMBER): validate_phone_number,
}).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_pin(config[CONF_PIN]))
    cg.add(var.set_phone_number(config[CONF_PHONE_NUMBER]))
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)


RIKA_GSM_SEND_COMMAND_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.use_id(RikaGSMComponent),
        cv.Required(CONF_COMMAND): cv.templatable(cv.string_strict),
    }
)

@automation.register_action(
    "rika_gsm.send_command", RikaGsmSendCommandAction, RIKA_GSM_SEND_COMMAND_SCHEMA
)
async def rika_gsm_send_command_to_code(config, action_id, template_arg, args):
    parent = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, parent)
    template_ = await cg.templatable(config[CONF_COMMAND], args, cg.std_string)
    cg.add(var.set_command(template_))
    return var