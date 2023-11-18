#pragma once
#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/time/real_time_clock.h"

namespace esphome {
namespace rika_gsm {
class RikaGSMComponent : public uart::UARTDevice, public PollingComponent {
 public:
  void update() override;
  void loop() override;
  void dump_config() override;

  void set_pin(std::string);
  void set_time(time::RealTimeClock *);

 protected:
  std::string pin_;
  time::RealTimeClock *time_;
};
}  // end namespace rika_gsm
}  // end namespace esphome
