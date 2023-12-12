
#include <stdint.h>
#include <time.h>

#include "TFT.hpp"
#include "TFTStatus.hpp"

#include "GrundschriftBold30.h"
#include "GrundschriftRegular15.h"


TFTStatus::TFTStatus(TFT &tft)
    : tft {tft}
{

}

    void setup(std::string title, int max = 5);
    void update();

    void set(std::string line);
    void push(std::string line);


// =========================================================================
// setup
// =========================================================================
void TFTStatus::setup(const String &title, uint8_t max) 
{
  this->max = max;
  this->title = title;
  this->update(Section::ALL);
}

void TFTStatus::update(Section section)
{
  Serial.printf("%s: section=%d\n", __PRETTY_FUNCTION__, section);
  if ((section & Section::BORDER) == Section::ALL)
    section = Section::ALL;

  TFT_eSPI &tft = this->tft.getHandle();

  if (section & Section::BORDER)
  {
    tft.fillScreen(TFT_LIGHTGREY);
    tft.drawSmoothRoundRect(30, 30, 30, 30, 179, 179, TFT_BLACK, TFT_LIGHTGREY);
    tft.fillSmoothRoundRect(31, 31, 178, 178, 29, TFT_WHITE, TFT_BLACK);
  }
  if (section & Section::TITLE)
  {
    tft.unloadFont();
    tft.loadFont(GrundschriftBold30);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.setTextDatum(TC_DATUM);
    tft.drawString(this->title, 120, 36);
    tft.unloadFont();
  }
  if (section & Section::TEXT)
  {
    tft.unloadFont();
    tft.loadFont(GrundschriftRegular15);
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.setTextDatum(TL_DATUM);
    tft.setViewport(40, 76, 160, this->max * 22, false);
    tft.fillRect(40, 76, 160, this->max * 22, TFT_WHITE);
    {
      int i = 0;
      for (const String &line : this->buffer)
        tft.drawString(line, 40, 76 + (i++*22));
    }
    tft.resetViewport();
  }
}

void TFTStatus::setTitle(const String &title)
{
  this->title = title;
  this->update(Section::TITLE);
}
  
void TFTStatus::set(const String &line)
{
  this->buffer.clear();
  this->buffer.emplace_back(line);
  this->update(Section::TEXT);
}

void TFTStatus::push(const String &line)
{
  this->buffer.emplace_back(line);
  while (this->buffer.size() > max)
    this->buffer.pop_front();
  this->update(Section::TEXT);
}
