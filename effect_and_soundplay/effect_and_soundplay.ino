// --- Arduino3_Waveform_Receiver_UART.ino ---

// #include <SPI.h> // SPIライブラリは不要になるのでコメントアウトまたは削除

// 今回はSerial1で受信するので、Serial1を使います
// TX: デジタルピン1, RX: デジタルピン0

void setup() {
  Serial.begin(115200);   // PCとのデバッグ用シリアル通信
  Serial.println("Arduino 3 (Receiver) Waveform Started!");

  // ★★★ 変更点：Serial1 のボーレートを送信側と合わせる ★★★
  Serial1.begin(SERIAL_BAUDRATE); // 送信側と同じボーレートに設定 (constants.hの値)
  Serial.println("Serial1 initialized.");

  // SPI関連のセットアップは不要なので削除またはコメントアウト
  // pinMode(SS, INPUT_PULLUP);
  // SPI.begin();
}

// 受信したデータを結合するための変数
volatile byte receivedHighByte = 0;
volatile byte receivedLowByte = 0;
volatile bool firstByteReceived = false; // 1バイト目を受信したかを示すフラグ

void loop() {
  if (Serial1.available()) { // Serial1 に受信データがあるか確認
    byte incomingByte = Serial1.read();

    if (!firstByteReceived) {
      // 1バイト目（上位バイト）を受信
      receivedHighByte = incomingByte;
      firstByteReceived = true;
    } else {
      // 2バイト目（下位バイト）を受信
      receivedLowByte = incomingByte;
      
      // 2バイトを結合して16bit値にする
      uint16_t receivedSample = ((uint16_t)receivedHighByte << 8) | receivedLowByte;

      // 受信したデータをシリアルモニターに表示
      Serial.print("Received (raw 2 bytes): ");
      Serial.print(receivedHighByte, HEX);
      Serial.print(" ");
      Serial.print(receivedLowByte, HEX);
      Serial.print(", Combined: ");
      Serial.println(receivedSample);

      // TODO: ここで receivedSample を使って音を生成
      // 例: analogWrite(PWM_PIN, receivedSample >> 4); // 12bitから8bitに変換してPWM出力
      // あるいは、適切なDAC出力など

      firstByteReceived = false; // 次のペアのためにフラグをリセット
    }
  }
}