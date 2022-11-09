#include "Arduino.h"
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h> 
#include <SoftwareSerial.h>
//#include <Preferences.h>
//#include <ESP8266WebServer.h>
//#include <ESP8266mDNS.h>
#include "Counter.h"
#include "webSrv.h"
#include <PubSubClient.h>




#define MYPORT_TX 12
#define MYPORT_RX 14
#define RS485_TX  2
SoftwareSerial mySerial;

 
const char* ssid = "Keenetic-2568";
const char* password = "T9TW8iHR"; 
const char* adr_mqtt_server = "192.168.1.105";
const char* mqtt_payload = "esp/hiking/all";


WiFiClient espClient;
PubSubClient client(espClient);


void hikingPolling();
void mqtt_reconnect();
void mqtt_callback(char* topic, byte* payload, unsigned int length);

void counter_callback(char* res, bool err);
Counter counter(&mySerial,  RS485_TX, counter_callback);

 
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

  webSrv::init();

  client.setServer(adr_mqtt_server, 1883);
  client.setCallback(mqtt_callback); 
  
                                 
}


 
void loop(){  
  webSrv::handle(); 
  counter.polling();
  if (WiFi.status()  != WL_CONNECTED) {
    Serial.println("\nWiFi not connected, reboot.");
  } 
  if (!client.connected()) {
    mqtt_reconnect();
  }
  client.loop();

}




/******************************************************************************************/
void counter_callback(char* res, bool err){
  if(err) {
    Serial.println("Hiking: err");
    return;
  }
  Serial.print("Hiking: ");
  Serial.println(res);
  client.publish(mqtt_payload, res);
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
    if (client.connect("arduinoClient", "root", "vv")) {
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
