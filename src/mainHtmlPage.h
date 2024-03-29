//Do not edit!
#include "Arduino.h"
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
<body onload="loadPage()">
<div style="width:800px; margin:0 auto;">
  <h3>Hiking-DDS238-2</h3>
  <table>
    <caption>
    <h4>Parameters</h4>
    </caption>
    <tr>
      <td style="width: 50%;">WiFi SSID</td>
      <td><input type="text" id="wifi_ssid" value="?">
        <button class="button" onclick="save('wifi_ssid')">Save</button></td>
    </tr>
    <tr>
      <td>WiFi Password</td>
      <td><input type="text" id="wifi_password" value="?">
        <button class="button" onclick="save('wifi_password')">Save</button></td>
    </tr>
    <tr>
      <td>MQTT server</td>
      <td><input type="text" id="mqtt_server" value="?">
        <button class="button" onclick="save('mqtt_server')">Save</button></td>
    </tr>
    <tr>
      <td>MQTT user</td>
      <td><input type="text" id="mqtt_user" value="?">
        <button class="button" onclick="save('mqtt_user')">Save</button></td>
    </tr>
    <tr>
      <td>MQTT password</td>
      <td><input type="text" id="mqtt_password" value="?">
        <button class="button" onclick="save('mqtt_password')">Save</button></td>
        </td>
    </tr>
    <tr>
      <td>MQTT channell</td>
      <td><input type="text" id="mqtt_channell" value="?">
        <button class="button" onclick="save('mqtt_channell')">Save</button></td>
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
      <td>Status</td>
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
      <td><input type="text" id="u_max" value="ND" readonly>
        <button class="button" onclick="save('u_max')">Reset</button></td>
    </tr>
    <tr>
      <td>I max</td>
      <td><input type="text" id="i_max" value="ND" readonly>
        <button class="button" onclick="save('i_max')">Reset</button></td>
    </tr>
    <tr>
      <td>RS-485 Errors</td>
      <td><input type="text" id="rs485" value="ND" readonly></td>
    </tr>
    <tr>
      <td>Working time, s</td>
      <td><input type="text" id="workingTime" value="?" readonly>
        <button class="button" onclick="reboot()">Reboot</button></td>
    </tr>
  </table>
  <br>
</div>
<a href="data">data</a> <a href="param">param</a> 
<script>

setInterval( function() { getData();}, 6000);

function loadPage() {
	getParam();
	getData();
};

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
      document.getElementById("total").innerHTML = jsonResponse.total;
	  document.getElementById("errcode").innerHTML = jsonResponse.status;

      document.getElementById("u_max").value = jsonResponse.u_max;
      document.getElementById("i_max").value = jsonResponse.i_max;
	  
	  document.getElementById("rs485").value = (jsonResponse.bad_req *100.0/jsonResponse.all_req).toFixed(2) + '%';
	  

    } else {
      // alert(this.status);
    }
  }
};

function getParam() {
  // alert("I am an alert box!");
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "param", true);
  xhttp.send();
  xhttp.onload = function() {
    if (this.status == 200) {
      // alert(this.responseText);
      var jsonResponse = JSON.parse(this.responseText);      
      if(jsonResponse.wifi_ssid != undefined) document.getElementById("wifi_ssid").value = jsonResponse.wifi_ssid;
      if(jsonResponse.wifi_password != undefined) document.getElementById("wifi_password").value = jsonResponse.wifi_password;
      if(jsonResponse.mqtt_server != undefined) document.getElementById("mqtt_server").value = jsonResponse.mqtt_server;      
      if(jsonResponse.mqtt_user != undefined) document.getElementById("mqtt_user").value = jsonResponse.mqtt_user;
      if(jsonResponse.mqtt_password != undefined) document.getElementById("mqtt_password").value = jsonResponse.mqtt_password;
	  if(jsonResponse.mqtt_channell != undefined) document.getElementById("mqtt_channell").value = jsonResponse.mqtt_channell;
	  if(jsonResponse.reboot != undefined) document.getElementById("workingTime").value = "Rebooting...";

    } else {
      // alert(this.status);
    }
  }
};

function save(param) {
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "param?" + param + "=" + document.getElementById(param).value, true);
  xhttp.send();  
  xhttp.onload = function() {
    if (this.status == 200) { 
      var jsonResponse = JSON.parse(this.responseText);     
      document.getElementById(param).value =  jsonResponse.wifi_ssid;
    }
  };  
};

function reboot() {
  var xhttp = new XMLHttpRequest();
  xhttp.open("GET", "param?reboot=1", true);
  xhttp.send();  
  xhttp.onload = function() {
    if (this.status == 200) { 
      //var jsonResponse = JSON.parse(this.responseText);     
      //document.getElementById(param).value =  jsonResponse.wifi_ssid;
    }
  };  
};

</script>
</body>
</html>

)=====";
