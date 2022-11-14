#include "Arduino.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <Preferences.h>

//#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
#include "Counter.h"
#include "webSrv.h"
#include <PubSubClient.h>

#define MYPORT_TX D6
#define MYPORT_RX D5
#define RS485_TX D7
SoftwareSerial mySerial;

const char* ssidDef = "Keenetic-2568";
const char* passwordDef = "T9TW8iHR";
const char* mqttSrvAdrDef = "192.168.1.105";
const char* mqttChannelDef = "esp/hiking/all";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void hikingPolling();
void mqtt_reconnect();
void mqtt_callback(char* topic, byte* payload, unsigned int length);

void counter_callback(Hiking_DDS238_2::results_t);
Counter counter(&mySerial, RS485_TX, counter_callback);

Preferences prefs;

typedef struct {
  char ssid[128];
  char password[128];
  char mqttSrvAdr[128];
  char mqttChannel[128];
  uint16_t pollTime;
} pref_t;

pref_t settings;
/******************************************************************************************/
void setup() {

  Serial.begin(115200);
  Serial.print("\n\nStart...");

  prefs.begin("my-app"); // use "my-app" namespace

  if (!prefs.isKey("settings")) {
    strcpy(settings.ssid, ssidDef);
    strcpy(settings.password, passwordDef);
    strcpy(settings.mqttSrvAdr, mqttSrvAdrDef);
    strcpy(settings.mqttChannel, mqttChannelDef);    
    prefs.putBytes("settings", &settings, sizeof(settings));
    Serial.print("\nCreate \"my-app\" namespace\n");
  }
  prefs.getBytes("settings", &settings, sizeof(settings));


  Serial.print("Connecting to ");
  Serial.println(settings.ssid);
  WiFi.begin(settings.ssid, settings.password);
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

  mqttClient.setServer(settings.mqttSrvAdr, 1883);
  mqttClient.setCallback(mqtt_callback);
}

/******************************************************************************************/
void loop() {
  webSrv::handle();
  counter.polling();
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\nWiFi not connected, reboot.");
    ESP.restart();
  }
  if (!mqttClient.connected()) {
    mqtt_reconnect();
  }
  mqttClient.loop();
}

/******************************************************************************************/
void counter_callback(Hiking_DDS238_2::results_t res) {

  const char* mqttStrFormat = "{ \
  \"voltage\":\"%3.1f\", \
  \"current\": \"%3.1f\", \
  \"power\":\"%5.1f\", \
  \"pf\":\"%1.3f\", \
  \"f\":\"%2.2f\", \
  \"total\":\"%d\", \
  \"status\":\"%d\" \
  }";

  char mqttStr[500];


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

    sprintf(mqttStr, mqttStrFormat, res.u, res.i, res.p, res.pf, res.f, res.totalCnt, res.err);
    mqttClient.publish(settings.mqttChannel, mqttStr);

    Serial.println("Hiking: ");
    Serial.println(mqttStr);  

    break;
  }

  webSrv::setInfo(res);



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
