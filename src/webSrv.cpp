#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include	"Hiking-DDS238-2.h"

extern Hiking_DDS238_2 counter;


namespace webSrv {

  void handleRootPath();
  ESP8266WebServer webServer(80);

  void init(){
    webServer.on("/", handleRootPath);
    webServer.onNotFound([](){   webServer.send(404, "text / plain", "Page Not Found\n\n");  });
    webServer.begin(); 
  }

  void handle(){
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
    if (counter.getResults().status == Hiking_DDS238_2::statusOk){
      sprintf(str, "<tr><td>U=%3.1fV</tr></td>", counter.getCounterU());        
      message += str;
      sprintf(str, "<tr><td>I=%3.1fA</tr></td>", counter.getCounterI());        
      message += str;          
      sprintf(str, "<tr><td>PF=%1.3f</tr></td>", counter.getCounterPF());        
      message += str;          
      sprintf(str, "<tr><td>P=%5.1fW</tr></td>", counter.getCounterP());        
      message += str;
    } else {
      message += "<tr><td>RS845: No response</tr></td>";
    }
    
    message += "</table>";
    message += "</div>";
    message += "</body>"; 
    message += "</html>";

    webServer.send(200, "text/html", message);
  }
}