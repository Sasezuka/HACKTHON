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

  // SSピンがLOWになっているか確認 (必須ではありませんが、デバッグで役立つことがあります)
  if (digitalRead(SS) == LOW) { 
    byte highByte = SPI.transfer(0x00); // 上位8bitを受信 (0x00はマスターに返すダミーデータ)
    byte lowByte = SPI.transfer(0x00);  // 下位4bitを受信

    uint16_t currentReceivedValue = (uint16_t)highByte << 4; // 上位8bitを12bitデータの上位にシフト
    currentReceivedValue |= (lowByte & 0x0F);       // 下位4bitのみ結合

    // 受信した生のバイト値も表示する（デバッグ用）
    Serial.print("Raw High: ");
    Serial.print(highByte, HEX); // 16進数で表示
    Serial.print(", Raw Low: ");
    Serial.print(lowByte, HEX);  // 16進数で表示

    Serial.print(", Received Value: ");
    Serial.println(currentReceivedValue);
  } else {
    // SSがHIGHの時は、ごく短いディレイを入れることでCPU使用率を下げられます。
    // ただし、このテストスケッチでは非常に短時間しかHIGHにならないため、効果は小さいです。
    // delayMicroseconds(10); 
  }
}