
#include "TFT.hpp"
#include "TFTStarfield.hpp"

TFTStarfield::TFTStarfield(TFT &tft)
    : tft {tft}
{

}

void TFTStarfield::setup() 
{
  za = random(256);
  zb = random(256);
  zc = random(256);
  zx = random(256);
}

void TFTStarfield::reset()
{
    this->tft.getHandle().fillScreen(TFT_BLACK);
}

void TFTStarfield::loop()
{
  unsigned long t0 = micros();
  uint8_t spawnDepthVariation = 255;

  auto &tft = this->tft.getHandle();

  for(int i = 0; i < TFT_STARFIELD_NSTARS; ++i)
  {
    if (sz[i] <= 1)
    {
      sx[i] = rng();
      sy[i] = rng();
      sz[i] = spawnDepthVariation--;
    }
    else
    {
      int old_screen_x = ((int)sx[i] - 120) * 256 / sz[i] + 120;
      int old_screen_y = ((int)sy[i] - 120) * 256 / sz[i] + 120;

      // This is a faster pixel drawing function for occassions where many single pixels must be drawn
      tft.drawPixel(old_screen_x, old_screen_y,TFT_BLACK);

      sz[i] -= 2;
      if (sz[i] > 1)
      {
        int screen_x = ((int)sx[i] - 120) * 256 / sz[i] + 120;
        int screen_y = ((int)sy[i] - 120) * 256 / sz[i] + 120;
  
        if (screen_x >= 0 && screen_y >= 0 && screen_x < 240 && screen_y < 240)
        {
          uint8_t r, g, b;
          r = g = b = 255 - sz[i];
          tft.drawPixel(screen_x, screen_y, tft.color565(r,g,b));
        }
        else
          sz[i] = 0; // Out of screen, die.
      }
    }
  }
}