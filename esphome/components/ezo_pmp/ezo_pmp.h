#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace ezo_pmp {

/// This class implements support for the EZO circuits in i2c mode
class EZOPMPSensor : public sensor::Sensor, public PollingComponent, public i2c::I2CDevice {
 public:
  void loop() override;
  void dump_config() override;
  void update() override;
  float get_setup_priority() const override { return setup_priority::DATA; };

  void dispense_ml(std::string &cmd);  
  void send_command(std::string &cmd); // command to pass to the EZO device

 protected:
  uint32_t start_time_ = 0;
  uint32_t wait_time_ = 0;
  uint16_t state_ = 0;
  const char *dispense_ml_;
  const char *command_;
};

}  // namespace ezo_pmp
}  // namespace esphome
