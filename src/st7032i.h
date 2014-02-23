
/**
 * LCDを初期化する。
 * @param slave_addr LCDのI2Cアドレス
 * @param wait ミリ秒単位のwait関数へのポインタ
 */
void st7032i_init(uint8_t slave_addr, void (*wait)(uint8_t ms));

/**
 * LCDに制御コマンドを送信する。
 * @param slave_addr LCDのI2Cアドレス
 * @param command LCDに送るコマンド
 */
int8_t st7032i_send_command(uint8_t slave_addr, uint8_t command);

/**
 * LCDにデータを送信する。
 * @param slave_addr LCDのI2Cアドレス
 * @param data LCDに送るデータ
 */
int8_t st7032i_send_data(uint8_t slave_addr, uint8_t data);

/**
 * LCD表示をクリアする
 * @param slave_addr LCDのI2Cアドレス
 */
void st7023i_clear_display(uint8_t slave_addr);

/**
 * LCDの指定座標位置から文字列を表示する。
 * @param slave_addr LCDのI2Cアドレス
 * @param x X座標(0から7)
 * @param y Y座標(0から1)
 * @param str 表示文字列
 */
void st7032i_display(uint8_t slave_addr, uint8_t x, uint8_t y, char *str);
