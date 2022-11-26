#include "Arduino.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>

#include <ESP8266WiFiMulti.h>

//#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
#include "Counter.h"
#include "webSrv.h"
#include "Settings.h"

#define MYPORT_TX D6
#define MYPORT_RX D5
#define RS485_TX D7

#define LED D2

void hikingPolling();
void mqtt_reconnect();
void WiFiEvent(WiFiEvent_t event);
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void counter_callback(Hiking_DDS238_2::results_t);
bool isServiceMode() { return (0 == digitalRead(D1)); }

SoftwareSerial mySerial;
WiFiClient espClient;
PubSubClient mqttClient(espClient);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);


Counter counter(&mySerial, RS485_TX, counter_callback);
Settings settings;

/******************************************************************************************/
void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.print("\n\nStart...");

  if (isServiceMode()) {
    WiFi.persistent(false);

    IPAddress apIP(192, 168, 5, 1);
    // delay(500);
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    delay(500);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP("esp_haking", "12345678");
    // Serial.println("AP \"esp_haking (vv)\" has began");

    Serial.println(WiFi.softAPSSID());
    Serial.println(WiFi.softAPIP());

  } else {
    Serial.print("Connecting to ");
    Serial.println(settings.getSettings().ssid);
    WiFi.begin(settings.getSettings().ssid, settings.getSettings().password);

    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    // WiFi.onEvent(WiFiEvent);

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  mySerial.begin(9600, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);

  timeClient.begin();

  webSrv::init();

  mqttClient.setServer(settings.getSettings().mqttSrvAdr, 1883);
  mqttClient.setCallback(mqtt_callback);

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);  
}

/******************************************************************************************/
void loop() {
  webSrv::handle();
  counter.polling();
  if ((WiFi.status() != WL_CONNECTED) && (!isServiceMode())) {
    Serial.println("\nWiFi not connected, reboot.");
    ESP.restart();
  } else {
    timeClient.update();
  }

  //if (isServiceMode()) return;

  mqtt_reconnect();
  mqttClient.loop();
}

/******************************************************************************************/
void counter_callback(Hiking_DDS238_2::results_t res) {
  const char* mqttStrFormat =
      "{ \
  \"voltage\":\"%3.1f\", \
  \"current\": \"%3.1f\", \
  \"power\":\"%5.1f\", \
  \"pf\":\"%1.3f\", \
  \"f\":\"%2.2f\", \
  \"total\":\"%d\", \
  \"status\":\"%d\" \
  }";

  char mqttStr[500];
  if (res.err != Hiking_DDS238_2::errOk) {
    sprintf(mqttStr, "{\"status\":\"%d\"}", res.err);
  }

  switch (res.err) {
    case Hiking_DDS238_2::errTimeout:
      Serial.println("Hiking: errTimeout");
      break;
    case Hiking_DDS238_2::errCs:
      Serial.println("Hiking: errCs");
      break;
    case Hiking_DDS238_2::errUnk:
      Serial.println("Hiking: errUnk");
      break;
    case Hiking_DDS238_2::errOk:
      sprintf(mqttStr, mqttStrFormat, res.u, res.i, res.p, res.pf, res.f,
              res.totalCnt, res.err);
      Serial.println("Hiking: ");
      Serial.println(mqttStr);
      settings.checkData(res.u, res.i);
      break;
  }

  mqttClient.publish(settings.getSettings().mqttChannel, mqttStr);
  webSrv::setInfo(res);
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
      if (mqttClient.connected()) break;      
      mqtt_state = mqtt_to_connect;
      break;
    case mqtt_to_connect:
      if (mqttClient.connect("arduinoClient", settings.getSettings().mqttUser, settings.getSettings().mqttPassword)) {
        Serial.println("mqtt connected");
        digitalWrite(LED, HIGH);
        mqtt_state = mqtt_connected;        
        break;
      } else {
        time = millis();        
        Serial.print("mqtt failed to connect, rc=");
        Serial.print(mqttClient.state());
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

void WiFiEvent(WiFiEvent_t event) {
  switch (event) {
    case WIFI_EVENT_SOFTAPMODE_STACONNECTED:
      Serial.println("WIFI_EVENT_SOFTAPMODE_STACONNECTED");
      Serial.println(WiFi.softAPIP());
      break;
    case WIFI_EVENT_SOFTAPMODE_STADISCONNECTED:
      Serial.println("WIFI_EVENT_SOFTAPMODE_STADISCONNECTED");
      Serial.println(WiFi.softAPIP());
      break;
    default:
      Serial.print("event ");
      Serial.println(event);
      break;
  }
}
