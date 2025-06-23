void setup() {
  Serial.begin(115200);
  Serial.println("Master D10 Pin Test Started!");
  pinMode(10, OUTPUT); // D10ピンを出力に設定
}

void loop() {
  digitalWrite(10, LOW);  // D10ピンをLOWにする
  Serial.println("D10 is LOW");
  delay(500);             // 0.5秒待つ

  digitalWrite(10, HIGH); // D10ピンをHIGHにする
  Serial.println("D10 is HIGH");
  delay(500);             // 0.5秒待つ
}