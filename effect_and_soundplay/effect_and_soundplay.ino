// --- Arduino3_I2C_Slave_Test.ino ---

#include <Wire.h> // I2C通信ライブラリをインクルード

const int SLAVE_ADDRESS = 8; // マスターと同じアドレスを設定

volatile uint16_t receivedValue = 0;
volatile bool newDataAvailable = false; // 新しいデータが受信されたことを示すフラグ

void setup() {
  Serial.begin(115200);
  Serial.println("Arduino 3 (Slave) I2C Test Started!");

  // スレーブアドレスを指定してI2C通信を開始
  Wire.begin(SLAVE_ADDRESS); 
  
  // マスターからデータが受信されたときに呼び出される関数を登録
  Wire.onReceive(receiveEvent); 

  Serial.println("I2C Slave Initialized.");
}

void loop() {
  // 新しいデータが受信されたらシリアルモニターに表示
  // onReceiveイベントは割り込みで実行されるため、loop()は他の処理に使える
  if (newDataAvailable) {
    Serial.print("Received: ");
    Serial.println(receivedValue);
    newDataAvailable = false; // フラグをリセット
  }
  // その他の処理があればここに記述
}

// データ受信時に呼び出される関数
// マスタからデータを受信すると、この関数が自動的に呼び出される
void receiveEvent(int howMany) {
  byte highByte = 0;
  byte lowByte = 0;

  if (howMany >= 2) { // 少なくとも2バイト受信したことを確認
    highByte = Wire.read(); // 最初のバイト（上位8ビット）を読み込む
    lowByte = Wire.read();  // 次のバイト（下位8ビット）を読み込む
    
    // 12bitデータとして結合 (マスターが送る形式に合わせて調整)
    // マスター側ではsendValueの>>8と&0xFFで上位8bitと下位8bitに分割しているため
    // スレーブ側ではreceivedValue = (uint16_t)highByte << 8; と結合し、
    // lowByteはそのまま加算する。
    // 元のSPIではlowByteが4ビットだけだったが、I2Cでは8ビットまるごと送信する。
    // もし12ビットデータとして厳密に扱うなら、lowByteの0x0Fマスクは不要。
    // ここではSPIの結合ロジックに合わせて、上位8bit + 下位4bitとして結合
    // もしくは、送るデータに合わせて結合ロジックを調整します。
    // 例：12bitデータを上位4bit(0x0F) + 下位8bit(0xFF)で送る場合
    // highByte = (sendValue >> 8) & 0x0F;
    // lowByte = sendValue & 0xFF;
    // receivedValue = (uint16_t)highByte << 8 | lowByte;

    // 今回はsendValueを上位8bitと下位8bitで送るので、その結合方法で。
    // もし12bitとして厳密に結合したいなら、lowByteのマスクは不要
    // receivedValue = ((uint16_t)highByte << 8) | lowByte;

    // SPIの時の12bitデータフォーマットを維持するなら
    // マスター: byte highByte = (sendValue >> 4) & 0xFF; byte lowByte = sendValue & 0x0F;
    // スレーブ: receivedValue = (uint16_t)highByte << 4; receivedValue |= (lowByte & 0x0F);
    // しかしI2Cなら2バイトまるごと送る方が自然なので、一般的な16bit結合で
    receivedValue = ((uint16_t)highByte << 8) | lowByte;
    
    newDataAvailable = true;
  }
}