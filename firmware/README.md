# NuiLED firmware

ホロフレ展示ケース（桃鈴ねね ソロライブ仕様）の制御ファーム。
ターゲット: Seeed XIAO ESP32-S3 / Arduino (PlatformIO)。

## 機能

- **単体動作（タクトSW×2）**
  - SW1 短: 輝度サイクル（30/50/70/100%） / 長: 全体ON-OFF（P-MOS母線）
  - SW2 短: プリセット順送り（8スロット） / 長: Front-Back個別サイクル
  - 状態（電源/輝度/スロット/チャンネル）はNVSに永続化、起動時復元
- **LED**: WS2812B Front60 + Back60、FastLEDで2.5A電流クランプ
- **プリセット**: データ駆動12byte（effect/speed/色）×8スロット、工場4＋ユーザー4
- **BLE①（NimBLE）**: プリセット作成・転送・NVS保存（散発R/W）。②リアルタイムは予約スタブ

## ビルド / 書き込み

```
pio run                     # ビルド
pio run -t upload           # 書き込み（BOOTSEL不要、USB-CDC）
pio device monitor          # シリアル 115200
```

## ピン（XIAO ESP32-S3）

| 信号 | ピン |
|---|---|
| Front Data | D0 (GPIO1) → 74AC125 |
| Back Data | D1 (GPIO2) → 74AC125 |
| 母線EN | D2 (GPIO3) → 2N7000 → P-MOS |
| SW1 / SW2 | D3 (GPIO4) / D4 (GPIO5) 内部プルアップ |

## 構成

| ファイル | 役割 |
|---|---|
| config.h | ピン/LED数/輝度段/色/Preset型/AppState |
| presets.* | 8スロット管理・工場出荷・NVS保存 |
| app.* | 状態＋操作アクション（ボタン/BLE共通） |
| leds.* | FastLED初期化・データ駆動描画エンジン・2.5Aクランプ |
| buttons.* | SW短長押し判定（デバウンス） |
| store.* | AppStateのNVS保存/復元 |
| power.* | 母線EN GPIO（突入抑制はHWのゲートRC） |
| ble.* | NimBLE GATT（①プリセット、②予約） |
| main.cpp | setup/loop |

## GATT

Service `a1b20001-5c3d-4e6f-8a90-1234567890ab`。詳細は設計仕様書 section 8 参照。

## 未実装（今後）

- 段階3: BLE② リアルタイム制御（Realtimeキャラ）・音楽同期
- Web Bluetooth UI（HTML）
- 実機での点灯・電流・レベル変換の確認（基板組み上げ後）
