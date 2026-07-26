#include <Preferences.h>
#include "config.h"
#include "store.h"

// Preferences は ESP32 Arduino core 内蔵。名前空間 "nuiled" に保存。
static Preferences prefs;

void storeLoad(AppState& s) {
  prefs.begin("nuiled", true);   // read-only
  s.powerOn         = prefs.getBool("pwr", true);
  s.brightnessIndex = prefs.getUChar("bri", 1);   // 既定=50%
  s.activeSlot      = prefs.getUChar("slot", 0);
  s.channelMask     = prefs.getUChar("ch",  CH_ALL);
  prefs.end();

  // 範囲外値のガード
  if (s.brightnessIndex >= NUM_BRIGHTNESS) s.brightnessIndex = 1;
  if (s.activeSlot >= NUM_SLOTS)           s.activeSlot = 0;
  if (s.channelMask == 0 || s.channelMask > CH_ALL) s.channelMask = CH_ALL;
}

void storeSave(const AppState& s) {
  // 書き込みは操作時のみ（頻度低）。Flash摩耗は実用上問題なし。
  prefs.begin("nuiled", false);
  prefs.putBool("pwr", s.powerOn);
  prefs.putUChar("bri", s.brightnessIndex);
  prefs.putUChar("slot", s.activeSlot);
  prefs.putUChar("ch",  s.channelMask);
  prefs.end();
}
