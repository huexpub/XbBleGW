#include "MijiaBleListener.h"

#pragma region 继承

void MijiaBleListener::reverse_(char *inChar){
  size_t l = strlen(inChar);
  for (int i = 0,j=l-2; i<j; i++,j=j-3)
  {
    char a = inChar[i];
    inChar[i] = inChar[j];
    inChar[j] = a;
    i++;
    j++;
    char b = inChar[i];
    inChar[i] = inChar[j];
    inChar[j] = b;
  }
}

char * MijiaBleListener::getJson_(char * inChar){
  char *json = strstr(inChar,"{");
  if (!json) {return NULL;}
  char *pch=strrchr(json,'}');
  if (pch)
  {
    json[pch-json+1] = 0;
    return json;
  } else {
    return NULL;
  }
}

void MijiaBleListener::handle_(){
  char *data = &rx_message_[0];
  int did;
  // 查找 ots 消息
  char *otsMessage = strstr(data,"ots:");
  if(!otsMessage){
    return;
  }
  Debug::AddInfo(PSTR("[otsMessage:] %s"), data);

  // 判断是否蓝牙事件
  if(!strstr(data,"_async.ble_event")){
    return;
  }
  char *json = getJson_(data);
  if(!json){
    Debug::AddInfo(PSTR("Json not found."));
    return;
  }

  // 蓝牙事件 json 例子:
  // {"id":1518998071,"method":"_async.ble_event","params":{"dev":{"did":"1011078646","mac":"AA:BB:CC:DD:EE:FF","pdid":794},"evt":[{"eid":7,"edata":"0036f6e45e"}],"frmCnt":97,"gwts":2362}}
  StaticJsonDocument<500> jsonData;
  DeserializationError error = deserializeJson(jsonData, json);

  // json 解析失败
  if(error){
    Debug::AddInfo(PSTR("Json parse failed. string:%s"), json);
    return;
  }

  // 获取 did
  const char* params_dev_did = jsonData["params"]["dev"]["did"];
  if(!params_dev_did){
    Debug::AddInfo(PSTR("did is not found."));
    return;
  }
  did = atoi(params_dev_did);
  if(did == 0){
    Debug::AddInfo(PSTR("did is not found."));
    return;
  }
  
  // 遍历数组获取edata与eid
  if(!jsonData["params"]["evt"].is<JsonArray>()){
    Debug::AddInfo(PSTR("evt is not an array."));
    return;
  }
  JsonArray arr = jsonData["params"]["evt"];
  // evt 结构是数组，以防数组内存在多个对象
  for (auto value : arr){
    char payload[500];
    serializeJson(value, payload, 500);
    int eid = value["eid"].as<int>();
    char edata[20]={0};
    // 如果找不到 edata 那直接发送当前 evt 数组对象到 mqtt
    if (!value["edata"].as<char*>()){
      Debug::AddInfo(PSTR("edata is not found."));
      this->mqttPublish(did, payload);
      break;
    }
    strcpy(edata,value["edata"].as<char*>());
    this->reverse_(edata);
    this->mqttPublish(did, eid, edata);
  }
}

// MQTT 发布
void MijiaBleListener::mqttPublish(int deviceID, const char *payload){
  char topic[80];
  String did = String(deviceID);
  sprintf(topic,"%s/unknown",Mqtt::getStatTopic(did).c_str());
  Mqtt::publish(topic, payload, globalConfig.mqtt.retain);
}

// MQTT 发布
void MijiaBleListener::mqttPublish(int deviceID, int eid, const char *edata){
  char topic[80];
  String did = String(deviceID);
  sprintf(topic,"%s/%d",Mqtt::getStatTopic(did).c_str(),eid);
  Mqtt::publish(topic, edata, globalConfig.mqtt.retain);
}

// 每行读取
void MijiaBleListener::readline_(int c)
{
  if (c > 0){
    switch (c) {
      case '\n':
        break;
      case 0x1b: // 这个字符会导致 json 错误
        break;
      case '\r':
      {
        // 字符串终止符
        rx_message_.push_back(0x00);
        // 解析 rx 的消息
        this->handle_();
        // 清除缓冲区
        this->rx_message_.clear();
        break;
      }
      default:
        this->rx_message_.push_back(c);
    }
  }
}

void MijiaBleListener::init(){
  Serial2.begin(115200,SERIAL_8N1,RX_PIN);
}

void MijiaBleListener::loop(){
  while (Serial2.available()) {
    this->readline_(Serial2.read());
  }
}

bool MijiaBleListener::moduleLed(){return false;}
void MijiaBleListener::perSecondDo(){}

void MijiaBleListener::readConfig(){}
void MijiaBleListener::resetConfig(){}
void MijiaBleListener::saveConfig(bool isEverySecond){}

void MijiaBleListener::mqttCallback(char *topic, char *payload, char *cmnd){}
void MijiaBleListener::mqttConnected(){}
void MijiaBleListener::mqttDiscovery(bool isEnable){}

void MijiaBleListener::httpAdd(WebServer *server){}
void MijiaBleListener::httpHtml(WebServer *server){}
String MijiaBleListener::httpGetStatus(WebServer *server){return "";}

#pragma endregion