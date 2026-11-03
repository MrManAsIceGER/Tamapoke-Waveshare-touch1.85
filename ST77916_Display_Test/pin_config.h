
#pragma once

/*
 * TamaPoke – Hardware-Konfiguration
 *
 * Zielhardware:
 * Waveshare ESP32-S3-Touch-LCD-1.85
 * Hardware-Revision: 2.0
 *
 * Display:     ST77916, QSPI, 360 x 360
 * Touch:       CST816, I2C
 * IO-Expander: TCA9554PWR
 * Audio:       PCM5101, I2S-DAC
 * RTC:         PCF85063
 * IMU:         QMI8658
 *
 * ESP32-S3:
 * 16 MB Flash
 * 8 MB PSRAM
 *
 * LCD reset and touch reset are controlled through the TCA9554.
 */


// ============================================================
// DISPLAY – ST77916, QSPI
// ============================================================

#define LCD_WIDTH       360
#define LCD_HEIGHT      360

#define LCD_CS          21
#define LCD_SCLK        40

#define LCD_SDIO0       46
#define LCD_SDIO1       45
#define LCD_SDIO2       42
#define LCD_SDIO3       41

#define LCD_TE          18
#define LCD_BL          5

// LCD reset is connected to TCA9554 EXIO2.
// No direct ESP32 GPIO connection.
#define LCD_RESET       (-1)


// ============================================================
// TOUCH – CST816
// Separate I2C bus
// ============================================================

#define TP_IIC_SDA      1
#define TP_IIC_SCL      3

#define TP_INT          4

// Touch reset is connected to TCA9554 EXIO1.
#define TP_RESET        (-1)

#define TOUCH_ADDR      0x15


// ============================================================
// I2C – RTC und Sensoren
// ============================================================

#define IIC_SDA         11
#define IIC_SCL         10

#define RTC_ADDR        0x51
#define RTC_INT         9


// ============================================================
// TCA9554PWR – GPIO-EXPANDER
// ============================================================

#define TCA9554_ADDR    0x20

#define EXIO_TP_RESET   1
#define EXIO_LCD_RESET  2
#define EXIO_SD_CS      3
#define EXIO_IMU_INT2   4
#define EXIO_IMU_INT1   5


// ============================================================
// microSD / TF card – SDMMC 1-bit
//
// CLK  = GPIO14
// CMD  = GPIO17
// D0   = GPIO16
//
// GPIO17 is CMD, not MOSI in the SPI sense.
// No separate CS pin is used in 1-bit mode.
//
// ============================================================

#define SDMMC_CLK      14
#define SDMMC_CMD      17
#define SDMMC_D0       16

// Compatibility names for existing project code:
#define SD_SCLK        SDMMC_CLK
#define SD_MOSI        SDMMC_CMD
#define SD_MISO        SDMMC_D0

// No direct chip select in SDMMC 1-bit mode.
#define SD_CS          (-1)


// ============================================================
// AUDIO – PCM5101 I2S-DAC
// ============================================================

// PCM5101 – Lautsprecherausgang
#define I2S_DOUT        47
#define I2S_LRCK        38
#define I2S_BCLK        48

// Mikrofonanschlüsse des Boards
#define MIC_WS          2
#define MIC_SCK         15
#define MIC_SD          39


// ============================================================
// IMU – QMI8658
// ============================================================

#define IMU_SDA         11
#define IMU_SCL         10

#define IMU_ADDR        0x6B


// ============================================================
// HARDWARE / SPEICHER
// ============================================================

#define BOARD_FLASH_SIZE    (16UL * 1024UL * 1024UL)
#define BOARD_PSRAM_SIZE    (8UL * 1024UL * 1024UL)

#define BOARD_HAS_PSRAM     1
#define BOARD_HAS_TCA9554   1
#define BOARD_HAS_RTC       1
#define BOARD_HAS_PCM5101   1
#define BOARD_HAS_ST77916   1
#define BOARD_HAS_CST816    1
