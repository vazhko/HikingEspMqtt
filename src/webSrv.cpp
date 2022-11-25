#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include	"Hiking-DDS238-2.h"
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
    webServer.onNotFound([]() {   webServer.send(404, "text / plain", "Page Not Found\n\n");  });
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
    char str[2000];

    message += "<!DOCTYPE html>";
    message += "<html>";
    message += "<head>";
    message += "<style>table, th, td { border: 1px solid black; border-collapse: collapse;}th, td { padding: 15px;}</style>";
    message += "<title>Hiking-DDS238</title>";
    //message += "<meta http-equiv='refresh' content='5'>"; // авто перезавантаження через 5 сек
    //message += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    message += "</head>";
    message += "<body>";
    message += "<h1>Hello from Hiking-DDS238!</h1>";
    //message += "<div>";
    message += "<table  style=\"text-align: left; width: 650px;\">";
    message += "<caption><h4>Parameters</h4></caption>";
    //Settings
    //sprintf(str, "<tr><td style=\"width: 50\%;\" contenteditable=\"true\">SSID</td><td>%s</td></tr>", settings.getSettings().ssid);
    sprintf(str, "<tr><td style=\"width: 40%%;\">SSID</td><td style=\"width: 40%%;\"><form action=\"/SaveParam\" method=\"get\"><input value=\"%s\" name=\"SSID\"><button type=\"submit\">Save</button></form></td></tr>",
      settings.getSettings().ssid);
    message += str;
    sprintf(str, "<tr><td>Password</td><td><input value=\"%s\"></td></tr>",
      settings.getSettings().password);
    message += str;
    sprintf(str, "<tr><td>MQTT server</td><td><input value=\"%s\"></td></tr>",
      settings.getSettings().mqttSrvAdr);
    message += str;
    sprintf(str, "<tr><td>MQTT channell</td><td><input value=\"%s\"></td></tr>",
      settings.getSettings().mqttChannel);
    message += str;

    message += "</table>";

    message += "<table>";
    message += "<caption>Counter</caption>";

    if (m_res.err == Hiking_DDS238_2::errOk) {
      sprintf(str, "<tr><td>U</td><td>%3.1fV</td></tr>", m_res.u);
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
    }
    else {
      sprintf(str, "<tr><td>Error</td><td>%d</td></tr>", m_res.err);
      message += str;
      sprintf(str, "<tr><td>NoErr/Err</td><td>%ld/%ld(%3.1f%%)</td></tr>",
        m_res.noErrCnt, m_res.errCnt, (double)m_res.errCnt * 100.0 / (double)m_res.noErrCnt);
      message += str;
    }
    message += "</table>";

    message += "<table>";
    message += "<caption>Extreems</caption>";
    sprintf(str, "<tr><td>U max</td><td>%3.1fV</td></tr>", settings.getData().voltageMax.val);
    message += str;
    sprintf(str, "<tr><td>I max</td><td>%3.1fA</td></tr>", settings.getData().currentMax.val);
    message += str;
    sprintf(str, "<tr><td>Working time, s</td><td>%lu</td></tr>", (millis() - startTime) / 1000);
    message += str;

    message += "</table>";


    //message += "</div>";
    message += "</body>";
    message += "</html>";

    webServer.send(200, "text/html", message);
  }

  void setInfo(Hiking_DDS238_2::results_t& res) {
    m_res = res;
  }

  void handleSaveParam(){
    Serial.print("Save");
    String message = "";
    //String req = webServer.
    //webServer.send(200, "text/html");


    for (int i = 0; i < webServer.args(); i++) {        
        message += webServer.argName(i) + ": ";     //Get the name of the parameter
        message += webServer.arg(i) + "\n";              //Get the value of the parameter
    }
    if(webServer.argName(0) == "SSID"){
      sprintf(settings.getSettings().ssid, webServer.arg(0).c_str());
    }
    //webServer.send(200, "text/html", message);
    //handleRootPath();
    webServer.sendHeader("Location","/");        // Add a header to respond with a new location for the browser to go to the home page again
    webServer.send(303); 
  }

}
