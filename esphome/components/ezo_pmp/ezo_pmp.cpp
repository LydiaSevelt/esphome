#include "ezo_pmp.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace ezo_pmp {

static const char *const TAG = "ezo_pmp.sensor";

static const uint16_t EZO_STATE_WAIT = 1;
static const uint16_t EZO_STATE_SEND_DISPENSE_ML = 2;
static const uint16_t EZO_STATE_WAIT_DISPENSE_ML = 4;

void EZOPMPSensor::dump_config() {
  LOG_SENSOR("", "EZO_PMP", this);
  LOG_I2C_DEVICE(this);
  if (this->is_failed())
    ESP_LOGE(TAG, "Communication with EZO_PMP circuit failed!");
  LOG_UPDATE_INTERVAL(this);
}

void EZOPMPSensor::update() {
  if (this->state_ & EZO_STATE_WAIT) {
    ESP_LOGE(TAG, "update overrun, still waiting for previous response");
    return;
  }
  uint8_t c = 'R';
  this->write(&c, 1);
  this->state_ |= EZO_STATE_WAIT;
  this->start_time_ = millis();
  this->wait_time_ = 900;
}

void EZOPMPSensor::loop() {
  uint8_t buf[21];
  if (!(this->state_ & EZO_STATE_WAIT)) {
//    if (this->state_ & EZO_STATE_SEND_DISPENSE_ML) {
//      int len = sprintf((char *) buf, "D,%0.3f", this->dispense_ml_);
//      this->write(buf, len);
//      this->state_ = EZO_STATE_WAIT | EZO_STATE_WAIT_DISPENSE_ML;
//      this->start_time_ = millis();
//      this->wait_time_ = 300;
//    }
    return;
  }
  if (millis() - this->start_time_ < this->wait_time_)
    return;
  buf[0] = 0;
  if (!this->read_bytes_raw(buf, 20)) {
    ESP_LOGE(TAG, "read error");
    this->state_ = 0;
    return;
  }
  switch (buf[0]) {
    case 1:
      break;
    case 2:
      ESP_LOGE(TAG, "device returned a syntax error");
      break;
    case 254:
      return;  // keep waiting
    case 255:
      ESP_LOGE(TAG, "device returned no data");
      break;
    default:
      ESP_LOGE(TAG, "device returned an unknown response: %d", buf[0]);
      break;
  }
  if (this->state_ & EZO_STATE_WAIT_DISPENSE_ML) {
    this->state_ = 0;
    return;
  }
  this->state_ &= ~EZO_STATE_WAIT;
  if (buf[0] != 1)
    return;

  // some sensors return multiple comma-separated values, terminate string after first one
  //for (size_t i = 1; i < sizeof(buf) - 1; i++)
  //  if (buf[i] == ',')
  //    buf[i] = '\0';

  float val = parse_number<float>((char *) &buf[1]).value_or(0);
  this->publish_state(val);
}

//void EZOPMPSensor::dispense_ml(float temp) {
//  this->dispense_ml_ = temp;
//  this->state_ |= EZO_STATE_SEND_DISPENSE_ML;
//}

}  // namespace ezo_pmp
}  // namespace esphome
