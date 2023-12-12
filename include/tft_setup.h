// See SetupX_Template.h for all options available
#define USER_SETUP_ID 46
#define USER_SETUP_LOADED 1

#define GC9A01_DRIVER
#define TFT_WIDTH 240 // GC9A01 240 x 240
#define TFT_HEIGHT 240 // GC9A01 240 x 240

#define TFT_MISO -1

#ifdef ESP32
    #ifdef USE_HSPI_PORT
//        #define TFT_MISO 19
        #define TFT_MOSI 23
        #define TFT_SCLK 18
        #define TFT_CS    15  // Chip select control pin
    #else /* USE_HSPI_PORT */
//        #define TFT_MISO 12
        #define TFT_MOSI 13
        #define TFT_SCLK 14
        #define TFT_CS    15  // Chip select control pin
    #endif /* USE_HSPI_PORT */

    #define TFT_DC    2  // Data Command control pin
    #define TFT_RST   4  // Reset pin (could connect to RST pin)
    #define TFT_BKL   22
    //#define TFT_RST  -1  // Set TFT_RST to -1 if display RESET is connected to ESP32 board RST
    #define TOUCH_CS -1
    #define SPI_FREQUENCY  40000000

#endif /* ESP32 */

#ifdef ESP8266

// For NodeMCU - use pin numbers in the form PIN_Dx where Dx is the NodeMCU pin designation
// #define TFT_MISO  PIN_D6  // Automatically assigned with ESP8266 if not defined
#define TFT_MOSI  PIN_D7  // Automatically assigned with ESP8266 if not defined
#define TFT_SCLK  PIN_D5  // Automatically assigned with ESP8266 if not defined

#define TFT_CS    PIN_D8  // Chip select control pin D8
#define TFT_DC    PIN_D3  // Data Command control pin
#define TFT_RST   PIN_D4  // Reset pin (could connect to NodeMCU RST, see next line)
//#define TFT_RST  -1     // Set TFT_RST to -1 if the display RESET is connected to NodeMCU RST or 3.3V

#define SPI_FREQUENCY  27000000
#endif

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#ifndef ESP8266
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#endif
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

#define SMOOTH_FONT


#define SPI_READ_FREQUENCY  40000000

#define SPI_TOUCH_FREQUENCY  2500000


// The ESP32 has 2 free SPI ports i.e. VSPI and HSPI, the VSPI is the default.
// If the VSPI port is in use and pins are not accessible (e.g. TTGO T-Beam)
// then uncomment the following line:
#define USE_HSPI_PORT

// #define SUPPORT_TRANSACTIONS