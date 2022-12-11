#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
// #include <NTPClient.h>
// #include <ezTime.h>

#include "Hiking-DDS238-2.h"
#include "Settings.h"
#include "mainHtmlPage.h"

extern Hiking_DDS238_2 counter;
extern Settings settings;
// extern NTPClient timeClient;

namespace webSrv {

// void handleRootPath();
void handleSaveParam();
void handleIndex();
void handleGetData();
void handleGetParam();

ESP8266WebServer webServer(80);
Hiking_DDS238_2::results_t m_res;

unsigned long startTime = 0;
unsigned long rebootTime = 0;
// char firstStartTimeStr[60];
String firstStartTime;
String dataStr = "{}";

/******************************************************************************************/
void init() {
  webServer.on("/", handleIndex);
  webServer.on("/data", handleGetData);
  webServer.on("/param", handleGetParam);
  webServer.onNotFound([]() { webServer.send(404, "text/plain", "Page Not Found\n\n"); });
  webServer.begin();
}

/******************************************************************************************/
void reboot() {
  rebootTime = 3000;
}

/******************************************************************************************/
void handle() {
  webServer.handleClient();

  if (rebootTime) {
    rebootTime--;
    if (!rebootTime) {
      Serial.print("\nRestart");
      ESP.restart();
    }
  }
}

/******************************************************************************************/
void setInfo(Hiking_DDS238_2::results_t& res) {
  m_res = res;
}

void setInfo(char* str) {
  dataStr = str;
}

/******************************************************************************************/
void handleIndex() {
  webServer.send(200, "text/html", mainPage);
}

void handleGetData() {
  webServer.send(200, "text/plane", dataStr);
}

void handleGetParam() {
  const char* strFormat =
      "{"
      "\"wifi_ssid\":\"%s\","
      "\"wifi_password\":\"%s\","
      "\"mqtt_server\":\"%s\","
      "\"mqtt_user\":\"%s\","
      "\"mqtt_password\":\"%s\","
      "\"mqtt_channell\":\"%s\""
      "}";

  const String tasks[] = {"wifi_ssid", "wifi_password", "mqtt_server", "mqtt_user", "mqtt_password", "mqtt_channell", "reboot"};  
  for (int i = 0; i < 7; i++) {
    String resp = webServer.arg(tasks[i]);
    if (resp != "") {          
      if (i == 0) {
        Serial.println(resp.c_str());  
        strcpy(settings.getSettings().ssid, resp.c_str());
      } else if (i == 1){
        strcpy(settings.getSettings().password, resp.c_str());      
      } else if (i == 2){
        strcpy(settings.getSettings().mqttSrvAdr, resp.c_str());        
      } else if (i == 3){
        strcpy(settings.getSettings().mqttUser, resp.c_str());
      } else if (i == 4){
        strcpy(settings.getSettings().mqttUser, resp.c_str());
      } else if (i == 5){
        strcpy(settings.getSettings().mqttChannel, resp.c_str());
      } else if (i == 6){
        reboot();
      }
      if (i != 6)settings.syncSettings();

      webServer.send(200, "text/plane", "{\"" + tasks[i] + "\":\"" + resp + "\"}");
      return;
    }
  }

  char str[1000];
  sprintf(str, strFormat,
          settings.getSettings().ssid,
          settings.getSettings().password,
          settings.getSettings().mqttSrvAdr,
          settings.getSettings().mqttUser,
          settings.getSettings().mqttPassword,
          settings.getSettings().mqttChannel);
  webServer.send(200, "text/plane", str);
}

}  // namespace webSrv