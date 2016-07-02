#ifndef LOSANT_DEVICE_ASYNC_HPP
#define LOSANT_DEVICE_ASYNC_HPP

#include <Losant.h>
#include <AsyncMqttClient.h>

#define LOSANT_TOPIC_MAXLENGTH 40

/**
 * \brief asynchronous Losant client
 */
class LosantDeviceAsync
{
public:
  explicit
  LosantDeviceAsync(const char* id = nullptr);

  const char*
  getId();

  void
  setId(const char* id);

  void
  setCredentials(const char* key, const char* secret);

  void
  onCommand(CommandCallback cb);

  bool
  connected();

  void
  connect();

  void
  disconnect();

  void
  sendState(JsonObject& state);

private:
  static void
  onMqttConnect();

  void
  onMqttConnect2();

  static void
  onMqttMessage(char* topic, char* payload, uint8_t qos, size_t len, size_t index, size_t total);

  void
  onMqttMessage2(char* topic, char* payload, uint8_t qos, size_t len, size_t index, size_t total);

private:
  static LosantDeviceAsync* s_self;
  const char* m_id;
  char m_stateTopic[LOSANT_TOPIC_MAXLENGTH];
  char m_commandTopic[LOSANT_TOPIC_MAXLENGTH];
  CommandCallback m_commandCallback;
  unsigned long m_lastConnect;
  AsyncMqttClient m_mqttClient;
};

#endif // LOSANT_DEVICE_ASYNC_HPP
