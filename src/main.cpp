#include "Arduino.h"
// #include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

#include "dateTime.h"
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

// WiFiUDP ntpUDP;
// NTPClient timeClient(ntpUDP);

Counter counter(&mySerial, RS485_TX, counter_callback);
Settings settings;

/******************************************************************************************/
void startAP() {
  WiFi.persistent(false);
  IPAddress apIP(192, 168, 5, 1);
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  delay(500);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("esp_haking", "12345678");
  Serial.println("AP \"esp_haking (vv)\" has began");
  Serial.println(WiFi.softAPSSID());
  Serial.println(WiFi.softAPIP());
}

/******************************************************************************************/
void setup() {
  delay(1000);
  Serial.begin(115200);
  Serial.print("\n\nStart...");

  if (settings.isServiceMode()) {
    startAP();
  } else {
    Serial.print("Connecting to ");
    Serial.println(settings.getSettings().ssid);
    WiFi.begin(settings.getSettings().ssid, settings.getSettings().password);

    uint8_t tryes = 20;
    while ((WiFi.status() != WL_CONNECTED) && (tryes > 0)) {
      delay(500);
      Serial.print(".");
      tryes--;
    }
    if (tryes == 0) {
      settings.setServiceMode(true);
      startAP();
    } else {
      Serial.println("");
      Serial.println("WiFi connected.");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
    }
  }

  mySerial.begin(9600, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);

  /// timeClient.begin();
  webSrv::init();
  mqttClient::init();
  /// dateTime::init();
}

/******************************************************************************************/
void loop() {
  webSrv::handle();
  counter.polling();
  if ((WiFi.status() != WL_CONNECTED) && (!settings.isServiceMode())) {
    Serial.println("\nWiFi not connected, reboot.");
    ESP.restart();
  } else {
    /// timeClient.update();
  }

  mqttClient::handle();
  /// dateTime::handle();
}

/******************************************************************************************/
void counter_callback(Hiking_DDS238_2::results_t res) {
  static uint32_t allCnt, badCnt;
  const char* mqttStrFormat =
      "{"
      "\"voltage\":\"%3.1f\","
      "\"current\": \"%3.1f\","
      "\"power\":\"%5.1f\","
      "\"pf\":\"%1.3f\","
      "\"f\":\"%2.2f\","
      "\"total\":\"%d\","
      "\"u_max\":\"%3.1f\","
      "\"i_max\":\"%3.1f\","
      "\"all_req\":\"%d\","
      "\"bad_req\":\"%d\","
      "\"status\":\"%d\""
      "}";

  allCnt ++;
  char mqttStr[500];
  if (res.err != Hiking_DDS238_2::errOk) {
    sprintf(mqttStr, "{\"status\":\"%d\"}", res.err);
    badCnt ++;
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
      sprintf(mqttStr, mqttStrFormat, res.u, res.i, res.p, res.pf, res.f, res.totalCnt,
              settings.getData().voltageMax.val, settings.getData().currentMax.val,
              allCnt, badCnt,
              res.err);
      Serial.println("Hiking: ");
      Serial.println(mqttStr);
      settings.checkData(res.u, res.i);
      break;
  }

  mqttClient::publish(settings.getSettings().mqttChannel, mqttStr);
  // webSrv::setInfo(res);
  webSrv::setInfo(mqttStr);
}
