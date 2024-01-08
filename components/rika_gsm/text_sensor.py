import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import (
    ENTITY_CATEGORY_DIAGNOSTIC,
)
from . import CONF_RIKA_GSM_ID, RikaGSMComponent

DEPENDENCIES = ["rika_gsm"]

CONF_RAW_STATUS = "raw_status"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_RIKA_GSM_ID): cv.use_id(RikaGSMComponent),
    cv.Optional(CONF_RAW_STATUS): text_sensor.text_sensor_schema(
        entity_category=ENTITY_CATEGORY_DIAGNOSTIC
    ),
}

async def to_code(config):
    rika_gsm_component = await cg.get_variable(config[CONF_RIKA_GSM_ID])

    if CONF_RAW_STATUS in config:
        sens = await text_sensor.new_text_sensor(config[CONF_RAW_STATUS])
        cg.add(rika_gsm_component.set_raw_status_sensor(sens))