#include "config.h"
#include "leds.h"

// LEDバッファ（Front/Back 独立2系統）
static CRGB frontLeds[NUM_FRONT];
static CRGB backLeds[NUM_BACK];

void ledsInit() {
  FastLED.addLeds<WS2812B, PIN_FRONT_DATA, GRB>(frontLeds, NUM_FRONT);
  FastLED.addLeds<WS2812B, PIN_BACK_DATA,  GRB>(backLeds,  NUM_BACK);
  // 全体電流の上限クランプ（全白でも自動減光で2.5A以下）
  FastLED.setMaxPowerInVoltsAndMilliamps(PWR_VOLTS, PWR_MILLIAMPS);
  FastLED.clear(true);
}

// パレットを t(0-255) で滑らかに巡回した色
static CRGB paletteAt(const ChannelPreset& c, uint8_t t) {
  uint8_t n = c.numColors ? c.numColors : 1;
  if (n == 1) return palColor(c, 0);
  uint16_t scaled = (uint16_t)t * n;   // 0..n*255
  uint8_t seg = scaled >> 8;           // 0..n-1
  uint8_t f = scaled & 0xFF;
  return blend(palColor(c, seg), palColor(c, seg + 1), f);
}

// 1チャンネル分を描画（演出エンジン本体）
static void renderChannel(CRGB* buf, uint16_t n, const ChannelPreset& c, uint32_t now) {
  const uint8_t nc = c.numColors ? c.numColors : 1;

  switch (c.effect) {
    case EFF_SOLID:
      fill_solid(buf, n, palColor(c, 0));
      break;

    case EFF_BREATH: {
      uint8_t bpm = map(c.speed, 0, 255, 4, 30);
      if (nc >= 2) {                      // 2色以上：色をゆらす
        fill_solid(buf, n, paletteAt(c, beatsin8(bpm)));
      } else {                            // 1色：明滅
        CRGB col = palColor(c, 0);
        col.nscale8_video(scale8(beatsin8(bpm), 200) + 55);
        fill_solid(buf, n, col);
      }
      break;
    }

    case EFF_ALTERNATE: {                 // 交互：周期＝ブロック長、任意で流す
      uint8_t blk = c.period ? c.period : 1;
      uint16_t scroll = (c.speed == 0) ? 0 : (uint16_t)(now / map(c.speed, 1, 255, 400, 20));
      for (uint16_t i = 0; i < n; i++) {
        uint8_t idx = ((i + scroll) / blk) % nc;
        buf[i] = palColor(c, idx);
      }
      break;
    }

    case EFF_WAVE: {                      // パレットのグラデを流す
      uint8_t denom = 17 - (uint8_t)map(c.speed, 0, 255, 2, 15);
      for (uint16_t i = 0; i < n; i++)
        buf[i] = paletteAt(c, (uint8_t)(i * 8 + now / denom));
      break;
    }

    case EFF_CHASE: {                     // パレット各色を等間隔で走らせる
      fadeToBlackBy(buf, n, 40);
      uint16_t div = map(c.speed, 0, 255, 80, 10);
      if (div < 1) div = 1;
      uint16_t step = now / div;
      for (uint8_t k = 0; k < nc; k++) {
        uint16_t pos = (step + (uint16_t)k * (n / nc)) % n;
        buf[pos] = palColor(c, k);
      }
      break;
    }

    default:
      fill_solid(buf, n, palColor(c, 0));
      break;
  }

  // チャンネル基本明るさを適用（グローバルマスタはFastLED.show側で掛かる）
  if (c.brightness < 255)
    for (uint16_t i = 0; i < n; i++) buf[i].nscale8_video(c.brightness);
}

void ledsRender(const AppState& s, const Preset& p, uint32_t now) {
  const bool fe = (p.flags & CH_FRONT) && (s.channelMask & CH_FRONT);
  const bool be = (p.flags & CH_BACK)  && (s.channelMask & CH_BACK);

  if (fe) renderChannel(frontLeds, NUM_FRONT, p.front, now);
  else    fill_solid(frontLeds, NUM_FRONT, CRGB::Black);

  if (be) renderChannel(backLeds, NUM_BACK, p.back, now);
  else    fill_solid(backLeds, NUM_BACK, CRGB::Black);
}

void ledsShow() {
  FastLED.show();
}
