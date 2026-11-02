
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
 *
 * WICHTIG:
 * LCD-Reset, Touch-Reset und SD-CS liegen am TCA9554-Expander.
 * Sie sind keine direkt ansteuerbaren ESP32-GPIOs.
 */

// ------------------------------------------------------------
// Display – ST77916 QSPI
// ------------------------------------------------------------

#define LCD_WIDTH       360
#define LCD_HEIGHT      360

#define LCD_CS          21
#define LCD_SCLK        40

#define LCD_SDIO0       46
#define LCD_SDIO1       45
#define LCD_SDIO2       42
#define LCD_SDIO3       41

#define LCD_BL          5

// Reset wird über den TCA9554 geschaltet.
// -1 verhindert, dass der Display-Treiber einen ESP32-GPIO
// als Reset-Pin verwendet.
#define LCD_RESET       (-1)


// ------------------------------------------------------------
// Touch – CST816
// Eigener I2C-Bus
// ------------------------------------------------------------

#define TP_IIC_SDA      1
#define TP_IIC_SCL      3

#define TP_INT          4

// Reset liegt am TCA9554, EXIO1.
#define TP_RESET        (-1)

#define TOUCH_ADDR      0x15


// ------------------------------------------------------------
// Gemeinsamer I2C-Bus für RTC und weitere Sensoren
// ------------------------------------------------------------

#define IIC_SDA         11
#define IIC_SCL         10

#define RTC_ADDR        0x51
#define RTC_INT         9


// ------------------------------------------------------------
// TCA9554PWR – GPIO-Expander
// ------------------------------------------------------------

#define TCA9554_ADDR    0x20

// Expander-Ausgänge (EXIO)
#define EXIO_TP_RESET   1
#define EXIO_LCD_RESET  2
#define EXIO_SD_CS      3
#define EXIO_IMU_INT2   4
#define EXIO_IMU_INT1   5


// ------------------------------------------------------------
// microSD / TF-Karten-Slot – SPI
// ------------------------------------------------------------

#define SD_MISO         16
#define SD_MOSI         17
#define SD_SCLK         14

// Chip Select liegt am TCA9554, EXIO3.
// Kein direkter ESP32-GPIO!
#define SD_CS           (-1)


// ------------------------------------------------------------
// Audio – PCM5101 I2S-DAC
// ------------------------------------------------------------

// PCM5101 benötigt kein ES8311-I2C-Setup und keinen MCLK.
#define I2S_DOUT        47
#define I2S_LRCK        38
#define I2S_BCLK        48

// Mikrofonanschlüsse des Boards
#define MIC_WS          2
#define MIC_SCK         15
#define MIC_SD          39


// ------------------------------------------------------------
// QMI8658 – Bewegungssensor
// ------------------------------------------------------------

#define IMU_SDA         11
#define IMU_SCL         10

#define IMU_ADDR        0x6B


// ------------------------------------------------------------
// Hardware / Speicher
// ------------------------------------------------------------

#define BOARD_FLASH_SIZE    (16UL * 1024UL * 1024UL)
#define BOARD_PSRAM_SIZE    (8UL * 1024UL * 1024UL)

#define BOARD_HAS_PSRAM     1
#define BOARD_HAS_TCA9554   1
#define BOARD_HAS_RTC       1
#define BOARD_HAS_PCM5101   1
#define BOARD_HAS_ST77916   1
#define BOARD_HAS_CST816    1

