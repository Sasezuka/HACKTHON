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
  // スレーブセレクトピンの状態をひたすら表示する
  // これでSSピンの配線とマスターからの信号が来ているかを確認

  int ss_state = digitalRead(SS);
  Serial.print("SS Pin State: ");
  Serial.println(ss_state == HIGH ? "HIGH" : "LOW");

  // SPI.transfer() は呼び出さない（ブロックされる可能性を排除）
  delay(100); // 100msごとに状態を表示
}