#ifndef _COMMON_h

#include "Arduino.h"

/*********************************************************************************************\
 * ESP8266 Support
\*********************************************************************************************/

#ifdef ESP8266

#include <ESP8266Wifi.h>
#include <ESP8266WebServer.h>
#include <flash_hal.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include "sntp.h"

#define ESPHTTPUpdate ESPhttpUpdate
#define WebServer ESP8266WebServer

#define ESP_Restart() ESP.reset()
#define ESP_getChipId() ESP.getChipId()
#define ESP_getSketchSize() ESP.getSketchSize()
#define ESP_getResetReason() ESP.getResetReason()
#define WIFI_setHostname(aHostname) WiFi.hostname(aHostname)

#define DRAM_ATTR

#define spiflash_read(addr, buffer, size) (spi_flash_read((addr), (buffer), (size)) == SPI_FLASH_RESULT_OK)
#define spiflash_write(addr, data, size) (spi_flash_write((addr), (data), (size)) == SPI_FLASH_RESULT_OK)
#define spiflash_erase_sector(addr) (spi_flash_erase_sector((addr) / SPI_FLASH_SEC_SIZE) == SPI_FLASH_RESULT_OK)

extern uint32_t _EEPROM_start; //See EEPROM.cpp
#define EEPROM_PHYS_ADDR ((uint32_t)(&_EEPROM_start) - 0x40200000)

#endif

/*********************************************************************************************\
 * ESP32 Support
\*********************************************************************************************/
#ifdef ESP32

#include <WebServer.h>
#include <Wifi.h>
#include <esp_spi_flash.h>
#include <nvs.h>
#include <Update.h>
#include <HTTPUpdate.h>
#include <rom/rtc.h>
#include <EEPROM.h>

#define ARDUINO_ESP8266_RELEASE ""
#define EEPROM_PHYS_ADDR 0
#define ESPHTTPUpdate httpUpdate

#define ESP_Restart() ESP.restart()
#define WIFI_setHostname(aHostname)                     \
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE); \
    WiFi.setHostname(aHostname)

#define isFlashInterfacePin(p) 0

bool spiflash_init();
bool spiflash_erase_sector(size_t sector);
bool spiflash_write(size_t dest_addr, const void *src, size_t size);
bool spiflash_read(size_t src_addr, void *dest, size_t size);

uint32_t sntp_get_current_timestamp();

String ESP32GetResetReason(uint32_t cpu_no);
String ESP_getResetReason(void);
uint32_t ESP_getChipId(void);
uint32_t ESP_getSketchSize(void);

#define PWM_CHANNEL_OFFSET 8
uint32_t pin2chan(uint32_t pin);
void analogWrite(uint8_t pin, int val);
extern uint8_t pwm_channel[8];
#endif
#endif
