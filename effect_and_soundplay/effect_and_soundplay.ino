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
  
  // 1バイト目を受信 (上位8bit)
  // マスターがデータを送信するまでここでブロックします
  byte highByte = SPI.transfer(0x00); // 0x00はマスターに返すダミーデータ
  
  // 2バイト目を受信 (下位4bit)
  // マスターがデータを送信するまでここでブロックします
  byte lowByte = SPI.transfer(0x00); // 0x00はマスターに返すダミーデータ

  // 12bitデータとして結合
  receivedValue = (uint16_t)highByte << 4; // 上位8bitを12bitデータの上位にシフト
  receivedValue |= (lowByte & 0x0F);       // 下位4bitのみ結合

  // 新しいデータが完全に受信された
  newDataAvailable = true; 

  // 新しいデータが受信されたらシリアルモニターに表示
  if (newDataAvailable) {
    Serial.print("Received: ");
    Serial.println(receivedValue);
    newDataAvailable = false; // フラグをリセット
  }

  // SPI通信のタイミングはマスターに依存するため、
  // ここで delay() などは入れない方が良い
}