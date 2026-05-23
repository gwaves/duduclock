// TFT_eSPI configuration for DuduClock
// Loaded automatically via -include build flag

#define USER_SETUP_INFO "DuduClock"

#define ST7789_DRIVER
#define TFT_RGB_ORDER TFT_BGR
#define TFT_WIDTH  240
#define TFT_HEIGHT 320
#define TFT_INVERSION_OFF

#define TFT_MOSI 3
#define TFT_SCLK 2
#define TFT_CS   7
#define TFT_DC   4
#define TFT_RST  5

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

#define SPI_FREQUENCY  27000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
