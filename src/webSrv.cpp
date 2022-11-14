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
    message += "<style>table, th, td { border: 1px solid black; border-collapse: collapse;}th, td { padding: 15px;}</style>";
    message += "<title>Hiking-DDS238</title>";
    message += "<meta http-equiv='refresh' content='5'>"; // авто перезавантаження через 5 сек
    //message += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    message += "</head>";
    message += "<body>";
    message += "<h1>Hello from Hiking-DDS238!</h1>";
    message += "<div>";
    message += "<table>";

    char str[200];
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
    message += "</div>";
    message += "</body>";
    message += "</html>";

    webServer.send(200, "text/html", message);
  }

  void setInfo(Hiking_DDS238_2::results_t& res) {
    m_res = res;
  }
}
