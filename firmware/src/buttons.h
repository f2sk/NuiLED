// タクトSW（SW1/SW2）の短押し・長押し判定（デバウンス付き）
#pragma once
#include <Arduino.h>

enum BtnEvent { BTN_NONE, BTN_SHORT, BTN_LONG };

void buttonsInit();
// 毎ループ呼ぶ。SW1/SW2のイベントを out で返す（無ければ BTN_NONE）
void buttonsUpdate(BtnEvent& e1, BtnEvent& e2, uint32_t now);
