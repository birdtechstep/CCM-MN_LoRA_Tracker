#include <Arduino.h>
#include "board_config.h"
//#include "main.h"
#include "esp_adc_cal.h"
#include <esp_task_wdt.h>

#include <Wire.h>
#include <LoRaWan-Arduino.h>
#include <SPI.h>
#include <WiFi.h>

#include <RX8900RTC.h>  // https://github.com/citriena/RX8900RTC

RX8900RTC myRTC;

// Replace these with your WiFi credentials
const char* ssid = "";  //mention WiFi name
const char* password = "";  //mention WiFi password

// Define NTP Server address
const char* ntpServer = "pool.ntp.org";  //Modify as per your country
const long  gmtOffset_sec = 7*60*60; // Offset from UTC (in seconds) (Thailand GMT 7:00 // 7*60*60 = 25200) Modify as per your country
const int   daylightOffset_sec = 3600; // Daylight offset (in seconds)

#include <lvgl.h>
#include "lv_display_driver/lv_display_driver.h"
// Created by GUI-Guider-1.7.2-GA
#include "generated/gui_guider.h"
#include "generated/widgets_init.h"
#include "custom/custom.h"
lv_ui guider_ui;

#include <TimeLib.h>
//#include <SoftwareSerial.h>
#include <SoftwareSerial.h>
EspSoftwareSerial::UART SerialAT;
//SoftwareSerial SerialAT(LTE_RX_PIN, LTE_TX_PIN);  // RX, TX
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 115200
//#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_MODEM_SIM7672
// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

#define TINY_GSM_TEST_GSM_LOCATION true
#define TINY_GSM_TEST_GPS true

#define TINY_GSM_USE_GPRS false
#define TINY_GSM_USE_WIFI false
#include <TinyGsmClient.h>
#define DUMP_AT_COMMANDS
#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm        modem(SerialAT);
#endif
// Define the serial console for debug prints, if needed
#define TINY_GSM_DEBUG SerialMon
TinyGsmClient client(modem);
//#define TINY_GSM_DEBUG Serial
#define GSM_PIN LTE_PWR_CTRL_PIN
bool GPS_FLAG = false;
#include "pwm_buzzer/pwm_buzzer.h"

float lat = 0; //13.7415801;
float lon = 0; //100.7184983;

// WDT 60 seconds for OTA update
#define WDT_TIMEOUT 120 // second
uint8_t wifimac[6];

uint8_t battPIN = BATT_ADC_PIN;
double battValue = 0;
uint8_t battLevel = 0;
void batteryLevel(double battValue);

uint8_t ELSAGetBatteryLevel(void);

// LoRaWan setup definitions
#define SCHED_MAX_EVENT_DATA_SIZE APP_TIMER_SCHED_EVENT_DATA_SIZE // Maximum size of scheduler events
#define SCHED_QUEUE_SIZE 60  // Maximum number of events in the scheduler queue

/**< Maximum number of events in the scheduler queue  */
#define LORAWAN_APP_DATA_BUFF_SIZE 256 // Size of the data to be transmitted
//#define LORAWAN_APP_TX_DUTYCYCLE 300000 // Defines the application data transmission duty cycle. 30s, value in [ms]
//#define LORAWAN_APP_TX_DUTYCYCLE 5*60*1000 // Defines the application data transmission duty cycle. 5min, value in [ms]
#define LORAWAN_APP_TX_DUTYCYCLE 1*60*1000 // Defines the application data transmission duty cycle. 1 min, value in [ms]
#define APP_TX_DUTYCYCLE_RND 1000 // Defines a random delay for application data transmission duty cycle. 1s, value in [ms]
#define JOINREQ_NBTRIALS 3  

bool doOTAA = true;
hw_config hwConfig;

