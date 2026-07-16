#include <Preferences.h>
#include <string.h>
#include "config.h"
#include "presets.h"

static Preset  g_slots[NUM_SLOTS];
static Preferences prefsP;

// 工場出荷プリセット（slot0-3=仕様書section7の4種、slot4-7=派生）
static void factory() {
  //          effect     speed  front(R,G,B)   backA(R,G,B)  backB(R,G,B)  flags
  g_slots[0] = { EFF_SOLID,  128, 255,145,65,   0,0,0,        0,0,0,        CH_FRONT }; // 暖白のみ
  g_slots[1] = { EFF_BREATH,  64, 255,145,65,   255,40,90,    255,90,0,     CH_BOTH  }; // 暖白+桃橙ブレス
  g_slots[2] = { EFF_WAVE,   128, 255,145,65,   255,90,0,     255,40,90,    CH_BOTH  }; // 桃橙ウェーブ
  g_slots[3] = { EFF_CHASE,  128, 255,145,65,   255,40,90,    255,90,0,     CH_BOTH  }; // チェイス
  g_slots[4] = { EFF_SOLID,  128, 255,120,40,   255,40,90,    0,0,0,        CH_BOTH  }; // 暖白+桃solid
  g_slots[5] = { EFF_BREATH,  40, 0,0,0,        255,40,90,    255,90,0,     CH_BACK  }; // Back桃橙ブレスのみ
  g_slots[6] = { EFF_WAVE,   200, 255,145,65,   255,40,90,    255,90,0,     CH_BACK  }; // Back速ウェーブ
  g_slots[7] = { EFF_SOLID,  128, 255,145,65,   0,0,0,        0,0,0,        CH_FRONT }; // 暖白のみ(予備)
}

void presetsLoad() {
  prefsP.begin("nuiled_ps", true);
  size_t got = prefsP.getBytes("slots", g_slots, sizeof(g_slots));
  prefsP.end();
  if (got != sizeof(g_slots)) {   // 未初期化 or サイズ不一致
    factory();
    presetsSave();
  }
}

void presetsSave() {
  prefsP.begin("nuiled_ps", false);
  prefsP.putBytes("slots", g_slots, sizeof(g_slots));
  prefsP.end();
}

void presetsFactoryReset() {
  factory();
  presetsSave();
}

const Preset& presetsGet(uint8_t i) {
  return g_slots[i < NUM_SLOTS ? i : 0];
}

void presetsSet(uint8_t i, const Preset& p) {
  if (i < NUM_SLOTS) g_slots[i] = p;
}
