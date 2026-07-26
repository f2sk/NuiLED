#include <Preferences.h>
#include <string.h>
#include "config.h"
#include "presets.h"

static Preset  g_slots[NUM_SLOTS];
static Preferences prefsP;

// 基本色（RGBバイト列）
static const uint8_t WARM[3]   = {255, 145, 65};
static const uint8_t PINK[3]   = {255, 40, 90};
static const uint8_t ORANGE[3] = {255, 90, 0};

// チャンネルを組み立て（rgbはnc色ぶんのRGB連結）
static void mkChan(ChannelPreset& c, uint8_t eff, uint8_t sp, uint8_t per,
                   uint8_t bri, uint8_t nc, const uint8_t* rgb) {
  c.effect = eff; c.speed = sp; c.period = per; c.brightness = bri;
  c.numColors = nc;
  memset(c.pal, 0, sizeof(c.pal));
  memcpy(c.pal, rgb, (size_t)nc * 3);
}

// 複数色を連結した一時バッファ生成用
static void cat2(uint8_t* dst, const uint8_t* a, const uint8_t* b) {
  memcpy(dst, a, 3); memcpy(dst + 3, b, 3);
}
static void cat3(uint8_t* dst, const uint8_t* a, const uint8_t* b, const uint8_t* c) {
  memcpy(dst, a, 3); memcpy(dst + 3, b, 3); memcpy(dst + 6, c, 3);
}

// 工場出荷プリセットの控えめ輝度。ブレッドボード仮組み・初回投入を安全側に。
// 実機ではWeb UI（チャンネル明るさ）やSW1（マスタ）で引き上げる前提。
static const uint8_t TB = 90;

static void factory() {
  uint8_t two[6], three[9];

  // 全スロット共通：Ground=暖白のアンビエント（後でUIで変更可）
  for (uint8_t i = 0; i < NUM_SLOTS; i++)
    mkChan(g_slots[i].ground, EFF_SOLID, 0, 0, TB, 1, WARM);

  // slot0: 暖白のみ（Front単色、Back無効）
  mkChan(g_slots[0].front, EFF_SOLID, 0, 0, TB, 1, WARM);
  mkChan(g_slots[0].back,  EFF_SOLID, 0, 0, TB, 1, WARM);
  g_slots[0].flags = CH_FRONT | CH_GROUND;

  // slot1: 暖白 + 桃橙ブレス
  cat2(two, PINK, ORANGE);
  mkChan(g_slots[1].front, EFF_SOLID,  0, 0, TB, 1, WARM);
  mkChan(g_slots[1].back,  EFF_BREATH, 64, 0, TB, 2, two);
  g_slots[1].flags = CH_ALL;

  // slot2: 暖白 + 桃橙ウェーブ
  cat2(two, ORANGE, PINK);
  mkChan(g_slots[2].front, EFF_SOLID, 0, 0, TB, 1, WARM);
  mkChan(g_slots[2].back,  EFF_WAVE, 128, 0, TB, 2, two);
  g_slots[2].flags = CH_ALL;

  // slot3: 暖白 + 桃橙チェイス
  cat2(two, PINK, ORANGE);
  mkChan(g_slots[3].front, EFF_SOLID,  0, 0, TB, 1, WARM);
  mkChan(g_slots[3].back,  EFF_CHASE, 128, 0, TB, 2, two);
  g_slots[3].flags = CH_ALL;

  // slot4: 暖白 + 3色交互（周期5LED）
  cat3(three, PINK, ORANGE, WARM);
  mkChan(g_slots[4].front, EFF_SOLID,     0, 0, TB, 1, WARM);
  mkChan(g_slots[4].back,  EFF_ALTERNATE, 40, 5, TB, 3, three);
  g_slots[4].flags = CH_ALL;

  // slot5: Back桃橙ブレスのみ（Front無効）
  cat2(two, PINK, ORANGE);
  mkChan(g_slots[5].front, EFF_SOLID,  0, 0, TB, 1, WARM);
  mkChan(g_slots[5].back,  EFF_BREATH, 40, 0, TB, 2, two);
  g_slots[5].flags = CH_BACK | CH_GROUND;

  // slot6: Back速ウェーブのみ
  cat2(two, PINK, ORANGE);
  mkChan(g_slots[6].front, EFF_SOLID, 0, 0, TB, 1, WARM);
  mkChan(g_slots[6].back,  EFF_WAVE, 220, 0, TB, 2, two);
  g_slots[6].flags = CH_BACK | CH_GROUND;

  // slot7: 暖白のみ（予備）
  mkChan(g_slots[7].front, EFF_SOLID, 0, 0, TB, 1, WARM);
  mkChan(g_slots[7].back,  EFF_SOLID, 0, 0, TB, 1, WARM);
  g_slots[7].flags = CH_FRONT | CH_GROUND;
}

void presetsLoad() {
  prefsP.begin("nuiled_ps", true);
  size_t got = prefsP.getBytes("slots", g_slots, sizeof(g_slots));
  prefsP.end();
  if (got != sizeof(g_slots)) {   // 未初期化 or サイズ不一致（型変更含む）
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
