#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

#include "rika_gsm.h"

namespace esphome {
namespace rika_gsm {

static const char *const TAG = "rika_gsm";
static constexpr uint8_t ASCII_CR = 0x0D;
static constexpr uint8_t ASCII_LF = 0x0A;
static constexpr uint8_t ASCII_SUB = 0x1A;

void RikaGSMComponent::loop() {
  // check state
  if (this->state_ == State::STATE_INIT) {
    this->reset_state();
    this->set_state(State::READ_STOVE_AT_COMMAND);
  }

  // read bytes
  while (this->available()) {
    char byte = this->read();

    if (this->state_ == State::READ_STOVE_OUTGOING_SMS) {
      if ((byte == ASCII_SUB)) {
        this->set_state(State::STOVE_OUTGOING_SMS_COMPLETE);
        ESP_LOGD(TAG, this->raw_stove_status_.c_str());
        break;
      }
      this->raw_stove_status_ += byte;
    }

    if (this->state_ == State::READ_STOVE_AT_COMMAND) {
      if ((byte == ASCII_LF) || (byte == ASCII_SUB) || (byte == ASCII_CR)) {
        this->set_state(State::STOVE_AT_COMMAND_COMPLETE);
        ESP_LOGD(TAG, this->stove_request_.c_str());
        break;
      }
      this->stove_request_ += byte;
    }
  }

  this->update();
}

void RikaGSMComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "rika_gsm:");
  ESP_LOGCONFIG(TAG, "  pin: %s", this->pin_.c_str());
  ESP_LOGCONFIG(TAG, "  phone_number: %s", this->phone_number_.c_str());
#ifdef USE_TEXT_SENSOR
  if (this->raw_status_sensor_ != nullptr) {
    LOG_TEXT_SENSOR("  ", "raw_status", this->raw_status_sensor_);
  }
#endif
}

void RikaGSMComponent::send_sms(std::string const &message) {
  this->pending_sms_command_ = message;
  this->send_pending_ = true;
}

void RikaGSMComponent::set_pin(std::string const &pin) { this->pin_ = pin; }

void RikaGSMComponent::set_phone_number(std::string const &number) { this->phone_number_ = number; }

#ifdef USE_TEXT_SENSOR
void RikaGSMComponent::set_raw_status_sensor(text_sensor::TextSensor *raw_sensor) {
  this->raw_status_sensor_ = raw_sensor;
}
#endif

#ifdef USE_BINARY_SENSOR
void RikaGSMComponent::set_gsm_status_binary_sensor(binary_sensor::BinarySensor *gsm_sensor) {
  this->gsm_status_sensor_ = gsm_sensor;
}
#endif

void RikaGSMComponent::update() {
  uint32_t current_timestamp = millis();

  if (this->gsm_status_ && (current_timestamp - this->last_stove_request_) > (10 * 1000)) {
    this->gsm_status_ = false;
#ifdef USE_BINARY_SENSOR
    if (this->gsm_status_sensor_ != nullptr) {
      this->gsm_status_sensor_->publish_state(this->gsm_status_);
    }
#endif
  }

  if (this->state_ == State::STOVE_OUTGOING_SMS_COMPLETE) {
    ESP_LOGV(TAG, "Stove Reply: %s", this->raw_stove_status_.c_str());
#ifdef USE_TEXT_SENSOR
    if (this->raw_status_sensor_ != nullptr) {
      this->raw_status_sensor_->publish_state(this->raw_stove_status_);
    }
#endif
    this->set_state(State::STATE_INIT);
    return;
  }

  if (this->state_ != State::STOVE_AT_COMMAND_COMPLETE) {
    return;
  }

  AT_Command command = this->parse_command(this->stove_request_);
  ESP_LOGV(TAG, "Command %s: %d", this->stove_request_.c_str(), command);

  this->last_stove_request_ = current_timestamp;
  if (!this->gsm_status_) {
    this->gsm_status_ = true;
#ifdef USE_BINARY_SENSOR
    if (this->gsm_status_sensor_ != nullptr) {
      this->gsm_status_sensor_->publish_state(this->gsm_status_);
    }
#endif
  }

  switch (command) {
    case AT_Command::CMGR:
      ESP_LOGV(TAG, "Stove Request: Read sms");
      if (!this->send_pending_ || this->pending_sms_command_.size() == 0) {
        ESP_LOGV(TAG, "\t nothing to read");
        this->send_carriage_return();
        this->send_ok();
        this->set_state(State::STATE_INIT);
        return;
      }

      ESP_LOGV(TAG, "\t writing sms: %s", this->pending_sms_command_.c_str());
      this->send_query();
      this->set_state(State::STATE_INIT);
      return;
    case AT_Command::CMGS:
      this->send_carriage_return();
      this->write_str("> ");
      this->set_state(State::READ_STOVE_OUTGOING_SMS);
      return;
    case AT_Command::CMGD:
      this->reset_pending_query();
      this->send_ok();
      this->set_state(State::STATE_INIT);
      return;
    case AT_Command::ATE:
    case AT_Command::AT:
    case AT_Command::ATF:
    case AT_Command::CNMI:
    case AT_Command::CMGF:
    case AT_Command::IPR:
      ESP_LOGV(TAG, "Stove Request: configuration\n\t %s", this->stove_request_.c_str());
      this->send_ok();
      this->set_state(State::STATE_INIT);
      return;
    case AT_Command::UNKNOWN:
    default:
      this->send_ok();
      this->set_state(State::STATE_INIT);
      return;
  }
}