// ESP32 - SX126x pin configuration
int PIN_LORA_RESET = LORA_NRST_PIN;   // LORA RESET
int PIN_LORA_NSS   = LORA_NSS_PIN;    // LORA SPI CS
int PIN_LORA_SCLK  = LORA_SCLK_PIN;   // LORA SPI CLK
int PIN_LORA_MISO  = LORA_MISO_PIN;   // LORA SPI MISO
int PIN_LORA_DIO_1 = LORA_DIO1_PIN;   // LORA DIO_1
int PIN_LORA_BUSY  = LORA_BUSY_PIN;   // LORA SPI BUSY
int PIN_LORA_MOSI  = LORA_MOSI_PIN;   // LORA SPI MOSI
int RADIO_TXEN     = LORA_TXEN_PIN ;  // LORA ANTENNA TX ENABLE
int RADIO_RXEN     = LORA_RXEN_PIN;   // LORA ANTENNA RX ENABLE

// Define Helium or TTN OTAA keys. All msb (big endian).
//uint8_t nodeDeviceEUI[8]  = {0x00, 0x00, 0xec, 0xda, 0x3b, 0x61, 0xbb, 0x01};
uint8_t nodeDeviceEUI[8]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //get from mac address
uint8_t nodeAppEUI[8]     = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06};
uint8_t nodeAppKey[16]    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06};

// Foward declaration
/** LoRaWAN callback when join network finished */
static void lorawan_has_joined_handler(void);
/** LoRaWAN callback when join network failed */
static void lorawan_join_fail_handler(void);
/** LoRaWAN callback when data arrived */
static void lorawan_rx_handler(lmh_app_data_t *app_data);
/** LoRaWAN callback after class change request finished */
static void lorawan_confirm_class_handler(DeviceClass_t Class);
/** LoRaWAN callback after class change request finished */
static void lorawan_unconfirm_tx_finished(void);
/** LoRaWAN callback after class change request finished */
static void lorawan_confirm_tx_finished(bool result);
/** LoRaWAN Function to send a package */
static void send_lora_frame(void);
static uint32_t timers_init(void);

// APP_TIMER_DEF(lora_tx_timer_id);  // LoRa tranfer timer instance.
TimerEvent_t appTimer;   // LoRa tranfer timer instance.
static uint8_t m_lora_app_data_buffer[LORAWAN_APP_DATA_BUFF_SIZE]; // Lora user application data buffer.
static lmh_app_data_t m_lora_app_data = {m_lora_app_data_buffer, 0, 0, 0, 0};  // Lora user application data structure.

/**@brief Structure containing LoRaWan parameters, needed for lmh_init()
 */
static lmh_param_t lora_param_init = {LORAWAN_ADR_OFF, DR_3, LORAWAN_PUBLIC_NETWORK,
                    JOINREQ_NBTRIALS, LORAWAN_DEFAULT_TX_POWER, LORAWAN_DUTYCYCLE_OFF};

////static lmh_callback_t lora_callbacks = {BoardGetBatteryLevel, BoardGetUniqueId, BoardGetRandomSeed,
////                    lorawan_rx_handler, lorawan_has_joined_handler, 
////                    lorawan_confirm_class_handler, lorawan_join_fail_handler,
////                    lorawan_unconfirm_tx_finished, lorawan_confirm_tx_finished};
static lmh_callback_t lora_callbacks = {ELSAGetBatteryLevel, BoardGetUniqueId, BoardGetRandomSeed,
                    lorawan_rx_handler, lorawan_has_joined_handler, 
                    lorawan_confirm_class_handler, lorawan_join_fail_handler,
                    lorawan_unconfirm_tx_finished, lorawan_confirm_tx_finished};

void modemOff();
void modemOn();

/**@brief void setup()
 */
