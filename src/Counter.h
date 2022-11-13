#pragma once

#include "Arduino.h"
#include "Hiking-DDS238-2.h"


class Counter { 
public:
  typedef enum {
    errOk,
    errTimeout,
    errCs,
    errUnk,
  } err_t;
  typedef void (*myfunc)(char *jsonResStr, err_t err);  
private:  
  myfunc m_clb ;
  Hiking_DDS238_2 *m_counter;
  Stream *m_uartRS485;
  uint8_t m_txPin;
  char *getResultsJsonStr(char *errStr);
public:
  Counter(Stream *uartRS485, uint8_t txPin, myfunc clb);
  void polling();  
};