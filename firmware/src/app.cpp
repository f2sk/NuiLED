#include "config.h"
#include "app.h"
#include "presets.h"
#include "store.h"
#include "power.h"
#include "ble.h"

AppState g_state;

static void applyBrightness() {
  FastLED.setBrightness(BRIGHTNESS_LEVELS[g_state.brightnessIndex]);
}

void appInit() {
  presetsLoad();
  storeLoad(g_state);
  if (g_state.activeSlot >= NUM_SLOTS) g_state.activeSlot = 0;
  applyBrightness();
  powerSet(g_state.powerOn);   // 復元した電源状態を反映（ソフトスタートはHW側）
}

void appTogglePower() { appSetPower(!g_state.powerOn); }

void appSetPower(bool on) {
  g_state.powerOn = on;
  powerSet(on);
  storeSave(g_state);
  bleNotifyState();
}

void appCycleBrightness() {
  g_state.brightnessIndex = (g_state.brightnessIndex + 1) % NUM_BRIGHTNESS;
  applyBrightness();
  storeSave(g_state);
  bleNotifyState();
}

void appSetBrightness(uint8_t idx) {
  if (idx >= NUM_BRIGHTNESS) return;
  g_state.brightnessIndex = idx;
  applyBrightness();
  storeSave(g_state);
  bleNotifyState();
}

void appSetActiveSlot(uint8_t slot) {
  if (slot >= NUM_SLOTS) return;
  g_state.activeSlot = slot;
  // プリセットの想定チャンネルを反映（SW2長押しの一時上書きはリセット）
  uint8_t m = presetsGet(slot).flags & CH_ALL;
  g_state.channelMask = (m == 0) ? CH_ALL : m;
  storeSave(g_state);
  bleNotifyState();
}

void appNextPreset() { appSetActiveSlot((g_state.activeSlot + 1) % NUM_SLOTS); }

void appCycleChannel() { appSetChannelMask(nextChannelMask(g_state.channelMask)); }

void appSetChannelMask(uint8_t m) {
  if (m == 0 || m > CH_ALL) m = CH_ALL;
  g_state.channelMask = m;
  storeSave(g_state);
  bleNotifyState();
}

const Preset& appActivePreset() { return presetsGet(g_state.activeSlot); }
