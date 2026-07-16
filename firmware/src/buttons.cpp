#include "config.h"
#include "buttons.h"

// 1ボタン分の状態。押下=LOW（内部プルアップ）。
struct BtnCtx {
  uint8_t  pin;
  bool     lastRaw;     // 直近の生読み値（押下=true）
  bool     stable;      // デバウンス後の確定値
  uint32_t lastChange;  // 生値が変化した時刻
  uint32_t downAt;      // 押し始めの時刻
  bool     longFired;   // この押下で長押しイベントを出したか
};

static BtnCtx b1, b2;

static void initCtx(BtnCtx& b, uint8_t pin) {
  b.pin = pin;
  b.lastRaw = false;
  b.stable = false;
  b.lastChange = 0;
  b.downAt = 0;
  b.longFired = false;
}

void buttonsInit() {
  pinMode(PIN_SW1, INPUT_PULLUP);
  pinMode(PIN_SW2, INPUT_PULLUP);
  initCtx(b1, PIN_SW1);
  initCtx(b2, PIN_SW2);
}

static BtnEvent poll(BtnCtx& b, uint32_t now) {
  bool raw = (digitalRead(b.pin) == LOW);   // 押下=LOW
  if (raw != b.lastRaw) {
    b.lastRaw = raw;
    b.lastChange = now;
  }

  BtnEvent ev = BTN_NONE;

  // デバウンス経過後に確定値を更新
  if ((now - b.lastChange) >= BTN_DEBOUNCE_MS && raw != b.stable) {
    b.stable = raw;
    if (raw) {
      // 押し始め
      b.downAt = now;
      b.longFired = false;
    } else {
      // 離した：長押し未発火かつ閾値未満なら短押し確定
      if (!b.longFired && (now - b.downAt) < BTN_LONG_MS) {
        ev = BTN_SHORT;
      }
    }
  }

  // 押しっぱなしで長押し閾値を超えたら即発火（離す前に反応）
  if (b.stable && !b.longFired && (now - b.downAt) >= BTN_LONG_MS) {
    b.longFired = true;
    ev = BTN_LONG;
  }

  return ev;
}

void buttonsUpdate(BtnEvent& e1, BtnEvent& e2, uint32_t now) {
  e1 = poll(b1, now);
  e2 = poll(b2, now);
}
