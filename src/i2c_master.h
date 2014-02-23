/*
 * i2c_master.h
 */

#ifndef I2C_MASTER_H_
#define I2C_MASTER_H_

static const int8_t I2C_MASTER_SUCCESS = 0;
static const int8_t I2C_MASTER_NO_ACK = 1;
static const int8_t I2C_MASTER_FAIL_WITH_ARBITRATION = -1;
static const int8_t I2C_MASTER_ERROR = -2;

/**
 * I2Cマスタとして初期化する。
 * @param clock I2Cの動作クロック(Hz)
 */
void I2CMasterInit(uint32_t clock);

/**
 * I2Cマスタからスレーブにデータを送信する。
 * @param slave_addr 送信先のスレーブアドレス
 * @param data 送信するデータ
 * @param length 送信するデータの長さ
 */
int8_t I2CMasterTX(uint8_t slave_addr, const uint8_t *data, uint8_t length);

/**
 * I2Cマスタにスレーブからデータを受信する。
 * @param slave_addr 受信するスレーブアドレス
 * @param data 受信したデータを格納するバッファ
 * @param length 受信サイズ
 */
int8_t I2CMasterRx(uint8_t slave_addr, uint8_t *data, uint8_t length);

#endif /* I2C_MASTER_H_ */
