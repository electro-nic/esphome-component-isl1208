#include "isl1208.h"
#include "esphome/core/log.h"

// Datasheet:
// - https://www.renesas.com/en/document/dst/isl1208-datasheet

namespace esphome {
namespace isl1208 {

static const char *const TAG = "isl1208";

void ISL1208Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ISL1208...");
  if (!this->read_rtc_()) {
    this->mark_failed();
  }
}

void ISL1208Component::update() { this->read_time(); }

void ISL1208Component::dump_config() {
  ESP_LOGCONFIG(TAG, "ISL1208:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with ISL1208 failed!");
  }
  ESP_LOGCONFIG(TAG, "  Timezone: '%s'", this->timezone_.c_str());
}

float ISL1208Component::get_setup_priority() const { return setup_priority::DATA; }

void ISL1208Component::read_time() {
  if (!this->read_rtc_()) {
    return;
  }
  if (isl1208_.reg.ch) {
    ESP_LOGW(TAG, "RTC halted, not syncing to system clock.");
    return;
  }
  ESPTime rtc_time{
      .second = uint8_t(isl1208_.reg.second + 10 * isl1208_.reg.second_10),
      .minute = uint8_t(isl1208_.reg.minute + 10u * isl1208_.reg.minute_10),
      .hour = uint8_t(isl1208_.reg.hour + 10u * isl1208_.reg.hour_10),
      .day_of_week = uint8_t(isl1208_.reg.weekday),
      .day_of_month = uint8_t(isl1208_.reg.day + 10u * isl1208_.reg.day_10),
      .day_of_year = 1,  // ignored by recalc_timestamp_utc(false)
      .month = uint8_t(isl1208_.reg.month + 10u * isl1208_.reg.month_10),
      .year = uint16_t(isl1208_.reg.year + 10u * isl1208_.reg.year_10 + 2000),
      .is_dst = false,  // not used
      .timestamp = 0    // overwritten by recalc_timestamp_utc(false)
  };
  rtc_time.recalc_timestamp_utc(false);
  if (!rtc_time.is_valid()) {
    ESP_LOGE(TAG, "Invalid RTC time, not syncing to system clock.");
    return;
  }
  time::RealTimeClock::synchronize_epoch_(rtc_time.timestamp);
}

void ISL1208Component::write_time() {
  auto now = time::RealTimeClock::utcnow();
  if (!now.is_valid()) {
    ESP_LOGE(TAG, "Invalid system time, not syncing to RTC.");
    return;
  }
  isl1208_.reg.year = (now.year - 2000) % 10;
  isl1208_.reg.year_10 = (now.year - 2000) / 10 % 10;
  isl1208_.reg.month = now.month % 10;
  isl1208_.reg.month_10 = now.month / 10;
  isl1208_.reg.day = now.day_of_month % 10;
  isl1208_.reg.day_10 = now.day_of_month / 10;
  isl1208_.reg.weekday = now.day_of_week;
  isl1208_.reg.hour = now.hour % 10;
  isl1208_.reg.hour_10 = now.hour / 10;
  isl1208_.reg.minute = now.minute % 10;
  isl1208_.reg.minute_10 = now.minute / 10;
  isl1208_.reg.second = now.second % 10;
  isl1208_.reg.second_10 = now.second / 10;
  isl1208_.reg.ch = false;

  this->write_rtc_();
}

bool ISL1208Component::read_rtc_() {
  if (!this->read_bytes(0, this->isl1208_.raw, sizeof(this->isl1208_.raw))) {
    ESP_LOGE(TAG, "Can't read I2C data.");
    return false;
  }
  ESP_LOGD(TAG, "Read  %0u%0u:%0u%0u:%0u%0u 20%0u%0u-%0u%0u-%0u%0u  CH:%s RS:%0u SQWE:%s OUT:%s", isl1208_.reg.hour_10,
           isl1208_.reg.hour, isl1208_.reg.minute_10, isl1208_.reg.minute, isl1208_.reg.second_10, isl1208_.reg.second,
           isl1208_.reg.year_10, isl1208_.reg.year, isl1208_.reg.month_10, isl1208_.reg.month, isl1208_.reg.day_10,
           isl1208_.reg.day, ONOFF(isl1208_.reg.ch), isl1208_.reg.rs, ONOFF(isl1208_.reg.sqwe), ONOFF(isl1208_.reg.out));

  return true;
}

bool ISL1208Component::write_rtc_() {
  if (!this->write_bytes(0, this->isl1208_.raw, sizeof(this->isl1208_.raw))) {
    ESP_LOGE(TAG, "Can't write I2C data.");
    return false;
  }
  ESP_LOGD(TAG, "Write %0u%0u:%0u%0u:%0u%0u 20%0u%0u-%0u%0u-%0u%0u  CH:%s RS:%0u SQWE:%s OUT:%s", isl1208_.reg.hour_10,
           isl1208_.reg.hour, isl1208_.reg.minute_10, isl1208_.reg.minute, isl1208_.reg.second_10, isl1208_.reg.second,
           isl1208_.reg.year_10, isl1208_.reg.year, isl1208_.reg.month_10, isl1208_.reg.month, isl1208_.reg.day_10,
           isl1208_.reg.day, ONOFF(isl1208_.reg.ch), isl1208_.reg.rs, ONOFF(isl1208_.reg.sqwe), ONOFF(isl1208_.reg.out));
  return true;
}
}  // namespace isl1208
}  // namespace esphome
