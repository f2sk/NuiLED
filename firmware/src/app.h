// アプリ状態と操作アクション（ボタン/BLE 双方から呼ぶ共通ロジック）
#pragma once
#include "config.h"

extern AppState g_state;

void appInit();                       // 起動時：プリセット/状態復元＋反映
void appTogglePower();
void appSetPower(bool on);
void appCycleBrightness();
void appSetBrightness(uint8_t idx);
void appSetActiveSlot(uint8_t slot);  // スロット適用（channelMaskをpreset.flagsで再設定）
void appNextPreset();
void appCycleChannel();
void appSetChannelMask(uint8_t m);
const Preset& appActivePreset();
