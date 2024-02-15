#pragma once
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace rika_temperature {

class RikaTemperatureComponent : public uart::UARTDevice, public PollingComponent {
 public:
  void update() override;
  void loop() override;
  void dump_config() override;

 protected:
  std::string read_buffer_;
};

}  // end namespace rika_temperature
}  // end namespace esphome
