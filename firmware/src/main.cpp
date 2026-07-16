// NuiLED メイン（段階2：単体動作 + BLE①プリセット管理）
//  ボタン単体で ON-OFF/輝度/プリセット/Front-Back個別。BLEでプリセット編集・転送・保存。
#include "config.h"
#include "app.h"
#include "leds.h"
#include "buttons.h"
#include "power.h"
#include "ble.h"

void setup() {
  Serial.begin(115200);

  powerInit();      // 母線ENは初期LOW（消灯）
  buttonsInit();
  ledsInit();
  appInit();        // プリセット/状態を復元し反映
  bleInit();        // GATT開始・アドバタイズ
}

void loop() {
  const uint32_t now = millis();

  // --- ボタン処理（毎ループ、低レイテンシ） ---
  BtnEvent e1, e2;
  buttonsUpdate(e1, e2, now);

  if (e1 == BTN_SHORT)      appCycleBrightness();   // SW1短：輝度サイクル
  else if (e1 == BTN_LONG)  appTogglePower();       // SW1長：全体ON-OFF

  if (e2 == BTN_SHORT)      appNextPreset();         // SW2短：プリセット順送り
  else if (e2 == BTN_LONG)  appCycleChannel();       // SW2長：Front-Back個別サイクル

  // --- 描画（FPS間引き。消灯中は母線OFFなので描かない） ---
  static uint32_t lastFrame = 0;
  if (g_state.powerOn && (now - lastFrame) >= (1000 / FPS)) {
    lastFrame = now;
    ledsRender(g_state, appActivePreset(), now);
    ledsShow();
  }
}
