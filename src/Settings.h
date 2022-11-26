#pragma once

#include <Arduino.h>
#include <Preferences.h>

typedef struct {
  char ssid[128];
  char password[128];
  char mqttSrvAdr[128];
  char mqttChannel[128];
  char mqttUser[128];
  char mqttPassword[128];
  uint16_t pollTime;
} settings_t;



typedef struct {
  double val;
  unsigned long epochTime;
} dataRecort_t;

typedef struct {
  //dataRecort_t currentMin;
  dataRecort_t currentMax;
  //dataRecort_t voltageMin;
  dataRecort_t voltageMax;
  unsigned long reloadCnt;  
} data_t;

class Settings {   
  Preferences m_preferences;
  settings_t m_settings;
  data_t m_data;
public:
  Settings();
  settings_t &getSettings(){
    return m_settings;
  }
  const data_t &getData(){
    return m_data;
  }

  //void setSettings(settings_t &p);
  //void setData(data_t &p);
  void resetDataU(){memset(&m_data.voltageMax, 0, sizeof(dataRecort_t));};
  void resetDataI(){memset(&m_data.currentMax, 0, sizeof(dataRecort_t));};

  void setDefault();
  void resetData();
  void syncSettings();
  void syncData();
  void checkData(double u, double i);
  
};
