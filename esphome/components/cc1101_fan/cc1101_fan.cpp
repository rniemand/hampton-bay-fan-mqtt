#include "cc1101_fan.h"
#include "cc1101_fan_fan.h"
#include "cc1101_fan_light.h"

namespace esphome {
namespace cc1101_fan {

void CC1101FanHub::setup() {
  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.setMHZ(this->frequency_);
  ELECHOUSE_cc1101.SetRx();
  this->switch_.enableReceive(this->rx_pin_);
}

void CC1101FanHub::loop() {
  if (!this->switch_.available())
    return;

  this->handle_received_frame_();
  this->switch_.resetAvailable();
}

uint32_t CC1101FanHub::generate_command_(uint8_t fan_id, const std::string &attr, const std::string &payload) {
  uint32_t base_command = 0b111111000110000000000000;
  uint32_t fan_id_dips = (fan_id ^ 0b1111) << 8;
  uint32_t command_mode = 0b01110000;
  uint32_t command = 0b0000;

  if (attr == "speed") {
    if (payload == "low") {
      command = 0b0110;
    } else if (payload == "medium") {
      command = 0b0101;
    } else if (payload == "high") {
      command = 0b0100;
    } else if (payload == "off") {
      command = 0b0111;
    }
  } else if (attr == "light") {
    if (payload == "on") {
      command = 0b0010;
    } else if (payload == "off") {
      command = 0b0001;
    }
  } else if (attr == "on") {
    if (payload == "on") {
      command = 0b1110;
    } else if (payload == "off") {
      command = 0b0111;
    }
  }

  return base_command + fan_id_dips + command_mode + command;
}

void CC1101FanHub::send_command(uint8_t fan_id, const std::string &attr, const std::string &payload) {
  ELECHOUSE_cc1101.SetTx();
  this->switch_.disableReceive();
  this->switch_.enableTransmit(this->tx_pin_);
  this->switch_.setRepeatTransmit(this->repeats_);
  this->switch_.setProtocol(this->protocol_);
  this->switch_.setPulseLength(this->pulse_length_);

  uint32_t rf_command = this->generate_command_(fan_id, attr, payload);
  this->switch_.send(rf_command, 24);

  ELECHOUSE_cc1101.SetRx();
  this->switch_.disableTransmit();
  this->switch_.enableReceive(this->rx_pin_);
}

void CC1101FanHub::handle_received_frame_() {
  unsigned long value = this->switch_.getReceivedValue();
  unsigned int prot = this->switch_.getReceivedProtocol();
  unsigned int bits = this->switch_.getReceivedBitlength();

  if (prot != 11 || bits != 24)
    return;

  uint32_t subtracted = value - 0b111111000110000000000000;
  uint32_t truncated = subtracted >> 8;
  uint8_t id = (truncated ^ 0b1111) & 0b1111;

  // cmdMode: (1) Normal commands (on/off/speed/light) - (0) light color/temperature, not yet handled
  int cmd_mode = ((subtracted & 0b000011110000) >> 4) % 6;
  int command = subtracted & 0b000000001111;

  if (cmd_mode != 1)
    return;

  auto fan_it = this->fans_.find(id);
  auto light_it = this->lights_.find(id);

  if (command == 7) {  // Fan OFF
    if (fan_it != this->fans_.end())
      fan_it->second->update_from_rf(false, FAN_SPEED_OFF);
  } else if (command == 6) {  // Fan Speed 1
    if (fan_it != this->fans_.end())
      fan_it->second->update_from_rf(true, FAN_SPEED_LOW);
  } else if (command == 5) {  // Fan Speed 2
    if (fan_it != this->fans_.end())
      fan_it->second->update_from_rf(true, FAN_SPEED_MEDIUM);
  } else if (command == 4) {  // Fan Speed 3
    if (fan_it != this->fans_.end())
      fan_it->second->update_from_rf(true, FAN_SPEED_HIGH);
  } else if (command == 1) {  // Light OFF
    if (light_it != this->lights_.end())
      light_it->second->update_from_rf(false);
  } else if (command == 2) {  // Light ON
    if (light_it != this->lights_.end())
      light_it->second->update_from_rf(true);
  } else if (command == 13) {  // All Off
    if (fan_it != this->fans_.end())
      fan_it->second->update_from_rf(false, FAN_SPEED_OFF);
    if (light_it != this->lights_.end())
      light_it->second->update_from_rf(false);
  } else if (command == 14) {  // On From Off (speed unchanged)
    if (fan_it != this->fans_.end())
      fan_it->second->update_from_rf(true, fan_it->second->speed);
  }
}

}  // namespace cc1101_fan
}  // namespace esphome