void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  // Initialize Serial for debug output
  Serial.begin(115200);
  myRTC.init();

  // set nodeDeviceEUI from MAC
	esp_read_mac(wifimac, ESP_MAC_WIFI_STA);  // Read WiFi MAC Address
	nodeDeviceEUI[2] = wifimac[0];
	nodeDeviceEUI[3] = wifimac[1];
	nodeDeviceEUI[4] = wifimac[2];
	nodeDeviceEUI[5] = wifimac[3];
	nodeDeviceEUI[6] = wifimac[4];
	nodeDeviceEUI[7] = wifimac[5];

    // Define the HW configuration between MCU and SX126x
  hwConfig.CHIP_TYPE = SX1262_CHIP; // Example uses an eByte E22 module with an SX1262
  hwConfig.PIN_LORA_RESET = PIN_LORA_RESET; // LORA RESET
  hwConfig.PIN_LORA_NSS = PIN_LORA_NSS;   // LORA SPI CS
  hwConfig.PIN_LORA_SCLK = PIN_LORA_SCLK;   // LORA SPI CLK
  hwConfig.PIN_LORA_MISO = PIN_LORA_MISO;   // LORA SPI MISO
  hwConfig.PIN_LORA_DIO_1 = PIN_LORA_DIO_1; // LORA DIO_1
  hwConfig.PIN_LORA_BUSY = PIN_LORA_BUSY;   // LORA SPI BUSY
  hwConfig.PIN_LORA_MOSI = PIN_LORA_MOSI;   // LORA SPI MOSI
  hwConfig.RADIO_TXEN = RADIO_TXEN; // LORA ANTENNA TX ENABLE (e.g. eByte E22 module)
  hwConfig.RADIO_RXEN = RADIO_RXEN; // LORA ANTENNA RX ENABLE (e.g. eByte E22 module)
  hwConfig.USE_DIO2_ANT_SWITCH = false; // LORA DIO2 does not control antenna
  hwConfig.USE_DIO3_TCXO = true;  // LORA DIO3 controls oscillator voltage (e.g. eByte E22 module)
  hwConfig.USE_DIO3_ANT_SWITCH = false; // LORA DIO3 does not control antenna

  // Test Buzzer support 2730 Hz
  pwm_buzzer_init();
  buzzer_on(50, 100); // 100 ms. test

  Serial.println("=====================================");
  Serial.println("SX126x LoRaWan test");
  Serial.println("=====================================");

  // Initialize LoRa chip.
  uint32_t err_code = lora_hardware_init(hwConfig);
  if (err_code != 0)
  {
    Serial.printf("lora_hardware_init failed - %d\n", err_code);
  } else {
    Serial.printf("lora_hardware_init - %d\n", err_code);
  }

  // Initialize Scheduler and timer (Must be after lora_hardware_init)
  err_code = timers_init();
  if (err_code != 0)
  {
    Serial.printf("timers_init failed - %d\n", err_code);
  }

  // Setup the EUIs and Keys
  lmh_setDevEui(nodeDeviceEUI);
  lmh_setAppEui(nodeAppEUI);
  lmh_setAppKey(nodeAppKey);

  // Initialize LoRaWan
  // CLASS C works for esp32 and e22, US915 region works in america, other local frequencies can be found 
  // here https://docs.helium.com/lorawan-on-helium/frequency-plans/
  
  err_code = lmh_init(&lora_callbacks, lora_param_init, doOTAA, CLASS_A, LORAMAC_REGION_AS923);
  if (err_code != 0)
  {
    Serial.printf("lmh_init failed - %d\n", err_code);
  }

  // For Helium and US915, you need as well to select subband 2 after you called lmh_init(), 
  // For US816 you need to use subband 1. Other subbands configurations can be found in
  // https://github.com/beegee-tokyo/SX126x-Arduino/blob/1c28c6e769cca2b7d699a773e737123fc74c47c7/src/mac/LoRaMacHelper.cpp

  lmh_setSubBandChannels(2);

  // Start Join procedure
  lmh_join();

  analogSetAttenuation(ADC_11db);  // vBatt config

  lv_display_driver_init(); // display screen driver initial
  //lv_demo_widgets();
  setup_ui(&guider_ui);
  custom_init(&guider_ui);

  char str_deveui[35];  // 9 + 17 |
  sprintf(str_deveui, "%02X%02X%02X%02X%02X%02X%02X%02X", nodeDeviceEUI[0], nodeDeviceEUI[1], nodeDeviceEUI[2], nodeDeviceEUI[3], nodeDeviceEUI[4], nodeDeviceEUI[5], nodeDeviceEUI[6], nodeDeviceEUI[7]);
  lv_label_set_text(guider_ui.screen_label_deveui_val, str_deveui);

  pinMode(LTE_PWRKEY_PIN, OUTPUT);
  Serial.println("Wait...for modem");

  modemOn();

  // Set GSM module baud rate
  SerialAT.begin(LTE_BAUD, EspSoftwareSerial::SWSERIAL_8N1, LTE_RX_PIN, LTE_TX_PIN);
  //TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
  // SerialAT.begin(9600);
   delay(3000);

  //modemOn();
  int retry = 0;
  while (!modem.testAT(1000)) {
    Serial.println(".");
    if (retry++ > 10) {
      digitalWrite(LTE_PWRKEY_PIN, LOW);
      delay(100);
      digitalWrite(LTE_PWRKEY_PIN, HIGH);
      delay(1000);    //Ton = 1000ms ,Min = 500ms, Max 2000ms
      digitalWrite(LTE_PWRKEY_PIN, LOW);
      retry = 0;
    }
  }

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  //modem.restart();
  modem.init();

  SerialMon.printf("GPS starting\n");
  if (modem.enableGPS()) {     
    SerialMon.printf("GPS started\n");
  }

  String modemInfo = modem.getModemInfo();
  DBG("Modem Info:", modemInfo);
