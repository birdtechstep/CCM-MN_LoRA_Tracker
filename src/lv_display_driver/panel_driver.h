#include <board_config.h>
//#include <LovyanGFX.hpp>
//#define LGFX_USE_V1
#include <Arduino_GFX_Library.h>

#define GFX_BL TFT_BL_PIN // default backlight pin, you may replace DF_GFX_BL to actual backlight pin

//Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC_PIN, TFT_CS_PIN /* CS */);
Arduino_DataBus *bus = new Arduino_SWSPI(TFT_DC_PIN, TFT_CS_PIN, TFT_SCLK_PIN, TFT_MOSI_PIN, TFT_MISO_PIN);
//Arduino_GFX *gfx = new Arduino_ST7789(bus, TFT_RST_PIN /* RST */, 0 /* rotation */, true /* IPS */, 240 /* width */, 240 /* height */, 0 /* col offset 1 */, 0 /* row offset 1 */, 0 /* col offset 2 */, 80 /* row offset 2 */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, TFT_RST_PIN /* RST */, 0 /* rotation */, true /* IPS */, 240 /* width */, 320 /* height */, 0 /* col offset 1 */, 0 /* row offset 1 */, 0 /* col offset 2 */, 0 /* row offset 2 */);
