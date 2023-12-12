
#include "TFT.hpp"

TFT::TFT(uint8_t brightness)
  : brightness {std::min(brightness, (uint8_t)100)}
{
  
}

TFT_eSPI &TFT::getHandle()
{
  return this->tft;
}

void TFT::setBrightness(uint8_t brightness)
{
  if (this->brightness != brightness)
  {
    this->brightness = std::min(brightness, (uint8_t)100);
    analogWrite(TFT_BKL, this->brightness);
  }
}

void TFT::enableBacklight(bool on)
{
  analogWrite(TFT_BKL, this->brightness);
}

void TFT::setup()
{
  Serial.println("Initializing TFT...");

  pinMode(TFT_BKL, OUTPUT);
  analogWrite(TFT_BKL, this->brightness);
  this->enableBacklight();


  // Initialise the screen
  tft.init();

  // Ideally set orientation for good viewing angle range because
  // the anti-aliasing effectiveness varies with screen viewing angle
  // Usually this is when screen ribbon connector is at the bottom
  tft.setRotation(0);
  tft.fillScreen(TFT_BLACK);
}
