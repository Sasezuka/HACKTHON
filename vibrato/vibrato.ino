// --- ユーザー設定項目 ---

// I/Oピン設定
const int analogPin = A1; // 距離センサーを接続するアナログピン
const int buzzerPin = A0;  // (オプション) ブザーを接続するデジタルピン

// 音の基本設定
const float baseFrequency = 440.0; // 音の基本周波数 (Hz) - A4 (ラの音)

// --- LFO（ビブラート）の設定 ---
// LFOの基本周波数 (ビブラートの速さ) - Hz単位
// 例: 5Hzだと1秒間に5回揺れる
const float lfoFrequencyHz = 5.0;

// LFOの最大振幅 (ビブラートの深さ) - Hz単位
// 音の基本周波数からどれだけ上下に揺れるか
const float lfoAmplitudeMaxHz = 20.0; // 例えば最大20Hzの幅で揺らす

// --- 距離とLFOの関連付け範囲 (cm) ---
const float minDistanceCm = 5.0;  // ビブラートが最大になる距離（近い方）
const float maxDistanceCm = 30.0; // ビブラートがゼロになる距離（遠い方）

// --- 非ブロッキング処理のための変数 ---
unsigned long previousMillis = 0;
const long interval = 20; // LFOの値を更新する間隔 (ms)

void setup() {
  // シリアル通信速度を115200bpsに設定
  Serial.begin(115200);
  Serial.println("LFO波形生成プログラムを開始します。");

  pinMode(analogPin, INPUT);
  // pinMode(buzzerPin, OUTPUT); // ブザーを使う場合はこの行のコメントを外す
}

void loop() {
  unsigned long currentMillis = millis(); // 現在の時間を取得

  // 一定間隔でLFOの周波数を更新する
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis; // 前回更新時刻を保存

    // 距離センサーのアナログ値を読み込み、距離(cm)に変換
    // ※この式はシャープ製測距モジュールGP2Y0A21YK0Fなどを想定したものです
    int analogValue = analogRead(analogPin);
    float voltage = 5.0 * analogValue / 1024.0;
    float distanceCm = 27.149 * pow(voltage, -1.185);

    // センサーの値を安全な範囲内に収める（クリッピング）
    float clampedDistance = constrain(distanceCm, minDistanceCm, maxDistanceCm);

    // --- 距離に応じたLFO振幅の調整 ---
    // minDistanceCmからmaxDistanceCmの間で線形補間
    // 距離が近いほど (clampedDistanceが小さいほど) 振幅を大きくする
    // map()関数は整数用なので、浮動小数点数で手動計算します
    float normalizedFactor = 1.0 - ((clampedDistance - minDistanceCm) / (maxDistanceCm - minDistanceCm));
    float currentLfoAmplitudeHz = lfoAmplitudeMaxHz * normalizedFactor;


    // --- LFO波形の生成 ---
    // (2 * PI * LFO周波数Hz * 時間(秒))でサイン波の引数を生成
    // 時間はmillis()がミリ秒なので、1000.0で割って秒に変換
    float lfoOffset = sin(TWO_PI * lfoFrequencyHz * (currentMillis / 1000.0)) * currentLfoAmplitudeHz;

    // 最終的な周波数を計算
    float calculatedFrequency = baseFrequency + lfoOffset;

    // 周波数が0以下にならないようにする
    if (calculatedFrequency <= 0) {
      calculatedFrequency = 1.0;
    }

    // --- (オプション) ブザーで音を鳴らす ---
    // tone(buzzerPin, calculatedFrequency); // この行のコメントを外すと音が鳴ります

    // --- シリアルモニターに出力して確認 ---
    // シリアルプロッタで複数のデータを表示する場合、タブで区切るのが便利です
    // グラフ1: 距離 (cm)
    Serial.print(distanceCm, 1);
    Serial.print("\t");
    // グラフ2: LFOオフセット (Hz) - 揺れ幅
    Serial.print(lfoOffset, 2);
    Serial.print("\t");
    // グラフ3: 最終的な周波数 (Hz) - ビブラートの波形
    Serial.println(calculatedFrequency, 1);
  }
}