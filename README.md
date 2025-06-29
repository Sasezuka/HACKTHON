```mermaid
graph TD
    A[スタート] --> B{送信側Arduino 起動};
    B --> C[CheckSensor.h / .cpp クラス初期化];
    C --> D[setup() 関数実行];
    D --> E[Serial.begin(115200) / Serial1.begin(115200)];
    E --> F[MySensorWatcher::onSensorStateChange(handleSensorStateChange)];
    F --> G[loop() 関数開始];

    subgraph 送信側 Arduino (Send Unit)
        G -- ループ開始 --> H[MySensorWatcher::checkAllSensors() 呼び出し];
        H --> I{各センサーの現在の状態を読み込む};
        I --> J{現在の状態 != 前回の安定状態?};
        J -- Yes --> K[10ms delay (チャタリング待ち)];
        K --> L[再度センサーの状態を読み込む];
        L --> M{安定状態を判断};
        J -- No --> M;
        M --> N{安定状態 != 前回のコールバック状態?};
        N -- Yes --> O[handleSensorStateChange() 呼び出し];
        O --> P[計測開始 (micros())];
        P --> Q[Serial1で"B<番号>:<状態>"を送信];
        Q --> R[計測終了 (micros())];
        R --> S[経過時間と検知回数をSerialで出力];
        S --> T[コールバック状態を更新];
        T --> U[次回の比較のために現在の読み取り値を保存];
        N -- No --> U;
        U --> V{全てのセンサーをチェックしたか?};
        V -- No --> I;
        V -- Yes --> H;
    end

    subgraph 受信側 Arduino (Receive & Sound Unit)
        G'[""] -- 独立並行処理 --> A'[スタート];
        A' --> B'[FspTimer, playSound クラス初期化];
        B' --> C'[setup() 関数実行];
        C' --> D'[Serial.begin(115200) / Serial1.begin(115200)];
        D' --> E'[DAC (A0) 初期化 (analogWriteResolution)];
        E' --> F'[サンプル生成タイマー設定 (FspTimer::begin)];
        F --> G'[タイマー開始 (timer.start())];
        G' --> H'[loop() 関数開始];

        H' -- ループ開始 --> I'[processSerialCommand() 呼び出し];
        I' --> J'{Serial1からデータが利用可能か?};
        J' -- Yes --> K'{1文字読み込み};
        K' --> L'{改行または復帰文字か?};
        L' -- Yes --> M{受信バッファにデータがあるか?};
        M -- Yes --> N{コマンドを解析 ("B<番号>:<状態>")};
        N -- 有効なコマンド --> O[計測開始 (micros()) & 計測フラグON];
        O --> P{状態が"PUSH"か?};
        P -- Yes --> Q[myHarpController.handlePlayCommand(noteIndex)];
        Q --> R[音再生回数をインクリメント];
        R --> S[受信バッファをリセット];
        P -- No (RELEASE) --> Q'[myHarpController.handleStopCommand()];
        Q' --> S;
        L' -- No --> K'';
        K''[受信バッファに文字を追加];
        K'' --> J';
        M -- No --> J';
        N -- 無効なコマンド --> S;
        J' -- No --> T[DAC出力処理開始];

        T --> U[noInterrupts() (割り込み無効)];
        U --> V{audioBufferにサンプルがあるか?};
        V -- Yes --> W[サンプルを取得し、bufferReadIndexを更新];
        W --> X[bufferCountをデクリメント];
        X --> Y[interrupts() (割り込み有効)];
        Y --> Z[analogWrite(A0, sampleToPlay)で音を出力];
        Z --> AA{計測フラグがONか?};
        AA -- Yes --> BB[計測終了 (micros())];
        BB --> CC[経過時間と音再生回数をSerialで出力];
        CC --> DD[計測フラグOFF];
        AA -- No --> EE[""];
        EE --> I';
        V -- No --> Y;
    end

    G --> G';
