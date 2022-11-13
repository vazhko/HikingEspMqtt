#include "Arduino.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
//#include <Preferences.h>
//#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
#include "Counter.h"
#include "webSrv.h"
#include <PubSubClient.h>

#define MYPORT_TX D6
#define MYPORT_RX D5
#define RS485_TX D7
SoftwareSerial mySerial;

const char* ssid = "Keenetic-2568";
const char* password = "T9TW8iHR";
const char* adr_mqtt_server = "192.168.1.105";
const char* mqtt_payload = "esp/hiking/all";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void hikingPolling();
void mqtt_reconnect();
void mqtt_callback(char* topic, byte* payload, unsigned int length);

void counter_callback(char* res, Counter::err_t err);
Counter counter(&mySerial, RS485_TX, counter_callback);

/******************************************************************************************/
void setup() {
  Serial.begin(115200);
  Serial.print("Start...");

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  mySerial.begin(9600, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);

  webSrv::init();

  mqttClient.setServer(adr_mqtt_server, 1883);
  mqttClient.setCallback(mqtt_callback);
}

/******************************************************************************************/
void loop() {
  webSrv::handle();
  counter.polling();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi not connected, reboot.");
  }
  if (!mqttClient.connected()) {
    mqtt_reconnect();
  }
  mqttClient.loop();
}

/******************************************************************************************/
void counter_callback(char* res, Counter::err_t err) {
  if (err != Counter::errOk) {
    //Serial.println("Hiking: err");
    switch (err) {
    case Counter::errTimeout:
      Serial.println("Hiking: errTimeout");
      break;
    case Counter::errCs:
      Serial.println("Hiking: errCs");
      break;
    case Counter::errUnk:
      Serial.println("Hiking: errUnk");
      break;
    }
    return;
  }
  Serial.print("Hiking: ");
  Serial.println(res);
  mqttClient.publish(mqtt_payload, res);
}

/******************************************************************************************/
void mqtt_reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect("arduinoClient", "root", "vv")) {
      Serial.println("mqtt connected");
      // Once connected, publish an announcement...
      // client.publish("outTopic", "hello world");
      // ... and resubscribe
      mqttClient.subscribe("inTopic");
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
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
