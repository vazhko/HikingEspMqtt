#include "Arduino.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h> 
#include <SoftwareSerial.h>
//#include <Preferences.h>
//#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
#include "Hiking-DDS238-2.h"
#include "webSrv.h"
#include <PubSubClient.h>


#define COUNTER_ID 1

#define MYPORT_TX 12
#define MYPORT_RX 14
#define RS485_TX  2
SoftwareSerial mySerial;

 
const char* ssid = "Keenetic-2568";
const char* password = "T9TW8iHR"; 
const char* adr_mqtt_server = "192.168.1.105";



//ESP8266WebServer webServer(80);
WiFiClient espClient;
PubSubClient client(espClient);
Hiking_DDS238_2 counter(&mySerial, 1000);




void hikingPolling();
void mqtt_reconnect();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void counter_callback(Hiking_DDS238_2::results_t* res);

 
void setup() {
  Serial.begin(115200);
  Serial.print("Start...");
                                                       

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

  webSrv::init();

  client.setServer(adr_mqtt_server, 1883);
  client.setCallback(mqtt_callback); 
                                 
}


 
void loop(){  
  webSrv::handle(); 
  hikingPolling();
  if (WiFi.status()  != WL_CONNECTED) {
    Serial.println("\nWiFi not connected, reboot.");
  } 
  if (!client.connected()) {
    mqtt_reconnect();
  }
  client.loop();

}




/******************************************************************************************/
void hikingPolling(){  
  typedef enum {
    init, idle, wait_for_poll, wait_for_resp,     
  } state_t;
  static state_t state = init;
  static  unsigned long time;

  
  switch(state){
    default:
    case init: 
      time =  millis();
      state = wait_for_poll;
      //Serial.println("Init");
    break;
    case wait_for_poll: 
      if ((millis() - time) < 2000) break;
      Serial.println("Hiking: Acking...");
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
         Serial.println("\nResult:");
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
        
        client.publish("esp/hiking/power", str);

        state = init;     
      } else if (st == Hiking_DDS238_2::stTimeout){
        Serial.println("Hiking: timeout"); 
        client.publish("esp/hiking/power", "timeout");
        state = init;
        break;            
      }else if (st == Hiking_DDS238_2::stErrCs){
        Serial.println("Hiking: crc error"); 
        client.publish("esp/hiking/power", "crc error");
        state = init;
        break;            
      } else {
        Serial.println("Hiking: some error"); 
        client.publish("esp/hiking/power", "error");
        state = init;
        break; 
      }
           
    break;    
  }
}

/******************************************************************************************/
void mqtt_reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect("arduinoClient", "vlad", "vv")) {
      Serial.println("mqtt connected");
      // Once connected, publish an announcement...
      //client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  /*
  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
  */

}
