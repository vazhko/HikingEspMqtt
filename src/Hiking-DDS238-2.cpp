#include "Hiking-DDS238-2.h"
#define _DEBUG_

/******************************************************************************************/
uint8_t Hiking_DDS238_2::mb_crc_calc(uint16_t *code, uint8_t next) {
  uint8_t flag;
  uint8_t i;
  *code ^= next;
  for (i = 0; i < 8; i++) {
    flag = *code & 1;
    *code = *code >> 1;
    if (flag) *code ^= 0xA001;
  }
  return next;
}

/******************************************************************************************/
uint16_t Hiking_DDS238_2::mb_crc_add(uint8_t *buffer, uint8_t count) {
  uint16_t CRC = 0xffff;
  for (uint8_t i = 0; i < count; i++) {
    mb_crc_calc(&CRC, *buffer++);
  }
  *buffer++ = (uint8_t)CRC;
  *buffer = (uint8_t)(CRC >> 8);
  return CRC;
}

/******************************************************************************************/
uint8_t Hiking_DDS238_2::mb_crc_check(uint8_t *buffer, uint8_t cnt) {
  uint16_t crc = 0xffff;
  for (uint8_t i = 0; i < cnt - 2; i++) {
    mb_crc_calc(&crc, *buffer++);
  }
  if (((uint8_t)(crc) == *buffer++) && ((uint8_t)(crc >> 8) == *buffer)) return 1;
  return 0;
}

/******************************************************************************************/
uint16_t Hiking_DDS238_2::regReq(uint8_t id, uint16_t adr, uint16_t count) {
  uint8_t txBuff[8];
  txBuff[0] = id;
  txBuff[1] = 3;
  txBuff[2] = (uint8_t)(adr >> 8);
  txBuff[3] = (uint8_t)(adr >> 0);
  txBuff[4] = (uint8_t)(count >> 8);
  txBuff[5] = (uint8_t)(count >> 0);
  mb_crc_add(txBuff, 6);
  m_uart->write(txBuff, 8);
  m_time = millis();
  m_rxCount = count * 2 + 5;

  // memset(&m_results, 0, sizeof(m_results));
  m_results.err = errTimeout;

  return m_rxCount;
}

/******************************************************************************************/
Hiking_DDS238_2::reqStatus_t Hiking_DDS238_2::getReqStatus() {
  if (m_time == 0) return reqStatus_t::stReady;
  if ((millis() - m_time) > m_timeout) {
    m_results.errCnt++;
#ifdef _DEBUG_
    String msg="";
    char c[200];
    int cnt = m_uart->available();
    sprintf(c, "RS485 in %d bytes:", cnt);
    msg += c;
    m_uart->read(m_rxBuff, cnt);
    for (uint8_t i = 0; i < cnt; i++) {
      //m_uart->read(m_rxBuff, m_rxCount), HEX);
      
      sprintf(c, "%02X", m_rxBuff[i]);
      msg += c;
    }    
    Serial.println(msg);
#endif
    return stTimeout;
  }
  if (m_rxCount > m_uart->available()) return stWait_for_resp;
  if (m_rxCount < m_uart->available()) {
    m_uart->flush();
    m_results.err = errUnk;
    m_results.errCnt++;
    return reqStatus_t::stErr;
  }
  if (m_rxCount == m_uart->available()) {
    m_uart->read(m_rxBuff, m_rxCount);
    if (!mb_crc_check(m_rxBuff, m_rxCount)) {
      m_results.err = errCs;

      m_results.errCnt++;
      return stErrCs;
    }
  }
  extractResults();
  return stDone;
}

/******************************************************************************************/
int16_t Hiking_DDS238_2::getRegVal(int16_t regNum) {
  return (m_rxBuff[3 + regNum * 2] << 8) | (m_rxBuff[4 + regNum * 2] << 0);
}
int32_t Hiking_DDS238_2::getRegVal32(int16_t regNum) {
  return (m_rxBuff[3 + regNum * 2] << 24) | (m_rxBuff[4 + regNum * 2] << 16) | (m_rxBuff[5 + regNum * 2] << 8) | (m_rxBuff[6 + regNum * 2] << 0);
}

/******************************************************************************************/
double Hiking_DDS238_2::getCounterU() {
  return (double)getRegVal(12) / 10.0;
}
double Hiking_DDS238_2::getCounterI() {
  return (double)getRegVal(13) / 100.0;
}
double Hiking_DDS238_2::getCounterP() {
  return getCounterU() * getCounterI() * getCounterPF();
}
double Hiking_DDS238_2::getCounterPF() {
  return (double)getRegVal(16) / 1000.0;
}
double Hiking_DDS238_2::getCounterF() {
  return (double)getRegVal(17) / 100.0;
}
uint32_t Hiking_DDS238_2::getCounterTotal() {
  return getRegVal32(0);
}

/******************************************************************************************/
void Hiking_DDS238_2::extractResults() {
  m_results.err = errOk;
  m_results.noErrCnt++;
  m_results.u = getCounterU();
  m_results.i = getCounterI();
  m_results.p = getCounterP();
  m_results.pf = getCounterPF();
  m_results.f = getCounterF();
  m_results.totalCnt = getCounterTotal();
}

/******************************************************************************************/
const char *Hiking_DDS238_2::statusToStr(err_t e){
  switch(e){
    case errOk:
      return "RS-485: Ok";
    break;
    case errTimeout:
      return "RS-485: Err: timeout";
    break;   
    case errCs:
      return "RS-485: Err: CS";
    break;
    case errUnk:
      return "RS-485: Err: Unknown";
    break;  
  }
  return "";
}
