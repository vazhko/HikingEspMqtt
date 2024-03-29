#pragma once

#include "Arduino.h"

#define REQ_REG_COUNT 22
#define REG_ADR_BEGIN 0
#define RESP_LENGTH REQ_REG_COUNT * 2 + 5

class Hiking_DDS238_2 {
public:
  typedef enum {
    errOk,
    errTimeout,
    errCs,
    errUnk,
  } err_t;

  typedef struct {
    double u;
    double i;
    double p;
    double pf;
    double f;
    uint32_t totalCnt;
    err_t err;
    unsigned long errCnt;
    unsigned long noErrCnt;
  } results_t;

  typedef enum {
    stReady,
    stWait_for_resp,
    stDone,
    stTimeout,
    stErrCs,
    stErr,
  } reqStatus_t;

private:
  Stream* m_uart;
  unsigned long m_timeout;
  unsigned long m_time;
  results_t m_results;
  uint16_t m_rxCount;
  uint16_t mb_crc_add(uint8_t* buffer, uint8_t count);
  uint8_t mb_crc_calc(uint16_t* code, uint8_t next);
  uint8_t m_rxBuff[RESP_LENGTH];
  uint16_t regReq(uint8_t id, uint16_t adr, uint16_t count);
  uint8_t mb_crc_check(uint8_t*, uint8_t);
  int16_t getRegVal(int16_t regNum);
  int32_t getRegVal32(int16_t regNum);
  void extractResults();

public:
  Hiking_DDS238_2(Stream* uart, unsigned long timeout)
    : m_uart(uart), m_timeout(timeout), m_rxCount(0) {
    memset(&m_results, 0, sizeof(m_results));
  };
  uint16_t regReq(uint8_t id) {
    return regReq(id, REG_ADR_BEGIN, REQ_REG_COUNT);
  };
  reqStatus_t getReqStatus();
  uint8_t* getRxBuff() { return m_rxBuff; };
  uint16_t getRxLength() { return RESP_LENGTH; };

  double getCounterU();
  double getCounterI();
  double getCounterP();
  double getCounterPF();
  double getCounterF();
  uint32_t getCounterTotal();
  char* getResultsJsonStr();
  results_t& getResults() {
    return m_results;
  }
  static const char *statusToStr(err_t e);
};
