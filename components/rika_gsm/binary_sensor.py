import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import (
    DEVICE_CLASS_CONNECTIVITY,
    ENTITY_CATEGORY_DIAGNOSTIC,
)
from . import CONF_RIKA_GSM_ID, RikaGSMComponent

DEPENDENCIES = ["rika_gsm"]

CONF_GSM_STATUS = "gsm_status"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_RIKA_GSM_ID): cv.use_id(RikaGSMComponent),
    cv.Optional(CONF_GSM_STATUS): binary_sensor.binary_sensor_schema(
        device_class=DEVICE_CLASS_CONNECTIVITY,
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
    ),
}


async def to_code(config):
    rika_gsm_component = await cg.get_variable(config[CONF_RIKA_GSM_ID])

    if CONF_GSM_STATUS in config:
        sens = await binary_sensor.new_binary_sensor(config[CONF_GSM_STATUS])
        cg.add(rika_gsm_component.set_gsm_status_binary_sensor(sens))