void RikaGSMComponent::send_ok() {
  this->write_str("OK");
  this->send_carriage_return();
}

void RikaGSMComponent::send_carriage_return() {
  this->write_byte(ASCII_CR);
  this->write_byte(ASCII_LF);
}

void RikaGSMComponent::send_query() {
  this->send_carriage_return();
  this->write_str("+CMGR:\"REC READ\",\"");
  this->write_str(this->phone_number_.c_str());
  this->write_str("\",,\"");
  this->write_str("70/01/01,01:00:00+0");
  this->write_str("\"");
  this->send_carriage_return();
  this->write_str(this->pin_.c_str());
  this->write_str(" ");
  this->write_str(this->pending_sms_command_.c_str());
  this->send_carriage_return();
  this->send_carriage_return();
  this->send_ok();
}

void RikaGSMComponent::reset_pending_query() {
  this->pending_sms_command_ = "";
  this->send_pending_ = false;
}

void RikaGSMComponent::reset_stove_request() { this->stove_request_ = ""; }

void RikaGSMComponent::set_state(State state) {
  this->state_ = state;
  ESP_LOGV(TAG, "State: %s", this->state_to_string(this->state_).c_str());
}

void RikaGSMComponent::reset_state() {
  this->reset_stove_request();
  this->raw_stove_status_ = "";
}

AT_Command RikaGSMComponent::parse_command(std::string const &command) const {
  if (esphome::str_startswith(command, "AT+CMGR"))
    return AT_Command::CMGR;
  if (esphome::str_startswith(command, "AT+CMGS"))
    return AT_Command::CMGS;
  if (esphome::str_startswith(command, "AT+CMGD"))
    return AT_Command::CMGD;
  if (command == "AT")
    return AT_Command::AT;
  if (command == "AT&F")
    return AT_Command::ATF;
  if (esphome::str_startswith(command, "AT+CNMI"))
    return AT_Command::CNMI;
  if (esphome::str_startswith(command, "AT+CMGF"))
    return AT_Command::CMGF;
  if (esphome::str_startswith(command, "AT+IPR"))
    return AT_Command::IPR;
  if (esphome::str_startswith(command, "ATE"))
    return AT_Command::ATE;

  return AT_Command::UNKNOWN;
}

std::string RikaGSMComponent::state_to_string(State state) const {
  switch (state) {
    case State::STATE_INIT:
      return "STATE_INIT";
    case State::READ_STOVE_AT_COMMAND:
      return "READ_STOVE_AT_COMMAND";
    case State::STOVE_AT_COMMAND_COMPLETE:
      return "STOVE_AT_COMMAND_COMPLETE";
    case State::READ_STOVE_OUTGOING_SMS:
      return "READ_STOVE_OUTGOING_SMS";
    case State::STOVE_OUTGOING_SMS_COMPLETE:
      return "STOVE_OUTGOING_SMS_COMPLETE";
  }
  return "UNKNOWN_STATE";
}

}  // end namespace rika_gsm
}  // end namespace esphome