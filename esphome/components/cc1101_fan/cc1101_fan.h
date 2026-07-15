#pragma once

#include <map>
#include <string>

#include "esphome/core/component.h"

#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <RCSwitch.h>

namespace esphome {
namespace cc1101_fan {

enum FanSpeed : uint8_t {
  FAN_SPEED_OFF = 0,
  FAN_SPEED_LOW = 1,
  FAN_SPEED_MEDIUM = 2,
  FAN_SPEED_HIGH = 3,
};

class CC1101FanFanOutput;
class CC1101FanLightOutput;

// Owns the CC1101 transceiver + RCSwitch protocol-11 encode/decode, ported from
// the original homefans.ino. Registered Fan/Light entities are looked up by their
// 4-bit DIP fan id whenever a frame from the physical remote is decoded.
class CC1101FanHub : public Component {
 public:
  void set_rx_pin(uint8_t pin) { this->rx_pin_ = pin; }
  void set_tx_pin(uint8_t pin) { this->tx_pin_ = pin; }
  void set_frequency(float mhz) { this->frequency_ = mhz; }
  void set_protocol(int protocol) { this->protocol_ = protocol; }
  void set_repeats(int repeats) { this->repeats_ = repeats; }
  void set_pulse_length(int pulse_length) { this->pulse_length_ = pulse_length; }

  void register_fan(uint8_t fan_id, CC1101FanFanOutput *fan) { this->fans_[fan_id] = fan; }
  void register_light(uint8_t fan_id, CC1101FanLightOutput *light) { this->lights_[fan_id] = light; }

  void setup() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

  // Encodes and RF-transmits an attr ("on"/"speed"/"light") + payload command for a fan id.
  // Follows the same switch-to-TX / send / switch-back-to-RX pattern as the original firmware
  // so passive RF receive is never left disabled.
  void send_command(uint8_t fan_id, const std::string &attr, const std::string &payload);

 protected:
  uint32_t generate_command_(uint8_t fan_id, const std::string &attr, const std::string &payload);
  void handle_received_frame_();

  uint8_t rx_pin_{4};
  uint8_t tx_pin_{5};
  float frequency_{303.875f};
  int protocol_{11};
  int repeats_{8};
  int pulse_length_{320};

  RCSwitch switch_{};
  std::map<uint8_t, CC1101FanFanOutput *> fans_{};
  std::map<uint8_t, CC1101FanLightOutput *> lights_{};
};

}  // namespace cc1101_fan
}  // namespace esphome
