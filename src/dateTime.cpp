#include "dateTime.h"
#include <ESP8266WiFi.h>
#include "Arduino.h"
#include <ezTime.h>
namespace dateTime {

void init() {

  waitForSync();

  Serial.println();
  Serial.println("UTC:             " + UTC.dateTime());
  Timezone myTZ;
  // Provide official timezone names
  // https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
  myTZ.setLocation(F("europe/kiev"));
  Serial.print(F("My loc:     "));
  Serial.println(myTZ.dateTime());
  // Wait a little bit to not trigger DDoS protection on server
  // See https://github.com/ropg/ezTime#timezonedropnl
  delay(5000);
  // Or country codes for countries that do not span multiple timezones
  myTZ.setLocation(F("uk"));
  Serial.print(F("Ukraine:         "));
  Serial.println(myTZ.dateTime());

}

void handle() {
  events();
}

}  // namespace dateTime
