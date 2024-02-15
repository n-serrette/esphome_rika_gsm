import re
from esphome import automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import (
    CONF_ID,
)

DEPENDENCIES = ['uart']

CONF_RIKA_TEMPERATURE_ID = "rika_temperature_id"

rika_temperature_component_ns = cg.esphome_ns.namespace('rika_temperature')
RikaTemperatureComponent = rika_temperature_component_ns.class_('RikaTemperatureComponent', cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(RikaTemperatureComponent),
}).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
