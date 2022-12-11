#include "Settings.h"
#include "Arduino.h"

const char* ssidDef = "Keenetic-2568";
const char* passwordDef = "6xDWi7@q";
const char* mqttSrvAdrDef = "192.168.1.105";
const char* mqttChannelDef = "esp/hiking/all";
const char* mqttUser = "root";
const char* mqttPassword = "vv";

const char* settingsStr = "settings";
const char* dataStr = "data";
const char* namespaceStr = "my-app";

Settings::Settings() {
  pinMode(D1, INPUT);
  delay(5);

  m_preferences.begin(namespaceStr);
  m_manualServiceMode = 0;
  // reset
  if (isServiceMode()) {
    setDefault();
    resetData();
    m_manualServiceMode = 1;
  }

  if (!m_preferences.isKey(settingsStr)) {
    setDefault();
    // Serial.print("\nCreate \"my-app\" namespace\n");
  }
  if (!m_preferences.isKey(dataStr)) {
    resetData();
    // Serial.print("\nCreate \"my-app\" namespace\n");
  }
  m_preferences.getBytes(settingsStr, &m_settings, sizeof(m_settings));
  m_preferences.getBytes(dataStr, &m_data, sizeof(m_data));
};

void Settings::syncSettings() {
  m_preferences.putBytes(settingsStr, &m_settings, sizeof(m_settings));
}

void Settings::syncData() {
  m_preferences.putBytes(dataStr, &m_data, sizeof(m_data));
}

void Settings::setDefault() {
  Serial.print("\n!Reset settings!\n");

  strcpy(m_settings.ssid, ssidDef);
  strcpy(m_settings.password, passwordDef);
  strcpy(m_settings.ssid, ssidDef);
  strcpy(m_settings.password, passwordDef);

  strcpy(m_settings.mqttSrvAdr, mqttSrvAdrDef);
  strcpy(m_settings.mqttChannel, mqttChannelDef);
  strcpy(m_settings.mqttUser, mqttUser);
  strcpy(m_settings.mqttPassword, mqttPassword);

  m_preferences.putBytes(settingsStr, &m_settings, sizeof(m_settings));
}

void Settings::resetData() {
  m_data.currentMax.val = 0;
  m_data.voltageMax.val = 0;
  m_data.reloadCnt = 0;
  m_preferences.putBytes(dataStr, &m_data, sizeof(m_data));
}

void Settings::checkData(double u, double i) {
  if (m_data.currentMax.val < i) {
    m_data.currentMax.val = i;
    syncData();
  }
  if (m_data.voltageMax.val < u) {
    m_data.voltageMax.val = u;
    syncData();
  }
}

bool Settings::isServiceMode() {
  return m_manualServiceMode || (0 == digitalRead(D1));
}

void Settings::setServiceMode(bool s) {
   m_manualServiceMode = s;  
}
