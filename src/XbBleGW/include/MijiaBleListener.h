#ifndef _MijiaBleListener
#define _MijiaBleListener

#include <ArduinoJson.h>
#include "Module.h"

#define RX_PIN 23

class MijiaBleListener : public Module 
{
  void init();
  void loop();

  // 以下框架必须函数
  String getModuleName() { return F("XbBleGW"); }
  String getModuleCNName() { return F("米家蓝牙网关"); }
  String getModuleVersion() { return F("2020.12.07.1418"); }
  String getModuleAuthor() { return F("Neroxps"); }
  bool moduleLed();
  void perSecondDo();

  void readConfig();
  void resetConfig();
  void saveConfig(bool isEverySecond);

  void mqttCallback(char *topic, char *payload, char *cmnd);
  void mqttConnected();
  void mqttDiscovery(bool isEnable);

  void httpAdd(WebServer *server);
  void httpHtml(WebServer *server);
  String httpGetStatus(WebServer *server);
protected:
  // 读取每行
  void readline_(int c);
  // TTL 消息解析器
  void handle_();
  // edata 解析器
  // void edataHandle_(int deviceID, int eid, const char *edata);
  // 从 UART 消息中获取json字符串
  char *getJson_(char * inChar);
  // UART 消息缓冲区
  std::vector<char> rx_message_;
  // MQTT 发布
  void mqttPublish(int deviceID, const char *payload);
  void mqttPublish(int deviceID, int eid, const char *edata);
  // 两字节高低转换
  void reverse_(char *inChar);
};

#endif