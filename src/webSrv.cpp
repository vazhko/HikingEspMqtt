#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
//#include <NTPClient.h>
//#include <ezTime.h>

#include "Hiking-DDS238-2.h"
#include "Settings.h"

const char mainPage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<style>
table {
	width: 650px;
	text-align: left;
}
table, th, td {
	border: 1px solid black;
	border-collapse: collapse;
}
th, td {
	padding: 2px;
}
caption {
	text-align: left;
}
</style>
<title>Hiking-DDS238</title>
</head>
<body onload="getParam()">
<div style="width:800px; margin:0 auto;">
  <h3>Hiking-DDS238-2</h3>
  <table>
    <caption>
    <h4>Parameters</h4>
    </caption>
    <tr>
      <td style="width: 50%;">WiFi SSID</td>
      <td>
        <input type="text" id="wifi_ssid" value="?"> 
        <button class="button" onclick="save()">Save</button></td>
    </tr>
    <tr>
      <td>WiFi Password</td>
      <td><form action="SaveParam" method="get">
          <input value="0" name="Password" type="password">
          <button type="submit">Save</button>
        </form></td>
    </tr>
    <tr>
      <td>MQTT server</td>
      <td><form action="SaveParam" method="get">
          <input value="0" name="MQTT server">
          <button type="submit">Save</button>
        </form></td>
    </tr>
    <tr>
      <td>MQTT channell</td>
      <td><form action="SaveParam" method="get">
          <input value="0" name="MQTT channell">
          <button type="submit">Save</button>
        </form></td>
    </tr>
    <tr>
      <td>MQTT user</td>
      <td><form action="/SaveParam" method="get">
          <input value="0" name="MQTT user">
          <button type="submit">Save</button>
        </form></td>
    </tr>
    <tr>
      <td>MQTT password</td>
      <td><form action="SaveParam" method="get">
          <input value="0" name="MQTT password">
          <button 
type="submit">Save</button>
        </form></td>
    </tr>
  </table>
  <br>
  <table>
    <caption >
    <h4>Hiking DDS238</h4>
    </caption>
    <tr>
      <td style="width: 50%;">U</td>
      <td><span id="voltage">ND</span></td>
    </tr>
    <tr>
      <td>I</td>
      <td><span id="current">ND</span></td>
    </tr>
    <tr>
      <td>PF</td>
      <td><span id="pf">ND</span></td>
    </tr>
    <tr>
      <td>P</td>
      <td><span id="power">ND</span></td>
    </tr>
    <tr>
      <td>F</td>
      <td><span id="f">ND</span></td>
    </tr>
    <tr>
      <td>Total</td>
      <td><span id="total">ND</span></td>
    </tr>
    <tr>
      <td>Error code</td>
      <td><span id="errcode">ND</span></td>
    </tr>
  </table>
  <br>
  <table>
    <caption>
    <h4>Extreems</h4>
    </caption>
    <tr>
      <td style="width: 50%;">U max</td>
      <td><form action="SaveParam" method="get">
          <input value="0" name="U max">
          <button type="submit">Reset </button>
        </form></td>
    </tr>
    <tr>
      <td>I max</td>
      <td><form action="SaveParam" method="get">
          <input value="0" name="I max">
          <button type="submit">Reset </button>
        </form></td>
    </tr>
    <tr>
      <td>Working time, s</td>
      <td><form action="SaveParam" method="get">
          <input value="0" name="Reboot" readonly>
          <button type="submit">Reboot</button>
        </form></td>
    </tr>
  </table>
  <br>
</div>
<a href="data">data</a> <a href="param">param</a> 
<script>

setInterval( function() { getData(); getParam();}, 10000);

function getData() {
  // alert("I am an alert box!");
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "data", true);
  xhttp.send();
  xhttp.onload = function() {
    if (this.status == 200) {
      // alert(this.responseText);
      var jsonResponse = JSON.parse(this.responseText);
      document.getElementById("voltage").innerHTML = jsonResponse.voltage;
      document.getElementById("current").innerHTML = jsonResponse.current;
      document.getElementById("power").innerHTML = jsonResponse.power;
      document.getElementById("pf").innerHTML = jsonResponse.pf;
      document.getElementById("f").innerHTML = jsonResponse.f;
      document.getElementById("errcode").innerHTML = jsonResponse.status;
      document.getElementById("total").innerHTML = jsonResponse.total;

    } else {
      // alert(this.status);
    }
  }
}

function getParam() {
  // alert("I am an alert box!");
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "param", true);
  xhttp.send();
  xhttp.onload = function() {
    if (this.status == 200) {
      // alert(this.responseText);
      var jsonResponse = JSON.parse(this.responseText);
      //document.getElementById("wifi_ssid").innerHTML = jsonResponse.wifi_ssid;
      document.getElementById("wifi_ssid").setAttribute("value",jsonResponse.wifi_ssid);
      document.getElementById("wifi_password").innerHTML = jsonResponse.wifi_password;
      document.getElementById("mqtt_server").innerHTML = jsonResponse.mqtt_server;
      document.getElementById("mqtt_channell").innerHTML = jsonResponse.mqtt_channell;
      document.getElementById("mqtt_user").innerHTML = jsonResponse.mqtt_user;
      document.getElementById("mqtt_password").innerHTML = jsonResponse.mqtt_password;

    } else {
      // alert(this.status);
    }
  }
}

function save() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("state").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "save?state=", true);
  xhttp.send();
}
</script>
</body>
</html>
)=====";

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
  // webServer.on("/", handleRootPath);
  webServer.on("/", handleIndex);
  webServer.on("/data", handleGetData);
  webServer.on("/SaveParam", handleSaveParam);
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
    sprintf(settings.getSettings().mqttSrvAdr, webServer.arg(0).c_str());
    settings.syncSettings();
  }
  if (webServer.argName(0) == "MQTT channell") {
    sprintf(settings.getSettings().mqttChannel, webServer.arg(0).c_str());
    settings.syncSettings();
  }
  if (webServer.argName(0) == "MQTT user") {
    sprintf(settings.getSettings().mqttUser, webServer.arg(0).c_str());
    settings.syncSettings();
  }
  if (webServer.argName(0) == "MQTT password") {
    sprintf(settings.getSettings().mqttPassword, webServer.arg(0).c_str());
    settings.syncSettings();
  }
  if (webServer.argName(0) == "U max") {
    settings.resetDataU();
  }
  if (webServer.argName(0) == "I max") {
    settings.resetDataI();
  }
  if (webServer.argName(0) == "Reboot") {
    if (digitalRead(D1)) {
      Serial.print(" .Pin D1 is 1");
    } else {
      Serial.print(" .Pin D1 is 0");
    }
    reboot();
  }
  webServer.sendHeader("Location", "/");  // Add a header to respond with a new location for the browser to go to the home page again
  webServer.send(303);
}

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
      "\"mqtt_channell\":\"%s\","
      "\"mqtt_user\":\"%s\","
      "\"mqtt_password\":\"%s\""
      "}";

  char str[1000];
  sprintf(str, strFormat,
          settings.getSettings().ssid,
          settings.getSettings().password, 
          settings.getSettings().mqttSrvAdr,
          settings.getSettings().mqttChannel, 
          settings.getSettings().mqttUser, 
          settings.getSettings().mqttPassword
  );
  webServer.send(200, "text/html", str);
}

}  // namespace webSrv