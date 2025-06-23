// --- Arduino3_SPI_Slave_Test_Polling.ino ---

#include <SPI.h>

volatile uint16_t receivedValue = 0;
volatile byte byteCount = 0; // 受信したバイトのカウント (0: 1バイト目, 1: 2バイト目)
volatile bool newDataAvailable = false; // 新しいデータが受信されたことを示すフラグ

void setup() {
  Serial.begin(115200);
  Serial.println("Arduino 3 (Slave) SPI Test Started! (Polling Mode)");

  // SSピンをスレーブセレクト入力として設定 (INPUT_PULLUPが推奨)
  // これにより、マスターがSSピンをLOWにすると、SPIモジュールが有効になる
  pinMode(SS, INPUT_PULLUP);
  
  // SPIをスレーブモードで初期化
  SPI.begin();
  
  // R4シリーズのスレーブモードでは、SPI.transfer() がマスターからのデータ到着を待つ（ブロックする）
  // 割り込みは設定しない
  Serial.println("SPI Slave Initialized (No Interrupts for Test).");
}

void loop() {
  // マスターがSSピンをLOWにしている間だけ、SPI通信が行われる
  // SPI.transfer()はマスターからのデータを受信するまでブロックする
  // Arduino 2が2バイト送信するので、2回SPI.transfer()を呼び出す
  
  // SSピンの状態を確認（スレーブが選択されているか）
  if (digitalRead(SS) == LOW) { // SSピンがLOWのときだけ処理
    byte highByte = SPI.transfer(0x00); // 上位8bitを受信
    byte lowByte = SPI.transfer(0x00);  // 下位4bitを受信

    uint16_t currentReceivedValue = (uint16_t)highByte << 4; // 上位8bitを12bitデータの上位にシフト
    currentReceivedValue |= (lowByte & 0x0F);       // 下位4bitのみ結合

    // 受信した生のバイト値も表示してみる
    Serial.print("Raw High: ");
    Serial.print(highByte, HEX); // 16進数で表示
    Serial.print(", Raw Low: ");
    Serial.print(lowByte, HEX);  // 16進数で表示

    Serial.print(", Received Value: ");
    Serial.println(currentReceivedValue);

    // 今回はデバッグのため、newDataAvailableフラグは使わない
    // serial.printの負荷は高いので、普段は使うべきではありません。
  } else {
    // SSピンがHIGHのときは何も受信しないはず
    // Serial.println("SS is HIGH. Waiting for master...");
    delay(10); // 無限ループにならないように少し待つ
  }
}