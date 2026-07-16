#include "config.h"
#include "power.h"

// EN=H → 2N7000 ON → P-MOSゲートを引き下げ → ハイサイドP-MOS ON → 母線給電。
// 立ち上がりの緩やかさ（突入抑制）はゲートのRCソフトスタートが担当。
void powerInit() {
  pinMode(PIN_BUS_EN, OUTPUT);
  digitalWrite(PIN_BUS_EN, LOW);   // 起動直後は消灯
}

void powerSet(bool on) {
  digitalWrite(PIN_BUS_EN, on ? HIGH : LOW);
}
