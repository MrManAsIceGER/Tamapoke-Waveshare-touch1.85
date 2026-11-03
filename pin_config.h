
#pragma once

/*
 * TamaPoke – Hardware-Konfiguration
 * Waveshare ESP32-S3-Touch-LCD-1.85 Rev 2.0
 *
 * Display: ST77916, QSPI, 360 x 360
 * Touch:   CST816, eigener I2C-Bus
 * Expander:TCA9554PWR
 * Audio:   PCM5101
 * RTC:     PCF85063
 * IMU:     QMI8658
 * SD:      SDMMC 1-Bit
 */

// Display – ST77916 QSPI
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
#define LCD_RESET       (-1)   // Reset ueber TCA9554 EXIO2

// Touch – CST816, separater I2C-Bus
#define TP_IIC_SDA      1
#define TP_IIC_SCL      3
#define TP_INT          4
#define TP_RESET        (-1)   // Reset ueber TCA9554 EXIO1
#define CST816_ADDR     0x15

// RTC und Sensoren – gemeinsamer I2C-Bus
#define IIC_SDA         11
#define IIC_SCL         10
#define RTC_ADDR        0x51
#define RTC_INT         9
#define IMU_SDA         IIC_SDA
#define IMU_SCL         IIC_SCL
#define IMU_ADDR        0x6B

// TCA9554PWR – GPIO-Expander
#define TCA9554_ADDR    0x20
#define EXIO_TP_RESET   1
#define EXIO_LCD_RESET  2
#define EXIO_SD_CS      3
#define EXIO_IMU_INT2   4
#define EXIO_IMU_INT1   5

// microSD / TF-Karte – SDMMC 1-Bit
#define SDMMC_CLK       14
#define SDMMC_CMD       17
#define SDMMC_D0        16

// Bestehende sdmon.cpp-Namen beibehalten:
#define SD_SCLK         SDMMC_CLK
#define SD_MOSI         SDMMC_CMD
#define SD_MISO         SDMMC_D0
#define SD_CS           (-1)

// Audio – PCM5101 I2S-DAC
#define I2S_DOUT        47
#define I2S_LRCK        38
#define I2S_BCLK        48

// Mikrofonanschluesse
#define MIC_WS          2
#define MIC_SCK         15
#define MIC_SD          39

// Speicher und Board-Eigenschaften
#define BOARD_FLASH_SIZE    (16UL * 1024UL * 1024UL)
#define BOARD_PSRAM_SIZE    (8UL * 1024UL * 1024UL)

#define BOARD_HAS_PSRAM     1
#define BOARD_HAS_TCA9554   1
#define BOARD_HAS_RTC       1
#define BOARD_HAS_PCM5101   1
#define BOARD_HAS_ST77916   1
#define BOARD_HAS_CST816    1
#define BOARD_HAS_QMI8658   1
#define BOARD_HAS_SDMMC     1
