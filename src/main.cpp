#include "Arduino.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

//#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
#include "Counter.h"
#include "webSrv.h"
#include "Settings.h"
#include "mqttClient.h"

#define MYPORT_TX D6
#define MYPORT_RX D5
#define RS485_TX D7

// void WiFiEvent(WiFiEvent_t event);
void counter_callback(Hiking_DDS238_2::results_t);


SoftwareSerial mySerial;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

Counter counter(&mySerial, RS485_TX, counter_callback);
Settings settings;

/******************************************************************************************/
void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.print("\n\nStart...");

  if (settings.isServiceMode()) {
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
  mqttClient::init();
}

/******************************************************************************************/
void loop() {
  webSrv::handle();
  counter.polling();
  if ((WiFi.status() != WL_CONNECTED) && (!settings.isServiceMode())) {
    Serial.println("\nWiFi not connected, reboot.");
    ESP.restart();
  } else {
    timeClient.update();
  }

  mqttClient::handle();
}

/******************************************************************************************/
void counter_callback(Hiking_DDS238_2::results_t res) {
  const char* mqttStrFormat =
      "{"
      "\"voltage\":\"%3.1f\","
      "\"current\": \"%3.1f\","
      "\"power\":\"%5.1f\","
      "\"pf\":\"%1.3f\","
      "\"f\":\"%2.2f\","
      "\"total\":\"%d\","
      "\"status\":\"%d\""
      "}";

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

  mqttClient::publish(settings.getSettings().mqttChannel, mqttStr);
  webSrv::setInfo(res);
}

/*
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
*/
