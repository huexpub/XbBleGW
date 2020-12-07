#ifndef DISABLE_MQTT

#include "Mqtt.h"
#include "Module.h"
#include "Rtc.h"

uint8_t Mqtt::operationFlag = 0;
PubSubClient Mqtt::mqttClient;
uint32_t Mqtt::lastReconnectAttempt = 0;   // 最后尝试重连时间
uint32_t Mqtt::kMqttReconnectTime = 60000; // 重新连接尝试之间的延迟（ms）
std::function<void()> Mqtt::connectedcallback = NULL;

bool Mqtt::mqttConnect()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        Debug::AddInfo(PSTR("wifi disconnected"));
        return false;
    }
    if (globalConfig.mqtt.port == 0)
    {
        Debug::AddInfo(PSTR("no set mqtt info"));
        return false;
    }
    if (mqttClient.connected())
    {
        return true;
    }

    Debug::AddInfo(PSTR("client mqtt not connected, trying to connect to %s:%d Broker"), globalConfig.mqtt.server, globalConfig.mqtt.port);
    mqttClient.setServer(globalConfig.mqtt.server, globalConfig.mqtt.port);
    if (mqttClient.connect(UID, globalConfig.mqtt.user, globalConfig.mqtt.pass, getTeleTopic(F("availability")).c_str(), 0, true, "offline"))
    {
        Debug::AddInfo(PSTR("successful client mqtt connection"));
        availability();
        if (globalConfig.mqtt.interval > 0)
        {
            doReportInfo();
        }
        if (connectedcallback != NULL)
        {
            connectedcallback();
        }
    }
    else
    {
        Debug::AddInfo(PSTR("Connecting to %s:%d Broker . . failed, rc=%d"), globalConfig.mqtt.server, globalConfig.mqtt.port, mqttClient.state());
    }
    return mqttClient.connected();
}

void Mqtt::doReportInfo()
{
    char message[250];
    sprintf(message, PSTR("{\"uid\":\"%s\",\"ssid\":\"%s\",\"rssi\":\"%s\",\"version\":\"%s\",\"ip\":\"%s\",\"mac\":\"%s\",\"freemem\":%d,\"uptime\":%d,\"buildtime\":\"%s\"}"),
            UID, WiFi.SSID().c_str(), String(WiFi.RSSI()).c_str(), (module ? module->getModuleVersion().c_str() : PSTR("0")), WiFi.localIP().toString().c_str(), WiFi.macAddress().c_str(), ESP.getFreeHeap(), millis() / 1000, Rtc::GetBuildDateAndTime().c_str());
    //Debug::AddInfo(PSTR("%s"), message);
    publish(getTeleTopic(F("info")), message);
}

void Mqtt::availability()
{
    publish(getTeleTopic(F("availability")), "online", true);
}

void Mqtt::perSecondDo()
{
    bitSet(operationFlag, 0);
}

void Mqtt::loop()
{
    if (WiFi.status() != WL_CONNECTED || globalConfig.mqtt.port == 0)
    {
        return;
    }
    uint32_t now = millis();
    if (!mqttClient.connected())
    {
        if (now - lastReconnectAttempt > kMqttReconnectTime || lastReconnectAttempt == 0)
        {
            lastReconnectAttempt = now;
            if (mqttConnect())
            {
                lastReconnectAttempt = 0;
            }
        }
    }
    else
    {
        mqttClient.loop();
        if (bitRead(operationFlag, 0))
        {
            bitClear(operationFlag, 0);
            if (globalConfig.mqtt.interval > 0 && (perSecond % globalConfig.mqtt.interval) == 0)
            {
                doReportInfo();
            }
            if (perSecond % 3609 == 0)
            {
                availability();
            }
        }
    }
}

String Mqtt::getCmndTopic(String topic)
{
    return getTopic(0, topic);
}

String Mqtt::getStatTopic(String topic)
{
    return getTopic(1, topic);
}

String Mqtt::getTeleTopic(String topic)
{
    return getTopic(2, topic);
}

void Mqtt::mqttSetLoopCallback(MQTT_CALLBACK_SIGNATURE)
{
    mqttClient.setCallback(callback);
}

void Mqtt::mqttSetConnectedCallback(MQTT_CONNECTED_CALLBACK_SIGNATURE)
{
    Mqtt::connectedcallback = connectedcallback;
}

PubSubClient &Mqtt::setClient(Client &client)
{
    return mqttClient.setClient(client);
}

bool Mqtt::publish(const char *topic, const char *payload, bool retained)
{
    return mqttClient.publish(topic, payload, retained);
}

bool Mqtt::publish(const char *topic, const uint8_t *payload, unsigned int plength, bool retained)
{
    return mqttClient.publish(topic, payload, plength, retained);
}

bool Mqtt::subscribe(const char *topic, uint8_t qos)
{
    return mqttClient.subscribe(topic, qos);
}
bool Mqtt::unsubscribe(const char *topic)
{
    return mqttClient.unsubscribe(topic);
}

String Mqtt::getTopic(uint8_t prefix, String subtopic)
{
    // 0: Cmnd  1:Stat 2:Tele
    String fulltopic = String(globalConfig.mqtt.topic);
    if ((0 == prefix) && (-1 == fulltopic.indexOf(F("%prefix%"))))
    {
        fulltopic += F("/%prefix%"); // Need prefix for commands to handle mqtt topic loops
    }
    fulltopic.replace(F("%prefix%"), (prefix == 0 ? F("cmnd") : ((prefix == 1 ? F("stat") : F("tele")))));
    fulltopic.replace(F("%hostname%"), UID);
    fulltopic.replace(F("%module%"), module ? module->getModuleName() : F("module"));
    fulltopic.replace(F("#"), F(""));
    fulltopic.replace(F("//"), F("/"));
    if (!fulltopic.endsWith(F("/")))
        fulltopic += F("/");
    return fulltopic + subtopic;
}
#endif