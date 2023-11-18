#include "esphome/core/log.h"
#include "rika_gsm.h"

namespace esphome {
namespace rika_gsm {

static const char *const TAG = "rika_gsm";

void RikaGSMComponent::update() {}

void RikaGSMComponent::loop() {}

void RikaGSMComponent::dump_config() {}

void RikaGSMComponent::set_pin(std::string pin) { this->pin_ = pin; }

void RikaGSMComponent::set_time(time::RealTimeClock *time) { this->time_ = time; }

}  // end namespace rika_gsm
}  // end namespace esphome