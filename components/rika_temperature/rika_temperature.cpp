#include "esphome/core/log.h"

#include "rika_temperature.h"

namespace esphome {
namespace rika_temperature {

static const char *const TAG = "rika_temperature";
static constexpr uint8_t ASCII_CR = 0x0D;
static constexpr uint8_t ASCII_LF = 0x0A;
static constexpr uint8_t ASCII_SUB = 0x1A;

void RikaTemperatureComponent::update() {}

void RikaTemperatureComponent::loop() {
  while (this->available()) {
    char byte = this->read();
    this->read_buffer_ += byte;

    if ((byte == ASCII_LF) || (byte == ASCII_SUB) || (byte == ASCII_CR)) {
      ESP_LOGD(TAG, "ending char %d", byte);
      this->read_buffer_ = "";
    }
  }
  ESP_LOGD(TAG, this->read_buffer_.c_str());
}

void RikaTemperatureComponent::dump_config() {}

}  // end namespace rika_temperature
}  // end namespace esphome
