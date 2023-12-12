#pragma once

#ifndef SETTINGS_HPP_
#define SETTINGS_HPP_

struct Settings
{
  // uint8_t TFT_MOSI;
  // uint8_t TFT_SCLK;
  // uint8_t TFT_CS;
  // uint8_t TFT_DC;
  // uint8_t TFT_RST;
  // uint32_t TFT_SPI_FREQUENCY;

  uint8_t BUTTON_PIN;

  uint8_t ROTARY_PIN1;
  uint8_t ROTARY_PIN2;
  uint8_t ROTARY_CLICKS_PER_STEP;

  uint8_t MRFC522_SS;
  uint8_t MRFC522_RST;
  uint32_t MFRC522_SPI_FREQUENCY;

  uint8_t DFMP3_TX;
  uint8_t DFMP3_RX;
  uint8_t DFMP3_BUSY;

  String MQTT_HOST;
  uint16_t MQTT_PORT;
  String MQTT_USER;
  String MQTT_PASS;

};

#endif /* SETTINGS_HPP_ */