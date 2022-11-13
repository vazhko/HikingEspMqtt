#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include	"Hiking-DDS238-2.h"

extern Hiking_DDS238_2 counter;


namespace webSrv {

  void handleRootPath();
  ESP8266WebServer webServer(80);
  Hiking_DDS238_2::results_t m_res;

  void init() {
    webServer.on("/", handleRootPath);
    webServer.onNotFound([]() {   webServer.send(404, "text / plain", "Page Not Found\n\n");  });
    webServer.begin();
  }

  void handle() {
    webServer.handleClient();
  }

  void handleRootPath() {
    String message = "";
    message += "<!DOCTYPE html>";
    message += "<html>";
    message += "<head>";
    message += "<title>Hiking-DDS238</title>";
    message += "<meta http-equiv='refresh' content='5'>";
    //message += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    message += "</head>";
    message += "<body>";
    message += "<h1>Hello from Hiking-DDS238!</h1>";
    message += "<div>";
    message += "<table>";

    char str[80];
    if (m_res.err == Hiking_DDS238_2::errOk) {
      sprintf(str, "<tr><td>U=%3.1fV</tr></td>", m_res.u);
      message += str;
      sprintf(str, "<tr><td>I=%3.1fA</tr></td>", m_res.i);
      message += str;
      sprintf(str, "<tr><td>PF=%1.3f</tr></td>", m_res.pf);
      message += str;
      sprintf(str, "<tr><td>P=%5.1fW</tr></td>", m_res.p);
      message += str;
    }
    else {
      sprintf(str, "<tr><td>Error=%dV</tr></td>", m_res.err);
      message += str;
    }


    message += "</table>";
    message += "</div>";
    message += "</body>";
    message += "</html>";

    webServer.send(200, "text/html", message);
  }

  void setInfo(Hiking_DDS238_2::results_t& res) {
    m_res = res;
  }
}