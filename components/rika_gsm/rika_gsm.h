#pragma once
#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace rika_gsm {

enum State {
  STATE_INIT = 0,
  READ_STOVE_AT_COMMAND,
  STOVE_AT_COMMAND_COMPLETE,
  READ_STOVE_OUTGOING_SMS,
  STOVE_OUTGOING_SMS_COMPLETE
};
enum AT_Command { AT, ATE, CNMI, CMGF, IPR, ATF, CMGD, CMGR, CMGS, UNKNOWN};


class RikaGSMComponent : public uart::UARTDevice, public PollingComponent {
 public:
  void update() override;
  void loop() override;
  void dump_config() override;

  void send_sms(std::string const &message);
  void set_pin(std::string const &);
  void set_time(time::RealTimeClock *);
  void set_phone_number(std::string const &);
  void set_raw_status_sensor(text_sensor::TextSensor *);
  void set_gsm_status_binary_sensor(binary_sensor::BinarySensor *);

 protected:
  State state_{State::STATE_INIT};
  std::string pin_;
  std::string phone_number_;
  time::RealTimeClock *time_;
  std::string pending_sms_command_;
  bool send_pending_;
  std::string stove_request_;
  std::string raw_stove_status_;
  text_sensor::TextSensor *raw_status_sensor_{nullptr};
  bool gsm_status_{false};
  binary_sensor::BinarySensor *gsm_status_sensor_{nullptr};

  void send_ok();
  void send_carriage_return();
  void send_query();
  void reset_pending_query();
  void reset_stove_request();
  void set_state(State);
  void reset_state();

  AT_Command parse_command(std::string const &) const;
  std::string state_to_string(State) const;
};

template<typename... Ts> class RikaGsmSendCommandAction : public Action<Ts...> {
 public:
  RikaGsmSendCommandAction(RikaGSMComponent *parent) : parent_(parent) {}
  TEMPLATABLE_VALUE(std::string, command)

  void play(Ts... x) {
    auto command = this->command_.value(x...);
    this->parent_->send_sms(command);
  }

 protected:
  RikaGSMComponent *parent_;
};

}  // end namespace rika_gsm
}  // end namespace esphome
