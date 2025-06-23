void setup() {
  Serial.begin(115200);
  Serial.println("Slave D10 Pin Test Started!");
  pinMode(10, INPUT_PULLUP); // D10ピンを入力（プルアップ有効）に設定
}

void loop() {
  int pinState = digitalRead(10);
  Serial.print("D10 Pin State: ");
  Serial.println(pinState == HIGH ? "HIGH" : "LOW");
  delay(100); // 0.1秒ごとに状態を表示
}