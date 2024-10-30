#pragma once

#ifdef __cplusplus

#include <Arduino.h>

// SERIAL 1 SimCOM A7672E-FASE
//static uint32_t rates[] = {115200, 57600,  38400, 19200, 9600,  74400, 74880,
//                           230400, 460800, 2400,  4800,  14400, 28800};
#define LTE_BAUD       115200
#define LTE_TX_PIN     47
#define LTE_RX_PIN     48
// Power Control
#define LTE_PWRKEY_PIN 1 //3

// LoRa E22-900M22S
#define LORA_HOST      SPI2_HOST
#define LORA_NSS_PIN   10
#define LORA_SCLK_PIN  12
#define LORA_MOSI_PIN  11
#define LORA_MISO_PIN  13
#define LORA_NRST_PIN  14 //40
#define LORA_BUSY_PIN  45 //41
#define LORA_DIO1_PIN  3 //42
#define LORA_TXEN_PIN  19 //1
#define LORA_RXEN_PIN  20 //2
#define SD_NSS_PIN     5
#define SD_SCLK_PIN    12
#define SD_MOSI_PIN    11
#define SD_MISO_PIN    13
// I2C PIN
#define I2C_SDA_PIN    8
#define I2C_SCL_PIN    9

// TFT LCD ST7789V [240*320]
#define TFT_HOST       SPI3_HOST
#define TFT_SCLK_PIN   39 //47
#define TFT_MOSI_PIN   40 //48
#define TFT_MISO_PIN   -1
#define TFT_CS_PIN     42 //21
#define TFT_DC_PIN     41 //14
#define TFT_RST_PIN    38
#define TFT_BL_PIN     46 //45

//1 wire DS18B20
#define DS18B20_PIN    4 //15 //39

// Battery Voltage. [*2]
#define BATT_ADC_PIN   6 //16 //ADC2_CH5
//uint8_t BATT_ADC_PIN=16;

// Switch Mode
#define BTN_CT_PIN   0
#define BTN_A_PIN    16
#define BTN_B_PIN    7
#define BTN_C_PIN    17
#define BTN_D_PIN    15
// Buzzer
//#define PWM_SPK_PIN  21
#define PWM_LEFT_CHANNEL_GPIO 21 //Buzzer or Aux
#else

#error Project requires a C++ compiler, please change file extension to .cc or .cpp

#endif