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
  // SSがLOWの時だけ処理 (前のデバッグスケッチのif文を復活させる)
  if (digitalRead(SS) == LOW) { 
    Serial.println("--- Master Selected ---"); // SSがLOWになったことを通知

    byte highByte = SPI.transfer(0x00);
    Serial.print("  Received 1st byte (High): ");
    Serial.println(highByte, HEX); // 16進数で表示

    byte lowByte = SPI.transfer(0x00);
    Serial.print("  Received 2nd byte (Low): ");
    Serial.println(lowByte, HEX);  // 16進数で表示

    uint16_t currentReceivedValue = (uint16_t)highByte << 4; 
    currentReceivedValue |= (lowByte & 0x0F);       

    Serial.print("  Combined Value: ");
    Serial.println(currentReceivedValue);
    Serial.println("-----------------------");
  } else {
    // マスターが選択していない間は、極力何もしない
    // ただし、loop()が速すぎてSerial.printlnがCPUを占有しないよう、
    // 必要なら少しのdelayを入れるか、SSがHIGHの間の出力は控える
    // Serial.println("SS is HIGH. Waiting..."); // これはたくさん出力されるので、通常はコメントアウト
    delay(1); // 軽く待つ
  }
}