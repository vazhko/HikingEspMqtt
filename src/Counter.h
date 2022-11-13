#pragma once

#include "Arduino.h"
#include "Hiking-DDS238-2.h"


class Counter { 
public:

  typedef void (*myfunc)(Hiking_DDS238_2 ::results_t);  
private:  
  myfunc m_clb ;
  Hiking_DDS238_2 *m_counter;
  Stream *m_uartRS485;
  uint8_t m_txPin;  
public:
  Counter(Stream *uartRS485, uint8_t txPin, myfunc clb);
  void polling();  
};