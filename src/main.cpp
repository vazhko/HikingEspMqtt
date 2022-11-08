#include "Arduino.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h> 
//#include <Wire.h>
//#include "SparkFunHTU21D.h"
//#include <OneWire.h>
//#include <DallasTemperature.h>
#include <SoftwareSerial.h>
//#include <Preferences.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "Hiking-DDS238-2.h"

#define COUNTER_ID 1

#define MYPORT_TX 12
#define MYPORT_RX 14
#define RS485_TX  2
SoftwareSerial mySerial;

//HTU21D myHumidity;  
//Preferences prefs;
 
const char* ssid = "Keenetic-2568";
const char* password = "T9TW8iHR"; 

//WiFiServer mbServer(333);
ESP8266WebServer webServer(80);

//const int oneWireBus = 13;
//OneWire oneWire(oneWireBus);
//DallasTemperature sensors(&oneWire);

const long utcOffsetInSeconds = 3*3600;//+3
WiFiUDP ntpUDP; // Define NTP Client to get time
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

Hiking_DDS238_2 counter(&mySerial, 1000);


void handleRootPath();
void hikingPolling();
void handleNotFound();

 
void setup() {
  Serial.begin(115200);
  Serial.print("Start...");
                                                       
  //myHumidity.begin();
  //sensors.begin();
  timeClient.begin();
  //prefs.begin("my-app");
  //char counter = prefs.getChar("counter", 1); 
  //Serial.print("Counter is ");
  //Serial.println(counter, HEX);   


  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print("."); 
  }
 
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());


  mySerial.begin(9600, SWSERIAL_8N1, MYPORT_RX, MYPORT_TX, false);  
  pinMode (RS485_TX, OUTPUT); 
  digitalWrite (RS485_TX, LOW);

  //mbServer.begin();
  
  webServer.on("/index.html", handleRootPath);
  webServer.on("/other.html", []() {   //Определите функцию обработки для пути
    webServer.send(200, "text / plain", "Additional page");
  });
  webServer.onNotFound(handleNotFound);  
  webServer.begin();  
                                 
}


 
void loop(){ 
  //mbServerTask(); 
  webServer.handleClient(); 
  hikingPolling();        
}
/*
void mbServerTask(){
  static uint8_t connected = 0;
  mySerial.listen();
  WiFiClient client = mbServer.available();

  if (client) {
    
    if(client.connected()) {
      if(!connected) {
        Serial.println("\nclient.connected");
      } 
      connected = 1;    
    }  else {
      if(connected) {           
        Serial.println("\nclient.disconnected");
      } 
      client.stop(); 
      connected = 0;
    }
    
    while (client.connected()) {
            
      if(client.available()) {
        size_t len = client.available();
        uint8_t sbuf[len];
        client.read(sbuf, len);               
        digitalWrite (RS485_TX, HIGH);
        delay(5);    
        mySerial.write(sbuf, len);
        delay(5);
        digitalWrite (RS485_TX, LOW);
        //Serial.writeBytes(sbuf, len);
        Serial.print("TCP: ");
        Serial.println(len);
      }

      int cnt = 0;
      while (mySerial.available()) {
        char c = mySerial.read();
        client.write(c);
        cnt ++;
        delay(7);
      }
      if(cnt)  {
        Serial.print("485: ");
        Serial.println(cnt);      
      } 
  
    }    
    //client.stop();    
    //Serial.println("\nclient.disconnected");
  }   
}
*/ 


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
  sprintf(str, "<tr><td>U=%3.1fV</tr></td>", counter.getCounterU());        
  message += str;
  sprintf(str, "<tr><td>I=%3.1fA</tr></td>", counter.getCounterI());        
  message += str;          
  sprintf(str, "<tr><td>PF=%1.3f</tr></td>", counter.getCounterPF());        
  message += str;          
  sprintf(str, "<tr><td>P=%5.1fW</tr></td>", counter.getCounterP());        
  message += str;
  
  message += "</table>";
  message += "</div>";
  message += "</body>"; 
  message += "</html>";

  webServer.send(200, "text/html", message);
}

void handleNotFound() {
  
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += webServer.uri();
  message += "\nMethod: ";
  message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webServer.args();
  message += "\n";
  for (uint8_t i = 0; i < webServer.args(); i++) {
    message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
  }
  webServer.send(404, "text/plain", message);  
}



void hikingPolling(){  
  typedef enum {
    init, idle, wait_for_poll, wait_for_resp,     
  } state_t;
  static state_t state = init;
  static  unsigned long time;
  //uint8_t txBuf[100], rxBuf[100];
  //uint16_t txCount, rxCount;
  
  switch(state){
    default:
    case init: 
      time =  millis();
      state = wait_for_poll;
      //Serial.println("Init");
    break;
    case wait_for_poll: 
      if ((millis() - time) < 2000) break;
      Serial.println("Ack");
      digitalWrite (RS485_TX, HIGH);
      delay(5);
      counter.regReq(COUNTER_ID);
      delay(5);      
      digitalWrite (RS485_TX, LOW);      
      state = wait_for_resp;
      time =  millis();   
    break;
    case wait_for_resp:
      Hiking_DDS238_2::reqStatus_t st = counter.getReqStatus();      
      if(st == Hiking_DDS238_2::stWait_for_resp) break;
      if (st == Hiking_DDS238_2::stDone){
         Serial.println("\nin:");
        for(uint8_t i = 0; i < counter.getRxLength(); i ++) {    
          //Serial.print(counter.getRxBuff()[i], HEX); 
        }
        char str[80];
        sprintf(str, "U=%3.1f", counter.getCounterU());        
        Serial.println(str);

        sprintf(str, "I=%3.1f", counter.getCounterI());        
        Serial.println(str); 
               
        sprintf(str, "PF=%1.3f", counter.getCounterPF());        
        Serial.println(str);
               
        sprintf(str, "P=%5.1f", counter.getCounterP());        
        Serial.println(str);
        
     
        state = init;     
      } else if (st == Hiking_DDS238_2::stTimeout){
        Serial.println("timeout"); 
        state = init;
        break;            
      }else if (st == Hiking_DDS238_2::stErrCs){
        Serial.println("crc error"); 
        state = init;
        break;            
      } else {
        Serial.println("some error"); 
        state = init;
        break; 
      }
           
    break;    
  }

  

}



