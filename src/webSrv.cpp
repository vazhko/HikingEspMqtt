#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include "Hiking-DDS238-2.h"
#include "Settings.h"

extern Hiking_DDS238_2 counter;
extern Settings settings;

namespace webSrv {

void handleRootPath();
void handleSaveParam();
ESP8266WebServer webServer(80);
Hiking_DDS238_2::results_t m_res;

unsigned long startTime = 0;

void init() {
  webServer.on("/", handleRootPath);
  webServer.on("/SaveParam", handleSaveParam);
  webServer.onNotFound([]() { webServer.send(404, "text / plain", "Page Not Found\n\n"); });
  webServer.begin();
}

void handle() {
  static bool first = true;
  if (first) {
    startTime = millis();
    first = false;
  }
  webServer.handleClient();
}

void handleRootPath() {
  String message = "";
  char str[1000];

  message += "<!DOCTYPE html>";
  message += "<html>";
  message += "<head>";
  message +=
      "<style>"
      "table {width: 650px;text-align: left;}"
      "table, th, td { border: 1px solid black; border-collapse: collapse;}"
      "th, td { padding: 2px;}"
      //"td {width: 50%%;}"
      "caption {text-align: left;}"
      "</style>";
  message += "<title>Hiking-DDS238</title>";
  // message += "<meta http-equiv='refresh' content='5'>"; // авто перезавантаження через 5 сек

  message += "</head>";
  message += "<body>";
  message += "<div style=\"width:800px; margin:0 auto;\">";

  message += "<h3>Hiking-DDS238-2</h3>";

  // message += "<br>";

  // Settings
  message += "<table>";
  message += "<caption><h4>Parameters</h4></caption>";

  sprintf(str,
          "<tr><td style=\"width: 50%%;\">SSID</td>"
          "<td><form action=\"/SaveParam\" method=\"get\"><input value=\"%s\" name=\"SSID\"><button type=\"submit\">Save</button></form></td></tr>",
          settings.getSettings().ssid);
  message += str;
  sprintf(str,
          "<tr><td>Password</td>"
          "<td><form action=\"/SaveParam\" method=\"get\"><input value=\"%s\" name=\"Password\"><button type=\"submit\">Save</button></form></td></tr>",
          settings.getSettings().password);
  message += str;
  sprintf(str,
          "<tr><td>MQTT server</td>"
          "<td><form action=\"/SaveParam\" method=\"get\"><input value=\"%s\" name=\"MQTT server\"><button type=\"submit\">Save</button></form></td></tr>",
          settings.getSettings().mqttSrvAdr);
  message += str;
  sprintf(str,
          "<tr><td>MQTT channell</td>"
          "<td><form action=\"/SaveParam\" method=\"get\"><input value=\"%s\" name=\"MQTT channell\"><button type=\"submit\">Save</button></form></td></tr>",
          settings.getSettings().mqttChannel);
  message += str;

  message += "</table>";
  message += "<br>";

  message += "<table>";

  if (m_res.err == Hiking_DDS238_2::errOk)
    message += "<caption style=\"color: green;\"><h4>Hiking DDS238 (Status: online)</h4></caption>";
  else
    message += "<caption style=\"color: red;\"><h4>Hiking DDS238 (Status: error)</h4></caption>";

  sprintf(str, "<tr><td style=\"width: 50%%;\">U</td><td>%3.1fV</td></tr>", m_res.u);
  message += str;
  sprintf(str, "<tr><td>I</td><td>%3.1fA</td></tr>", m_res.i);
  message += str;
  sprintf(str, "<tr><td>PF</td><td>%1.3f</td></tr>", m_res.pf);
  message += str;
  sprintf(str, "<tr><td>P</td><td>%5.1fW</td></tr>", m_res.p);
  message += str;
  sprintf(str, "<tr><td>NoErr/Errors count</td><td>%ld/%ld(%3.1f%%)</td></tr>",
          m_res.noErrCnt, m_res.errCnt, (double)m_res.errCnt * 100.0 / (double)m_res.noErrCnt);
  message += str;
  sprintf(str, "<tr><td>Error code</td><td>%d</td></tr>", m_res.err);
  message += str;

  message += "</table>";
  message += "<br>";

  message += "<table>";
  message += "<caption><h4>Extreems</h4></caption>";

  sprintf(str,
          "<tr><td style=\"width: 50%%;\">U max</td>"
          "<td><form action=\"/SaveParam\" method=\"get\"><input value=\"%3.1fV\" name=\"U max\"><button type=\"submit\">Reset</button></form></td></tr>",
          settings.getData().voltageMax.val);
  message += str;
  sprintf(str,
          "<tr><td>I max</td>"
          "<td><form action=\"/SaveParam\" method=\"get\"><input value=\"%3.1fA\" name=\"I max\"><button type=\"submit\">Reset</button></form></td></tr>",
          settings.getData().currentMax.val);
  message += str;
  sprintf(str,
          "<tr><td>Working time, s</td>"
          "<td><form action=\"/SaveParam\" method=\"get\"><input value=\"%lu\" name=\"Reboot\"><button type=\"submit\">Reboot</button></form></td></tr>",
          (millis() - startTime) / 1000);
  message += str;

  message += "</table>";
  message += "<br>";

  message += "</div>";
  message += "</body>";
  message += "</html>";

  webServer.send(200, "text/html", message);
}

void setInfo(Hiking_DDS238_2::results_t& res) {
  m_res = res;
}

void handleSaveParam() {
  Serial.print("Save");
  String message = "";

  if (webServer.argName(0) == "SSID") {
     sprintf(settings.getSettings().ssid, webServer.arg(0).c_str());
     settings.syncSettings();
  }
  if (webServer.argName(0) == "Password") {
     sprintf(settings.getSettings().password, webServer.arg(0).c_str());
     settings.syncSettings();
  }
  if (webServer.argName(0) == "MQTT server") {
    // sprintf(settings.getSettings().mqttSrvAdr, webServer.arg(0).c_str());
    // settings.syncSettings();
  }
  if (webServer.argName(0) == "MQTT channell") {
    // sprintf(settings.getSettings().mqttChannel, webServer.arg(0).c_str());
    // settings.syncSettings();
  }
  if (webServer.argName(0) == "U max") {
    settings.resetDataU();
  }
  if (webServer.argName(0) == "I max") {
    settings.resetDataI();
  }
  if (webServer.argName(0) == "Reboot") {
    Serial.print("\nRestart");
    if(digitalRead(D1)){
      Serial.print(" .Pin D1 is 1");
    } else {
      Serial.print(" .Pin D1 is 0");
    }
    ESP.restart();
  }
  webServer.sendHeader("Location", "/");  // Add a header to respond with a new location for the browser to go to the home page again
  webServer.send(303);
}

}  // namespace webSrv
