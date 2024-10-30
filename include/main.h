/*************************************************************************** *
 * @file main.h
 *************************************************************************** */
#ifndef _MAIN_H
#define _MAIN_H
#ifdef __cplusplus
extern "C" {
#endif

#define USE_SERIAL_DEBUG 1

#define USE_SENSOR_FLAG 0

#define BOARD_RESET()    ESP.restart()

#define devicerg "Not success"
#define fw_version "1.0.0RTD"

//#define out_topic "ROTHWELL/DATA/DOWN"
//#define in_topic  "ROTHWELL/DATA/UP"
#define in_topic "ROTHWELL/DATA/IoT_DOWN"
#define out_topic  "ROTHWELL/DATA/IoT_UP"
/*
    "DEVICE_NAME" : "Things_e502e403",
    "SENSOR_COUNT" : 2,
    "BACKLIGHT" : 80,
    "ETH_FLAG" : true,
    "WIFI_FLAG" : false,
    "BLE_FLAG" : true,
    "SOUND_FLAG" : false,
    "HUMI_FLAG" : true,
    "TIME_DATA" : 15,
    "ETH_DHCP" : true,
    "ETH_IP" : "192.168.1.177",
    "ETH_SUBNET" : "255.255.255.0",
    "ETH_GATEWAY" : "192.168.1.1",
    "ETH_DNS" : "8.8.8.8",
    "MQTT_HOST" : "examples.com",
    "MQTT_PORT" : 1883,
    "MQTT_USER" : "user",
    "MQTT_PASS" : "pass",
    "ADJ_TEMP" : 0.0,
    "MIN_TEMP" : 5.0,
    "MAX_TEMP" : 10.0,
    "WiFi_SSID" : "TA@ENTER_ Innovitae",
    "WiFi_PASS" : "Th!nkAnalyt!c01",
    "LoRa_DevEui"  : "0000fcc23d222ea3",
    "BATT"  : "2024/05/03"
    */
struct Config {
  char device_name[15];
  int sensor_count; // 1-6
  int  backlight;
  bool eth_flag;
  bool wifi_flag;
  bool ble_flag;
  bool sound_flag;
  bool humi_flag;
  int time_data;
  bool eth_dhcp;
  char eth_ip[16];
  char eth_subnet[16];
  char eth_gateway[16];
  char eth_dns[16];
  char mqtt_host[25];
  int  mqtt_port;
  char mqtt_user[25];
  char mqtt_pass[25];
  double adj_temp;
  double min_temp;
  double max_temp;
  char wifi_ssid[25];
  char wifi_pass[25];
  char lora_deveui[17]; // 16+1 // use dev_eui only for config
  char batt[32];
};

//void clock_count_24(int * hour, int * minute, int * seconds);
//void loadConfiguration(const char *filename, struct Config &config);
//void saveConfiguration(const char *filename, const struct Config &config);
char *Get_WiFiSSID_DD_List( void );
void WiFi_Init( void );
void WiFi_Connect(void);
void WiFi_ScanSSID( void );
void WarningCheck(void);

//void writeFile(fs::FS &fs, const char * path, const char * message);
//void appendFile(fs::FS &fs, const char * path, const char * message);

#ifdef __cplusplus
}
#endif
#endif // _MAIN_H