// FastLED初期化と描画（データ駆動：Presetを解釈して描く、2.5Aクランプ）
#pragma once
#include "config.h"

void ledsInit();
void ledsRender(const AppState& s, const Preset& p, uint32_t now); // showは呼び出し側
void ledsShow();
