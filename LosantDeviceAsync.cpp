#include "LosantDeviceAsync.hpp"
#include <PString.h>

LosantDeviceAsync* LosantDeviceAsync::s_self = nullptr;

LosantDeviceAsync::LosantDeviceAsync(const char* id)
  : m_id(nullptr)
  , m_lastConnect(0)
{
  LosantDeviceAsync::s_self = this;

  m_mqttClient.onConnect(LosantDeviceAsync::onMqttConnect);
  m_mqttClient.onMessage(LosantDeviceAsync::onMqttMessage);
  m_mqttClient.setServer(LOSANT_BROKER, LOSANT_PORT);

  if (id != nullptr) {
    this->setId(id);
  }
}

const char*
LosantDeviceAsync::getId()
{
  return m_id;
}

void
LosantDeviceAsync::setId(const char* id)
{
  m_id = id;
  m_mqttClient.setClientId(m_id);

  PString stateTopic(m_stateTopic, sizeof(m_stateTopic));
  stateTopic.print(LOSANT_TOPIC_PREFIX);
  stateTopic.print(m_id);
  stateTopic.print(LOSANT_TOPIC_STATE);

  PString commandTopic(m_commandTopic, sizeof(m_commandTopic));
  commandTopic.print(LOSANT_TOPIC_PREFIX);
  commandTopic.print(m_id);
  commandTopic.print(LOSANT_TOPIC_COMMAND);
}

void
LosantDeviceAsync::setCredentials(const char* key, const char* secret)
{
  m_mqttClient.setCredentials(key, secret);
}

void
LosantDeviceAsync::onCommand(CommandCallback cb)
{
  m_commandCallback = cb;
}

bool
LosantDeviceAsync::connected()
{
  return m_mqttClient.connected();
}

void
LosantDeviceAsync::connect()
{
  unsigned long now = millis();
  if (now - m_lastConnect < 2000) {
    return;
  }
  m_lastConnect = now;

  m_mqttClient.connect();
}

void
LosantDeviceAsync::disconnect()
{
  m_lastConnect = 0;
  m_mqttClient.disconnect();
}

void
LosantDeviceAsync::sendState(JsonObject& state)
{
  StaticJsonBuffer<100> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["data"] = state;

  String stateStr;
  root.printTo(stateStr);

  m_mqttClient.publish(m_stateTopic, 0, false, stateStr.c_str(), stateStr.length());
}

void
LosantDeviceAsync::onMqttConnect()
{
  LosantDeviceAsync::s_self->onMqttConnect2();
}

void
LosantDeviceAsync::onMqttConnect2()
{
  m_mqttClient.subscribe(m_commandTopic, 0);
}

void
LosantDeviceAsync::onMqttMessage(char* topic, char* payload, uint8_t qos, size_t len, size_t index, size_t total)
{
  LosantDeviceAsync::s_self->onMqttMessage2(topic, payload, qos, len, index, total);
}

void
LosantDeviceAsync::onMqttMessage2(char* topic, char* payload, uint8_t qos, size_t len, size_t index, size_t total)
{
  if (strcmp(topic, m_commandTopic) != 0) {
    return;
  }

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payload);
  if (root.success()) {
    LosantCommand command;
    command.name = root["name"];
    command.time = root["$time"];
    command.payload = &(root["payload"].asObject());

    m_commandCallback(&command);
  }
}
