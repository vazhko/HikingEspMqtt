#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "Arduino.h"
#include "mqttClient.h"
#include "Settings.h"

#define LED D2

extern Settings settings;

namespace mqttClient {

void mqtt_reconnect();
void mqtt_callback(char* topic, byte* payload, unsigned int length);

WiFiClient m_espClient;
PubSubClient m_mqttClient(m_espClient);

/******************************************************************************************/
void init() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  m_mqttClient.setServer(settings.getSettings().mqttSrvAdr, 1883);
  m_mqttClient.setCallback(mqtt_callback);
}

/******************************************************************************************/
void handle() {
  mqtt_reconnect();
}

/******************************************************************************************/
bool publish(const char* topic, const char* payload) {
  return m_mqttClient.publish(topic, payload);
}

/******************************************************************************************/
void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  /*
  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the
  voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage
  HIGH
  }
  */
}

/******************************************************************************************/
void mqtt_reconnect() {
  typedef enum { mqtt_start,
                 mqtt_to_connect,
                 mqtt_connected,
                 mqtt_wait_connect,
  } mqtt_state_t;
  static mqtt_state_t mqtt_state = mqtt_start;
  static unsigned long time;

  switch (mqtt_state) {
    default:
    case mqtt_start:
      mqtt_state = mqtt_to_connect;
      break;
    case mqtt_connected:
      if (m_mqttClient.connected()) break;
      mqtt_state = mqtt_to_connect;
      break;
    case mqtt_to_connect:
      if (m_mqttClient.connect("arduinoClient", settings.getSettings().mqttUser, settings.getSettings().mqttPassword)) {
        Serial.println("mqtt connected");
        digitalWrite(LED, HIGH);
        mqtt_state = mqtt_connected;
        break;
      } else {
        time = millis();
        Serial.print("mqtt failed to connect, rc=");
        Serial.print(m_mqttClient.state());
        Serial.println(" try again in 5 seconds");
        digitalWrite(LED, LOW);
        mqtt_state = mqtt_wait_connect;
      }
      break;
    case mqtt_wait_connect:
      if ((millis() - time) < 5000) break;
      mqtt_state = mqtt_to_connect;
      break;
  }
}

};  // namespace mqttClient