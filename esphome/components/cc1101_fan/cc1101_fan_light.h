#pragma once

#include "esphome/components/light/light_output.h"

#include "cc1101_fan.h"

namespace esphome {
namespace cc1101_fan {

class CC1101FanLightOutput : public light::LightOutput {
 public:
  void set_hub(CC1101FanHub *hub) { this->hub_ = hub; }
  void set_fan_id(uint8_t fan_id) { this->fan_id_ = fan_id; }

  light::LightTraits get_traits() override {
    auto traits = light::LightTraits();
    traits.set_supported_color_modes({light::ColorMode::ON_OFF});
    return traits;
  }

  void write_state(light::LightState *state) override {
    this->state_ = state;

    // Suppressed when this write was triggered by update_from_rf() below, so we don't
    // echo a command the physical remote just sent back out over RF.
    if (this->suppress_next_write_) {
      this->suppress_next_write_ = false;
      return;
    }

    bool binary;
    state->current_values_as_binary(&binary);
    this->hub_->send_command(this->fan_id_, "light", binary ? "on" : "off");
  }

  // Called by the hub when the physical remote is used, to reflect the new state in
  // Home Assistant without re-transmitting it (avoids an RF echo loop).
  void update_from_rf(bool on) {
    if (this->state_ == nullptr)
      return;
    this->suppress_next_write_ = true;
    this->state_->make_call().set_state(on).set_transition_length(0).perform();
  }

 protected:
  CC1101FanHub *hub_{nullptr};
  uint8_t fan_id_{0};
  light::LightState *state_{nullptr};
  bool suppress_next_write_{false};
};

}  // namespace cc1101_fan
}  // namespace esphome
