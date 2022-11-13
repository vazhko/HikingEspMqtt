
#include "Counter.h"
#define COUNTER_ID 1

/******************************************************************************************/
Counter::Counter(Stream* uartRS485, uint8_t txPin, myfunc clb) : m_clb(clb) {
  m_uartRS485 = uartRS485;
  m_counter = new Hiking_DDS238_2(m_uartRS485, 1000);
  m_txPin = txPin;
}

/******************************************************************************************/
void Counter::polling() {
  typedef enum {
    init_st, idle_st, wait_for_poll_st, wait_for_resp_st,
  } state_t;

  static state_t state = init_st;
  static  unsigned long time;

  switch (state) {
  default:
  case init_st:
    time = millis();
    state = wait_for_poll_st;
    pinMode(m_txPin, OUTPUT);
    digitalWrite(m_txPin, HIGH);
    break;
  case wait_for_poll_st:
    if ((millis() - time) < 2000) break;
    Serial.println("Hiking: Acking...");
    //digitalWrite(m_txPin, HIGH);
    //delay(5);
    m_counter->regReq(COUNTER_ID);
    delay(2);
    digitalWrite(m_txPin, LOW);
    state = wait_for_resp_st;
    time = millis();
    break;
  case wait_for_resp_st:
    Hiking_DDS238_2::reqStatus_t st = m_counter->getReqStatus();
    if (st == Hiking_DDS238_2::stWait_for_resp) break;
    if (st == Hiking_DDS238_2::stDone) {
      //for(uint8_t i = 0; i < m_counter->getRxLength(); i ++) {    
        //Serial.print(counter.getRxBuff()[i], HEX); 
      //}        
      if (m_clb) m_clb(m_counter->getResults());
      state = init_st;
    }
    else if (st == Hiking_DDS238_2::stTimeout) {
      if (m_clb) m_clb(m_counter->getResults());
      state = init_st;
      break;
    }
    else if (st == Hiking_DDS238_2::stErrCs) {
      if (m_clb) m_clb(m_counter->getResults());
      state = init_st;
      break;
    }
    else if (st == Hiking_DDS238_2::stErr) {
      if (m_clb) m_clb(m_counter->getResults());
      state = init_st;
      break;
    }
    else {
      state = init_st;
      break;
    }

    break;
  }
}

