#pragma once

#include "esphome/components/fan/fan.h"
#include "esphome/core/component.h"

#include "cc1101_fan.h"

namespace esphome {
namespace cc1101_fan {

class CC1101FanFanOutput : public Component, public fan::Fan {
 public:
  void set_hub(CC1101FanHub *hub) { this->hub_ = hub; }
  void set_fan_id(uint8_t fan_id) { this->fan_id_ = fan_id; }

  fan::FanTraits get_traits() override { return fan::FanTraits(false, true, false, 3); }

  // Called by the hub when the physical remote is used, to reflect the new state in
  // Home Assistant without re-transmitting it (avoids an RF echo loop).
  void update_from_rf(bool state, uint8_t speed) {
    this->state = state;
    this->speed = speed;
    this->publish_state();
  }

 protected:
  void control(const fan::FanCall &call) override {
    if (call.get_state().has_value())
      this->state = *call.get_state();
    if (call.get_speed().has_value())
      this->speed = *call.get_speed();

    if (!this->state) {
      this->hub_->send_command(this->fan_id_, "on", "off");
    } else if (this->speed == FAN_SPEED_LOW) {
      this->hub_->send_command(this->fan_id_, "speed", "low");
    } else if (this->speed == FAN_SPEED_MEDIUM) {
      this->hub_->send_command(this->fan_id_, "speed", "medium");
    } else if (this->speed == FAN_SPEED_HIGH) {
      this->hub_->send_command(this->fan_id_, "speed", "high");
    } else {
      this->hub_->send_command(this->fan_id_, "on", "on");
    }

    this->publish_state();
  }

  CC1101FanHub *hub_{nullptr};
  uint8_t fan_id_{0};
};

}  // namespace cc1101_fan
}  // namespace esphome