/*
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  uint8_t i = 0;
  while ((WiFi.status() != WL_CONNECTED) || i <= 10) {
    delay(500);
    i++;
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");

  // Init and get the time
  if (WiFi.status() == WL_CONNECTED) configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  */
  //tmElements_t tm = {0, 0, 12, 0, 5, 5, CalendarYrToTm(2020)}; // second, minute, hour, week, day, month, year
  //tmElements_t tm = {timeinfo.tm_sec, timeinfo.tm_min, timeinfo.tm_hour, timeinfo.tm_wday, timeinfo.tm_mday, timeinfo.tm_mon+1, timeinfo.tm_year+1900}; // second, minute, hour, week, day, month, year
  //myRTC.set(&timeinfo);
}

/**@brief void loop()
 */
unsigned long previousMillis = 0;
const long interval = 1000; // for display time
void loop() {
  // put your main code here, to run repeatedly:
  lv_timer_handler(); /* let the GUI do its work */
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    //previousMillis = currentMillis;
    previousMillis += interval;

    tmElements_t tmz = myRTC.read();
    // DATE
    char str_date[12];
    sprintf(str_date, "%02D/%02D/%02D", tmz.Day, tmz.Month, tmYearToCalendar(tmz.Year));
    lv_label_set_text(guider_ui.screen_label_date_val, str_date);
    // TIME
    char str_time[10];
    sprintf(str_time, "%02D:%02D:%02D", tmz.Hour, tmz.Minute, tmz.Second);
    lv_label_set_text(guider_ui.screen_label_time_val, str_time);

    if (lmh_join_status_get() != LMH_SET) {
      // Not joined, try again later
      lv_obj_add_flag(guider_ui.screen_img_lora_on, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_clear_flag(guider_ui.screen_img_lora_on, LV_OBJ_FLAG_HIDDEN);
    }

    // read Battery
    battValue = analogReadMilliVolts(battPIN)*0.00203;
    uint8_t batt = map(battValue*100, 327, 420, 0, 100); // min 3.27v max 4.20v
    if (batt >= 100) batt = 100;
    battLevel = (batt*255)/100; // % Batt
    char str_batt[10];
    sprintf(str_batt, "BATT:%d%%", batt);
    lv_label_set_text(guider_ui.screen_label_batt, str_batt);
    Serial.printf("Current BATT. : %.2f V\n", battValue);
    
    if (GPS_FLAG != true) {
      if (modem.enableGPS()) {     
        SerialMon.printf("GPS started\n");
        GPS_FLAG = true;
      }
    }
    String gps_raw = modem.getGPSraw();
    Serial.println(gps_raw);
    uint8_t status;
    float gps_latitude  = 0;
    float gps_longitude = 0;
    float gps_speed     = 0;
    float gps_altitude  = 0;
    int   gps_vsat      = 0;
    int   gps_usat      = 0;
    float gps_accuracy  = 0;
    int   gps_year      = 0;
    int   gps_month     = 0;
    int   gps_day       = 0;
    int   gps_hour      = 0;
    int   gps_minute    = 0;
    int   gps_second    = 0;
    if (modem.getGPS(&status,&gps_latitude, &gps_longitude, &gps_speed, &gps_altitude,
                     &gps_vsat, &gps_usat, &gps_accuracy, &gps_year, &gps_month,
                     &gps_day, &gps_hour, &gps_minute, &gps_second)) {
      Serial.println(gps_latitude);
      Serial.println(gps_longitude);
      lat = gps_latitude;
      lon = gps_longitude;

      char str_lat[35];  // 9 + 17 |
      sprintf(str_lat, "%02F", gps_latitude);
      lv_label_set_text(guider_ui.screen_label_lat_val, str_lat);
      char str_lon[35];  // 9 + 17 |
      sprintf(str_lon, "%02F", gps_longitude);
      lv_label_set_text(guider_ui.screen_label_lon_val, str_lon);

    } else {
      //Serial.println("GPs not get!");
      lv_label_set_text(guider_ui.screen_label_lat_val, " ");
      lv_label_set_text(guider_ui.screen_label_lon_val, " ");

    }
    ////esp_task_wdt_reset();            // Added to repeatedly reset the Watch Dog
  } else {
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

static void lorawan_join_fail_handler(void)
{
  Serial.println("OTAA joined failed");
  Serial.println("Check LPWAN credentials and if a gateway is in range");
  // Restart Join procedure
  Serial.println("Restart network join request");
}

/**@brief LoRa function for handling HasJoined event.
 */
static void lorawan_has_joined_handler(void)
{
#if (OVER_THE_AIR_ACTIVATION != 0)
  Serial.println("Network Joined");
#else
  Serial.println("OVER_THE_AIR_ACTIVATION != 0");

#endif
  lmh_class_request(CLASS_A);

  TimerSetValue(&appTimer, LORAWAN_APP_TX_DUTYCYCLE);
  TimerStart(&appTimer);
  // app_timer_start(lora_tx_timer_id, APP_TIMER_TICKS(LORAWAN_APP_TX_DUTYCYCLE), NULL);
  Serial.println("Sending frame");
  send_lora_frame();
}

/**@brief Function for handling LoRaWan received data from Gateway
 *
 * @param[in] app_data  Pointer to rx data
 */
static void lorawan_rx_handler(lmh_app_data_t *app_data)
{
  Serial.printf("LoRa Packet received on port %d, size:%d, rssi:%d, snr:%d\n",
          app_data->port, app_data->buffsize, app_data->rssi, app_data->snr);

  for (int i = 0; i < app_data->buffsize; i++)
  {
    Serial.printf("%0X ", app_data->buffer[i]);
  }
  Serial.println("");

  switch (app_data->port)
  {
  case 3:
    // Port 3 switches the class
    if (app_data->buffsize == 1)
    {
      switch (app_data->buffer[0])
      {
      case 0:
        lmh_class_request(CLASS_A);
        break;

      case 1:
        lmh_class_request(CLASS_B);
        break;

      case 2:
        lmh_class_request(CLASS_C);
        break;

      default:
        break;
      }
    }
    break;

  case LORAWAN_APP_PORT:
    // YOUR_JOB: Take action on received data
    break;

  default:
    break;
  }
}

/**@brief Function to confirm LORaWan class switch.
 *
 * @param[in] Class  New device class
 */
static void lorawan_confirm_class_handler(DeviceClass_t Class)
{
  Serial.printf("switch to class %c done\n", "ABC"[Class]);

  // Informs the server that switch has occurred ASAP
  m_lora_app_data.buffsize = 0;
  m_lora_app_data.port = LORAWAN_APP_PORT;
  lmh_send(&m_lora_app_data, LMH_UNCONFIRMED_MSG);
}

/**
 * @brief Called after unconfirmed packet was sent
 * 
 */
static void lorawan_unconfirm_tx_finished(void)
{
  Serial.println("Uncomfirmed TX finished");
}

/**
 * @brief Called after confirmed packet was sent
 * @param result Result of sending true = ACK received false = No ACK
 */
static void lorawan_confirm_tx_finished(bool result)
{
  Serial.printf("Comfirmed TX finished with result %s", result ? "ACK" : "NAK");
}

/**@brief Function for sending a LoRa package.
 */
static void send_lora_frame(void)
{

  // Building the message to send
  unsigned char *puc;
  //int16_t dataTemp = 0; //(double)currentTEMP*10; //0; // from sensor
  //int16_t dataHumi = 0; //(double)currentHUMI*10;
  uint32_t i = 0;
  //Ticker ledTicker;

  if (lmh_join_status_get() != LMH_SET)
  {
    // Not joined, try again later
    Serial.println("Did not join network, skip sending frame");
    return;
  }

  // Building the message to send

//  char t100 = (char)(dataTemp / 100);
//  char t10  = (char)((dataTemp - (t100 * 100)) / 10);
//  char t1   = (char)((dataTemp - (t100 * 100) - (t10 * 10)) / 1);

  // Buffer contruction
  m_lora_app_data.port = LORAWAN_APP_PORT;

  puc = (unsigned char *)(&lat);
  m_lora_app_data.buffer[i++] = puc[0];
  m_lora_app_data.buffer[i++] = puc[1];
  m_lora_app_data.buffer[i++] = puc[2];
  m_lora_app_data.buffer[i++] = puc[3];
  puc = (unsigned char *)(&lon);
  m_lora_app_data.buffer[i++] = puc[0];
  m_lora_app_data.buffer[i++] = puc[1];
  m_lora_app_data.buffer[i++] = puc[2];
  m_lora_app_data.buffer[i++] = puc[3];

  //m_lora_app_data.buffer[i++] = (byte) (dataHumi >> 8) & 0xff; // dummy humi high byte
  //m_lora_app_data.buffer[i++] = (byte)  dataHumi       & 0xff; // dummy humi low  byte
  //m_lora_app_data.buffer[i++] = (byte) (dataTemp >> 8) & 0xff; // temp high byte
  //m_lora_app_data.buffer[i++] = (byte)  dataTemp       & 0xff; // temp low  byte
  m_lora_app_data.buffsize = i;

  Serial.print("Data: ");
  Serial.println((char *)m_lora_app_data.buffer);
  Serial.print("Size: ");
  Serial.println(m_lora_app_data.buffsize);
  Serial.print("Port: ");
  Serial.println(m_lora_app_data.port);

  //dataTemp += 1;
  //if (dataTemp >= 999)
  //  dataTemp = 0;

  lmh_error_status error = lmh_send(&m_lora_app_data, LMH_UNCONFIRMED_MSG);
  if (error == LMH_SUCCESS)
  {
  }

  Serial.printf("lmh_send result %d\n", error);
  //digitalWrite(LED_BUILTIN, LED_ON);
  //ledTicker.once(1, ledOff);
}

/**@brief Function for handling a LoRa tx timer timeout event.
 */
static void tx_lora_periodic_handler(void)
{
  TimerSetValue(&appTimer, LORAWAN_APP_TX_DUTYCYCLE);
  TimerStart(&appTimer);
  Serial.println("Sending frame");
  send_lora_frame();
}

static uint32_t timers_init(void)
{
  appTimer.timerNum = 3;
  TimerInit(&appTimer, tx_lora_periodic_handler);
  return 0;
}

uint8_t ELSAGetBatteryLevel(void)
{
	//uint8_t batteryLevel = 0; // 0-255

	//TO BE IMPLEMENTED

	return battLevel; //batteryLevel;
}

void modemOff(){
  digitalWrite(LTE_PWRKEY_PIN,LOW);
  vTaskDelay(100 / portTICK_PERIOD_MS);
  digitalWrite(LTE_PWRKEY_PIN,HIGH);
  vTaskDelay(6000 / portTICK_PERIOD_MS);
  digitalWrite(LTE_PWRKEY_PIN,LOW);
}

void modemOn(){
  digitalWrite(LTE_PWRKEY_PIN,HIGH);
  vTaskDelay(6000 / portTICK_PERIOD_MS);
  digitalWrite(LTE_PWRKEY_PIN,LOW);
}
