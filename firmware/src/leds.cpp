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

// Presetのパラメータを解釈して1フレーム描く（データ駆動エンジン）
void ledsRender(const AppState& s, const Preset& p, uint32_t now) {
  const CRGB front = CRGB(p.frontR, p.frontG, p.frontB);
  const CRGB backA = CRGB(p.backR1, p.backG1, p.backB1);
  const CRGB backB = CRGB(p.backR2, p.backG2, p.backB2);

  // チェイス以外は毎フレームクリア（チェイスは残像フェードのため保持）
  if (p.effect != EFF_CHASE) {
    fill_solid(frontLeds, NUM_FRONT, CRGB::Black);
    fill_solid(backLeds,  NUM_BACK,  CRGB::Black);
  }

  switch (p.effect) {
    case EFF_SOLID:
      fill_solid(frontLeds, NUM_FRONT, front);
      fill_solid(backLeds,  NUM_BACK,  backA);
      break;

    case EFF_BREATH: {
      fill_solid(frontLeds, NUM_FRONT, front);
      uint8_t bpm = map(p.speed, 0, 255, 4, 30);
      uint8_t b = beatsin8(bpm);                 // 0-255
      CRGB c = blend(backA, backB, b);
      c.nscale8_video(scale8(b, 200) + 55);      // 明るさも緩く揺らす
      fill_solid(backLeds, NUM_BACK, c);
      break;
    }

    case EFF_WAVE: {
      fill_solid(frontLeds, NUM_FRONT, front);
      uint8_t denom = 17 - (uint8_t)map(p.speed, 0, 255, 2, 16); // 速度で流れる速さ
      for (uint16_t i = 0; i < NUM_BACK; i++) {
        uint8_t w = sin8(i * 8 + (uint8_t)(now / denom));
        CRGB c = blend(backA, backB, w);
        c.nscale8_video(w);
        backLeds[i] = c;
      }
      break;
    }

    case EFF_CHASE: {
      fadeToBlackBy(frontLeds, NUM_FRONT, 40);
      fadeToBlackBy(backLeds,  NUM_BACK,  40);
      uint16_t div = map(p.speed, 0, 255, 80, 15);
      if (div < 1) div = 1;
      uint16_t step = now / div;
      frontLeds[step % NUM_FRONT] = front;
      backLeds[step % NUM_BACK]   = backA;
      backLeds[(step + NUM_BACK / 2) % NUM_BACK] = backB;
      break;
    }

    default:
      fill_solid(frontLeds, NUM_FRONT, front);
      break;
  }

  // Front/Back 個別マスク適用
  if (!(s.channelMask & CH_FRONT)) fill_solid(frontLeds, NUM_FRONT, CRGB::Black);
  if (!(s.channelMask & CH_BACK))  fill_solid(backLeds,  NUM_BACK,  CRGB::Black);
}

void ledsShow() {
  FastLED.show();
}
