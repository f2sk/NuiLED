// 設定・定数・型（仕様書 section 6/7/14 に対応）
#pragma once
#include <Arduino.h>
#include <FastLED.h>

// ===== ピン定義（XIAO ESP32-S3, 仕様書14.4） =====
constexpr uint8_t PIN_FRONT_DATA = 1;   // D0 (GPIO1) → 74AC125 → Front Din
constexpr uint8_t PIN_BACK_DATA  = 2;   // D1 (GPIO2) → 74AC125 → Back  Din
constexpr uint8_t PIN_BUS_EN     = 3;   // D2 (GPIO3) → 2N7000 → P-MOS母線EN(H=点灯)
constexpr uint8_t PIN_SW1        = 4;   // D3 (GPIO4) 内部プルアップ
constexpr uint8_t PIN_SW2        = 5;   // D4 (GPIO5) 内部プルアップ

// ===== LED構成 =====
constexpr uint16_t NUM_FRONT = 60;      // 60LED/m × 1本
constexpr uint16_t NUM_BACK  = 60;      // 60LED/m × 1本

// ===== 電源クランプ（仕様書14.1/14.6：2.5A頭打ち） =====
constexpr uint8_t  PWR_VOLTS     = 5;
constexpr uint16_t PWR_MILLIAMPS = 2500;

// ===== 輝度段（30/50/70/100% を 0-255 にマップ） =====
constexpr uint8_t BRIGHTNESS_LEVELS[] = {77, 128, 179, 255};
constexpr uint8_t NUM_BRIGHTNESS = sizeof(BRIGHTNESS_LEVELS);

// ===== プリセットスロット =====
constexpr uint8_t NUM_SLOTS  = 8;       // 工場4 + ユーザー4（全編集可）
constexpr uint8_t MAX_COLORS = 6;       // パレット最大色数（可変。UI/BLE/NVS共通）

// ===== 演出タイプ =====
enum Effect : uint8_t {
  EFF_SOLID     = 0,  // 単色（palette[0]）
  EFF_BREATH    = 1,  // ブレス（1色=明滅 / 2色以上=色ゆらぎ）
  EFF_ALTERNATE = 2,  // 交互（周期＝ブロック長、パレットを繰り返し）
  EFF_WAVE      = 3,  // ウェーブ（パレットのグラデを流す）
  EFF_CHASE     = 4,  // チェイス（パレット各色を走らせる）
  EFF_COUNT
};

// ===== チャンネル別プリセット（Front/Back で共通の演出エンジン）=====
// 23byte固定：effect,speed,period,brightness,numColors, palette(RGB×MAX_COLORS)
struct ChannelPreset {
  uint8_t effect;
  uint8_t speed;                  // 演出速度 0-255
  uint8_t period;                 // 交互のブロック長(LED) 等
  uint8_t brightness;             // チャンネル基本明るさ 0-255（×グローバルマスタ）
  uint8_t numColors;              // 1..MAX_COLORS
  uint8_t pal[MAX_COLORS * 3];    // RGB × MAX_COLORS
};
static_assert(sizeof(ChannelPreset) == 5 + MAX_COLORS * 3, "ChannelPreset size");

// ===== プリセット（Front/Back 各チャンネル＋有効フラグ）=====
struct Preset {
  ChannelPreset front;
  ChannelPreset back;
  uint8_t flags;                  // bit0=Front有効, bit1=Back有効（=CH_*）
};
static_assert(sizeof(Preset) == sizeof(ChannelPreset) * 2 + 1, "Preset size (NVS/BLE共通)");

// パレットのi番目をCRGBで取得
inline CRGB palColor(const ChannelPreset& c, uint8_t i) {
  uint8_t n = c.numColors ? c.numColors : 1;
  i %= n;
  return CRGB(c.pal[i * 3], c.pal[i * 3 + 1], c.pal[i * 3 + 2]);
}

// ===== 基本色（工場出荷プリセットで使用） =====
const CRGB COL_WARM_WHITE = CRGB(255, 145, 65);  // 暖色寄りの白（R満・G中・B低）
const CRGB COL_PINK       = CRGB(255, 40, 90);
const CRGB COL_ORANGE     = CRGB(255, 90, 0);

// ===== チャンネルマスク（SW2長押しで Front/Back 個別サイクル） =====
constexpr uint8_t CH_FRONT = 0x01;
constexpr uint8_t CH_BACK  = 0x02;
constexpr uint8_t CH_BOTH  = 0x03;
inline uint8_t nextChannelMask(uint8_t m) {
  if (m == CH_BOTH)  return CH_FRONT;   // 両方 → Frontのみ → Backのみ → 両方
  if (m == CH_FRONT) return CH_BACK;
  return CH_BOTH;
}

// ===== ボタン判定 =====
constexpr uint16_t BTN_DEBOUNCE_MS = 20;
constexpr uint16_t BTN_LONG_MS     = 1000;

// ===== 描画 =====
constexpr uint8_t FPS = 60;

// ===== アプリ状態（NVSで永続化する対象） =====
struct AppState {
  bool    powerOn;          // 全体ON/OFF（母線EN）
  uint8_t brightnessIndex;  // BRIGHTNESS_LEVELS のインデックス
  uint8_t activeSlot;       // 0..NUM_SLOTS-1
  uint8_t channelMask;      // CH_FRONT/CH_BACK/CH_BOTH
};
