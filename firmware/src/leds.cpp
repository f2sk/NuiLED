#include "config.h"
#include "leds.h"

// 物理データ線バッファ（各60球）。lineA=D0, lineB=D1。
// 各線 [0..GROUND_LEN) = 床セグメント、[GROUND_LEN..STRIP_LEN) = 壁面。
static CRGB lineA[STRIP_LEN];
static CRGB lineB[STRIP_LEN];
// 床ゾーンの作業バッファ（34球連続。チェイスのフェード保持のため常設）
static CRGB groundBuf[GROUND_TOTAL];

void ledsInit() {
  FastLED.addLeds<WS2812B, PIN_FRONT_DATA, GRB>(lineA, STRIP_LEN);
  FastLED.addLeds<WS2812B, PIN_BACK_DATA,  GRB>(lineB, STRIP_LEN);
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
  const bool fe = (p.flags & CH_FRONT)  && (s.channelMask & CH_FRONT);
  const bool be = (p.flags & CH_BACK)   && (s.channelMask & CH_BACK);
  const bool ge = (p.flags & CH_GROUND) && (s.channelMask & CH_GROUND);

  // 壁の割り当て（コネクタ逆対応）：どちらの線の壁がFront/Backか
  CRGB* frontLine = SWAP_WALL ? lineB : lineA;
  CRGB* backLine  = SWAP_WALL ? lineA : lineB;

  // Front壁 = frontLine[GROUND_LEN..STRIP_LEN)
  if (fe) renderChannel(frontLine + GROUND_LEN, WALL_LEN, p.front, now);
  else    fill_solid(frontLine + GROUND_LEN, WALL_LEN, CRGB::Black);

  // Back壁 = backLine[GROUND_LEN..STRIP_LEN)
  if (be) renderChannel(backLine + GROUND_LEN, WALL_LEN, p.back, now);
  else    fill_solid(backLine + GROUND_LEN, WALL_LEN, CRGB::Black);

  // Ground = 34球連続で描画 → lineA床[0..17)とlineB床[0..17)へ分配
  if (ge) {
    renderChannel(groundBuf, GROUND_TOTAL, p.ground, now);
    for (uint16_t i = 0; i < GROUND_LEN; i++) {
      lineA[i] = groundBuf[i];
      lineB[i] = groundBuf[GROUND_LEN + i];
    }
  } else {
    fill_solid(lineA, GROUND_LEN, CRGB::Black);
    fill_solid(lineB, GROUND_LEN, CRGB::Black);
  }
}

void ledsShow() {
  FastLED.show();
}
