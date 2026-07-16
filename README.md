# NuiLED

ホロライブ公式ぬいぐるみ「ホロフレ」（桃鈴ねね ソロライブ仕様）向けの、電飾付き展示ケース。
透明バッグ＋3Dプリント内部フレームに WS2812B を仕込み、XIAO ESP32-S3 で制御する。

- **本体操作**：タクトスイッチ2個でON/OFF・輝度・プリセット・Front/Back個別（スマホ不要で完結）
- **スマホ操作**：Web Bluetooth（Chrome/Edge）でプリセットの作成・転送・保存
- **給電**：USB-C 5V/3A、LED総電流は2.5Aで上限クランプ

## 🎛 Web コントローラ

👉 **https://f2sk.github.io/NuiLED/**

Android Chrome / PC Chrome・Edge で開き、「接続」から本体（`NuiLED`）へBLE接続。
※ iOSは標準ブラウザ非対応（Bluefy等のWebBLEブラウザが必要）。

## 構成

| ディレクトリ | 内容 |
|---|---|
| [`firmware/`](firmware/) | XIAO ESP32-S3 ファーム（PlatformIO / FastLED / NimBLE） |
| [`docs/`](docs/) | Web Bluetooth UI（GitHub Pagesで配信） |
| [設計仕様書 (DESIGN.md)](DESIGN.md) | 筐体・回路・ファーム・BLEの詳細設計 |

## ハードウェア概要

- MCU: Seeed XIAO ESP32-S3
- LED: WS2812B 60LED/m × 2（Front照明＝暖色白 / Back演出＝ピンク・オレンジ）
- レベル変換: TC74AC125P（VCCシフトで3.3→5V成立）
- 母線スイッチ: Pch MOSFET（ゲートRCソフトスタート）＋ 2N7000ゲート駆動
- 電源: USB-C 5V/3A、FastLEDで2.5A電流クランプ

詳細・BOM（秋月電子）・成立性確認は設計仕様書を参照。

## ライセンス

[MIT](LICENSE)
