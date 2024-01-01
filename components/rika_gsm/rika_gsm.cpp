#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "rika_gsm.h"

namespace esphome {
namespace rika_gsm {

static const char *const TAG = "rika_gsm";
const uint8_t ASCII_CR = 0x0D;
const uint8_t ASCII_LF = 0x0A;
const uint8_t ASCII_SUB = 0x1A;

void RikaGSMComponent::update() {}

void RikaGSMComponent::loop() {
  // check state
  ESP_LOGW(TAG, "State: %d", this->state_);
  if (this->state_ == State::STATE_INIT) {
    this->reset_stove_request();
    this->state_ = State::AWAIT_STOVE_REQUEST;
  }

  // read bytes
  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    this->stove_request_ += byte;

    if ((byte == '\n') || (byte == ASCII_SUB) || (byte == ASCII_CR)) {
      this->stove_request_complete_ = true;
      break;
    }
  }
}

void RikaGSMComponent::dump_config() {}

void RikaGSMComponent::send_sms(std::string const &message) {
  this->outgoing_message_ = message;
  this->send_pending_ = true;
}

void RikaGSMComponent::set_pin(std::string const &pin) { this->pin_ = pin; }

void RikaGSMComponent::set_phone_number(std::string const &number) { this->phone_number_ = number; }

void RikaGSMComponent::set_time(time::RealTimeClock *time) { this->time_ = time; }

void RikaGSMComponent::parse_stove_request() {
  if (!this->stove_request_complete_)
    return;

  if (this->state_ == State::AWAIT_STOVE_REPLY) {
    ESP_LOGW(TAG, "Stove Reply: %s", this->stove_request_.c_str());
    this->state_ = State::STATE_INIT;
    return;
  }
    ESP_LOGW(TAG, "Stove Request: %s", this->stove_request_.c_str());

  if (esphome::str_startswith(this->stove_request_, "AT+CMGR")) {  // the stove wants to read an sms
    this->state_ = State::STATE_STOVE_READ;
    ESP_LOGW(TAG, "Stove Request: Read sms");
    if (!this->send_pending_ || this->outgoing_message_.size() == 0) {
      ESP_LOGW(TAG, "\t nothing to read");
      this->send_carriage_return();
      this->send_ok();
      this->state_ = State::STATE_INIT;
    }

    ESP_LOGV(TAG, "\t writing sms: %s", this->outgoing_message_.c_str());
    this->send_query();
    this->reset_pending_query();
    this->state_ = State::AWAIT_STOVE_REPLY;
    return;
  }
  if (esphome::str_startswith(this->stove_request_, "AT+CMGD")) {  // the stove wants to delete the SMS
    ESP_LOGW(TAG, "Stove Request: Delete sms");
    this->reset_pending_query();
    this->send_ok();
    return;
  }
  if (esphome::str_startswith(this->stove_request_, "ATE0") ||
      esphome::str_startswith(this->stove_request_, "AT+CNMI") ||
      esphome::str_startswith(this->stove_request_, "AT+CMGF")) {  // configuration request
    ESP_LOGW(TAG, "Stove Request: configuration\n\t %s", this->stove_request_.c_str());
    this->send_ok();
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
  this->write_str("+CMGR: \"REC UNREAD\",\"");
  this->write_str(this->phone_number_.c_str());
  this->write_str("\",,\"");
  this->write_str(this->time_->now().strftime("%y/%m/%d,%X+0").c_str());
  this->write_str("\"");
  this->send_carriage_return();
  this->write_str(this->pin_.c_str());
  this->write_str(" ");
  this->write_str(this->outgoing_message_.c_str());
  this->send_carriage_return();
  this->send_carriage_return();
  this->send_ok();
}

void RikaGSMComponent::reset_pending_query() {
  this->outgoing_message_ = "";
  this->send_pending_ = false;
}

void RikaGSMComponent::reset_stove_request() {
  this->stove_request_ = "";
  this->stove_request_complete_ = false;
}
}  // end namespace rika_gsm
}  // end namespace esphome