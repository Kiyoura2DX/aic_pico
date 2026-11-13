/*
 * AIC Reader Board Definitions
 * WHowe <github.com/whowechina>
 */

#if defined BOARD_AIC_PICO

#define I2C_PORT_LIST { { i2c0, 21, 20 }, { i2c1, 19, 18 }, { i2c0, 17, 16 }, { i2c1, 27, 26 } }
#define I2C_FREQ 433*1000

#define SPI_PORT spi0
#define SPI_MISO 16
#define SPI_SCK 18
#define SPI_MOSI 19
#define SPI_RST 27
#define SPI_NSS 17
#define SPI_BUSY 26

#define RGB_PIN 12
#define DEFAULT_RGB_ORDER GRB
#define LED_DEF { 25, 22, 13, 15 }

#define KEYPAD_ROW_PINS { 8, 9, 10, 11 }  // 4 pines para las filas
#define KEYPAD_COL_PINS { 5, 6, 7 }       // 3 pines para las columnas
#define AIC_TOUCH_EN 14

/* HID Keycode: Números superiores 1-9, 0, - y . (Inmunes a NumLock) */
#define KEYPAD_NKRO_MAP "\x1e\x1f\x20\x21\x22\x23\x24\x25\x26\x27\x2d\x37"
#else

#endif
