/*
 * i2c_master.c
 */

#include "LPC13Uxx.h"
#include "i2c_master.h"

static const uint8_t AA_BIT = (1 << 2);
static const uint8_t SI_BIT = (1 << 3);
static const uint8_t STO_BIT = (1 << 4);
static const uint8_t STA_BIT = (1 << 5);
static const uint8_t I2EN_BIT = (1 << 6);
#define FALSE 0
#define TRUE 1

void I2CMasterInit(uint32_t clock) {
  // Setup peripheral clock(I2C 5, GPIO 6, IOCON 16)
  LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 5) | (1 << 6) | (1 << 16);

  // Setup GPIO P0_4 (FUNC=I2C SCL(1), I2CMODE=Standard mode/Fast mode I2C(0))
  LPC_IOCON->PIO0_4 = 0x00000001;
  // Setup GPIO P0_5 (FUNC=I2C SDA(1), I2CMODE=Standard mode/Fast mode I2C(0))
  LPC_IOCON->PIO0_5 = 0x00000001;

  // Reset I2C Peripheral
  LPC_SYSCON->PRESETCTRL &= ~(1 << 1);
  LPC_SYSCON->PRESETCTRL |= (1 << 1);

  // Setup I2C master clock speed.
  uint32_t width = SystemCoreClock / clock / 2;
  LPC_I2C->SCLH = width;
  LPC_I2C->SCLL = width;
}
static inline uint8_t waitSi(void) {
  while (1) {
    if (LPC_I2C->CONSET & SI_BIT) {
      uint8_t status = LPC_I2C->STAT;
      if (status != 0xF8) {
        return status;
      }
    }
  }
}
int8_t I2CMasterRx(uint8_t slave_addr, uint8_t *data, uint8_t length) {
  uint8_t status;
  // clear flags
  LPC_I2C->CONCLR = AA_BIT | STO_BIT | STA_BIT | SI_BIT;
  // startup master mode and send start bit
  LPC_I2C->CONSET = STA_BIT | I2EN_BIT;
  // wait start bit
  status = waitSi();
  // check status
  if (status != 0x08 && status != 0x10) {
    return I2C_MASTER_ERROR;
  }
  // send slave address and read bit(1)
  LPC_I2C->DAT = slave_addr | 0x01;
	// send data
  LPC_I2C->CONCLR = SI_BIT | STA_BIT;
  // wait data send
  status = waitSi();
  // check status
  if (status != 0x40) {
    // 0x40: SLA+R has been transmitted(ACK)
    if (status == 0x48) {
      // 0x48: SLA+R has been transmitted but NOT ACK -> send stop bit and return with error
      LPC_I2C->CONSET = STO_BIT;
      LPC_I2C->CONCLR = SI_BIT;
      while (LPC_I2C->CONSET & 0x10) ;
      LPC_I2C->CONCLR = STA_BIT | STO_BIT | SI_BIT;
      return I2C_MASTER_NO_ACK;
		} else if (status == 0x38) {
		  // 0x38: arbitration is lost -> abort and return with error
		  LPC_I2C->CONCLR = STA_BIT | STO_BIT | AA_BIT | SI_BIT;
		  return I2C_MASTER_FAIL_WITH_ARBITRATION;
		}
    return I2C_MASTER_ERROR;
  }
  for (uint8_t i = 0; i < length; i++) {
    if (i == (length - 1)) {
      // last byte. set NO ACK
      LPC_I2C->CONCLR = AA_BIT | SI_BIT;
    } else {
      // set ACK
      LPC_I2C->CONSET = AA_BIT;
      LPC_I2C->CONCLR = SI_BIT;
    }
    status = waitSi();
    if (status != 0x50 && status != 0x58) {
      // CHECK ERROR
    }
    data[i] = LPC_I2C->DAT;
  }
  LPC_I2C->CONSET = STO_BIT | AA_BIT;
  LPC_I2C->CONCLR = SI_BIT;
  // wait stop bit.
  while (LPC_I2C->CONSET & 0x10) ;
  // clear all status
  LPC_I2C->CONCLR = SI_BIT | STO_BIT | STA_BIT | AA_BIT | I2EN_BIT;
  // done.
  return I2C_MASTER_SUCCESS;
}

int8_t I2CMasterTX(uint8_t slave_addr, const uint8_t *data, uint8_t length) {
  uint8_t status;
  // clear flags
  LPC_I2C->CONCLR = AA_BIT | STO_BIT | STA_BIT | SI_BIT;
  // startup master mode and send start bit
  LPC_I2C->CONSET = STA_BIT | I2EN_BIT;
  // wait start bit (
  status = waitSi();
  // check status
  if (status != 0x08 && status != 0x10) {
    // error check
    return I2C_MASTER_ERROR;
  }
  // send slave address and write bit(0);
  LPC_I2C->DAT = slave_addr & 0xFE;
  // send data
  LPC_I2C->CONCLR = SI_BIT | STA_BIT;
  // wait data send
  status = waitSi();
  // check status
  if (status != 0x18) {
    // 0x18: SLA+W has been transmitted(ACK)
    if (status == 0x20) {
      // 0x20: SLA+W has not been transmitted(NOT ACK) -> send stop bit and return with error
      LPC_I2C->CONSET = STO_BIT;
      LPC_I2C->CONCLR = SI_BIT;
      while (LPC_I2C->CONSET & 0x10) ;
      LPC_I2C->CONCLR = STA_BIT | STO_BIT | SI_BIT;
      return I2C_MASTER_NO_ACK;
    } else if (status == 0x38) {
      // 0x38: arbitration is lost -> abort and return with error
      LPC_I2C->CONCLR = STA_BIT | STO_BIT | AA_BIT | SI_BIT;
      return I2C_MASTER_FAIL_WITH_ARBITRATION;
    }
    return I2C_MASTER_ERROR;
  }
  for (uint8_t i = 0; i < length; i++) {
    // set data
    LPC_I2C->DAT = *(data++);
    LPC_I2C->CONCLR = SI_BIT;
    // wait transmit data.
    status = waitSi();
    // check status
    if (status != 0x28) {
      // 0x28: data sent with ACK
      if (status == 0x30) {
        // 0x30: data sent with NO-ACK -> send stop bit and return with error
        LPC_I2C->CONSET = STO_BIT;
        LPC_I2C->CONCLR = SI_BIT;
        waitSi();
        return I2C_MASTER_NO_ACK;
      } else
        if (status == 0x38) {
        // 0x38: arbitration is lost -> abort and return with error
        LPC_I2C->CONCLR = STA_BIT | STO_BIT | AA_BIT | SI_BIT;
        return I2C_MASTER_FAIL_WITH_ARBITRATION;
      } else {
        return I2C_MASTER_ERROR;
      }
    }
  }
  // send stop bit
  LPC_I2C->CONSET = STO_BIT;
  LPC_I2C->CONCLR = SI_BIT;
  // wait stop bit.
  while (LPC_I2C->CONSET & 0x10) ;
  // clear all status
  LPC_I2C->CONCLR = SI_BIT | STO_BIT | STA_BIT | AA_BIT | I2EN_BIT;
  // done.
  return I2C_MASTER_SUCCESS;
}
