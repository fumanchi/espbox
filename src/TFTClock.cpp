
#include <stdint.h>
#include <time.h>

#include "TFT.hpp"
#include "TFTClock.hpp"

// #include "NotoSansBold15.h"
// #include "openiconic28.h"
// #include "NotoSansRegular28.h"
// #include "NotoSansBold28.h"
#include "GrundschriftBold28.h"
// #include "GrundschriftBold30.h"


#define CLOCK_X_POS (TFT_WHITE / 2)
#define CLOCK_Y_POS (TFT_HEIGHT / 2)

#define CLOCK_FG   TFT_BLACK
#define CLOCK_BG   TFT_NAVY
#define SECCOND_FG TFT_RED
#define LABEL_FG   TFT_GOLD

#define CLOCK_R       240.0f / 2.0f // Clock face radius (float type)
#define H_HAND_LENGTH CLOCK_R/2.0f
#define M_HAND_LENGTH CLOCK_R/1.4f
#define S_HAND_LENGTH CLOCK_R/1.3f

#define FACE_W CLOCK_R * 2 + 1
#define FACE_H CLOCK_R * 2 + 1

// Calculate 1 second increment angles. Hours and minute hand angles
// change every second so we see smooth sub-pixel movement
#define SECOND_ANGLE 360.0 / 60.0
#define MINUTE_ANGLE SECOND_ANGLE / 60.0
#define HOUR_ANGLE   MINUTE_ANGLE / 12.0

// Sprite width and height
#define FACE_W CLOCK_R * 2 + 1
#define FACE_H CLOCK_R * 2 + 1

#define DEG2RAD 0.0174532925


TFTClock::TFTClock(TFT &tft)
    : tft {tft},
      face {&tft.getHandle()},
      backgroundColor {TFT_WHITE}
{

}

void TFTClock::setBackgroundColor(uint16_t color)
{
  if (this->backgroundColor != color)
  {
    this->backgroundColor = color;
  }
}

// =========================================================================
// setup
// =========================================================================
void TFTClock::setup() 
{
  // Create the clock face sprite
  face.setColorDepth(8); // 8 bit will work, but reduces effectiveness of anti-aliasing
  face.createSprite(FACE_W, FACE_H);

  // Only 1 font used in the sprite, so can remain loaded
  // face.loadFont(NotoSansBold15);
  // face.loadFont(NotoSansRegular28);
  // face.loadFont(NotoSansBold28);
  // face.loadFont(GrundschriftBold28);
  face.loadFont(GrundschriftBold28);
}

// =========================================================================
// loop
// =========================================================================
void TFTClock::loop() 
{
  // All graphics are drawn in sprite to stop flicker
  renderFace();
}

// namespace
// {
//     uint16_t rgb888torgb565(uint8_t red, uint8_t green, uint8_t blue)
//     {
//         uint16_t b = (blue >> 3) & 0x1f;
//         uint16_t g = ((green >> 2) & 0x3f) << 5;
//         uint16_t r = ((red >> 3) & 0x1f) << 11;

//         return (uint16_t) (r | g | b);
//     }

// }

// =========================================================================
// Draw the clock face in the sprite
// =========================================================================
void TFTClock::renderFace() 
{
  struct tm info;
  if (getLocalTime(&info, 100))
  {
    int t = (((info.tm_hour * 60) + info.tm_min) * 60) + info.tm_sec;

    float h_angle = t * HOUR_ANGLE;
    float m_angle = t * MINUTE_ANGLE;
    float s_angle = t * SECOND_ANGLE;

    // The face is completely redrawn - this can be done quickly
    face.fillSprite(this->backgroundColor);

    // int c = ((int)(t/60.*255.))%256;
    // uint16_t bg = rgb888torgb565(c,c,c);
    // Serial.printf("T=%f ~> s=%d ~> %d bg=%d\n", t, ((int)(t))%60, ((int)(t/60.*255.))%256, bg);

    // Draw the face circle
    // face.fillSmoothCircle( CLOCK_R, CLOCK_R, CLOCK_R, this->backgroundColor);

    // Set text datum to middle centre and the colour
    face.setTextDatum(MC_DATUM);

    // The background colour will be read during the character rendering
    face.setTextColor(CLOCK_FG, this->backgroundColor);

    // Text offset adjustment
    constexpr uint32_t dialOffset = CLOCK_R - 20;

    float xp = 0.0, yp = 0.0; // Use float pixel position for smooth AA motion

    // Draw digits around clock perimeter
    for (uint32_t h = 1; h <= 12; h++) 
    {
        getCoord(CLOCK_R, CLOCK_R, &xp, &yp, dialOffset, h * 360.0 / 12);
        face.drawNumber(h, xp, 2 + yp);
    }

    // Add text (could be digital time...)
    face.setTextColor(LABEL_FG, this->backgroundColor);
    // face.drawString("Paula & Leo", CLOCK_R, CLOCK_R * 0.75);

    // Draw minute hand
    getCoord(CLOCK_R, CLOCK_R, &xp, &yp, M_HAND_LENGTH, m_angle);
    face.drawWideLine(CLOCK_R, CLOCK_R, xp, yp, 6.0f, CLOCK_FG);
    face.drawWideLine(CLOCK_R, CLOCK_R, xp, yp, 2.0f, this->backgroundColor);

    // Draw hour hand
    getCoord(CLOCK_R, CLOCK_R, &xp, &yp, H_HAND_LENGTH, h_angle);
    face.drawWideLine(CLOCK_R, CLOCK_R, xp, yp, 6.0f, CLOCK_FG);
    face.drawWideLine(CLOCK_R, CLOCK_R, xp, yp, 2.0f, this->backgroundColor);

    // Draw the central pivot circle
    face.fillSmoothCircle(CLOCK_R, CLOCK_R, 4, CLOCK_FG);

    // Draw cecond hand
    getCoord(CLOCK_R, CLOCK_R, &xp, &yp, S_HAND_LENGTH, s_angle);
    face.drawWedgeLine(CLOCK_R, CLOCK_R, xp, yp, 2.5, 1.0, SECCOND_FG);
    face.pushSprite(0, 0, TFT_TRANSPARENT);
  }
  

}

// =========================================================================
// Get coordinates of end of a line, pivot at x,y, length r, angle a
// =========================================================================
// Coordinates are returned to caller via the xp and yp pointers
void TFTClock::getCoord(int16_t x, int16_t y, float *xp, float *yp, int16_t r, float a)
{
  float sx1 = cos( (a - 90) * DEG2RAD);
  float sy1 = sin( (a - 90) * DEG2RAD);
  *xp =  sx1 * r + x;
  *yp =  sy1 * r + y;
}
