// NimBLE GATTサービス。
//  ① プリセット作成・転送：State/SlotSelect/PresetData/Command（散発R/W＋NVS）
//  ② リアルタイム制御：Realtimeキャラを予約（今は未処理スタブ）
#include <NimBLEDevice.h>
#include <string.h>
#include "config.h"
#include "app.h"
#include "presets.h"
#include "ble.h"

// カスタム128bit UUID（NuiLED専用）
#define UUID_SVC   "a1b20001-5c3d-4e6f-8a90-1234567890ab"
#define UUID_STATE "a1b20002-5c3d-4e6f-8a90-1234567890ab"
#define UUID_SLOT  "a1b20003-5c3d-4e6f-8a90-1234567890ab"
#define UUID_PDATA "a1b20004-5c3d-4e6f-8a90-1234567890ab"
#define UUID_CMD   "a1b20005-5c3d-4e6f-8a90-1234567890ab"
#define UUID_RT    "a1b20006-5c3d-4e6f-8a90-1234567890ab"

static NimBLECharacteristic* chState = nullptr;
static NimBLECharacteristic* chPData = nullptr;
static uint8_t selSlot = 0;   // SlotSelectで選択中の編集対象スロット

// State: [powerOn, brightnessIdx, activeSlot, channelMask, numSlots]
static void fillState(uint8_t* b) {
  b[0] = g_state.powerOn ? 1 : 0;
  b[1] = g_state.brightnessIndex;
  b[2] = g_state.activeSlot;
  b[3] = g_state.channelMask;
  b[4] = NUM_SLOTS;
}

// --- SlotSelect: 編集対象スロットの選択 ---
class SlotCB : public NimBLECharacteristicCallbacks {
  void onRead(NimBLECharacteristic* c) override { c->setValue(&selSlot, 1); }
  void onWrite(NimBLECharacteristic* c) override {
    NimBLEAttValue v = c->getValue();
    if (v.length() >= 1) {
      selSlot = v.data()[0];
      if (selSlot >= NUM_SLOTS) selSlot = 0;
      Preset p = presetsGet(selSlot);           // 選択に合わせてPresetDataも更新
      if (chPData) chPData->setValue((const uint8_t*)&p, sizeof(Preset));
    }
  }
};

// --- PresetData: 選択スロットのPreset(12byte)を読み書き ---
class PDataCB : public NimBLECharacteristicCallbacks {
  void onRead(NimBLECharacteristic* c) override {
    Preset p = presetsGet(selSlot);
    c->setValue((const uint8_t*)&p, sizeof(Preset));
  }
  void onWrite(NimBLECharacteristic* c) override {
    NimBLEAttValue v = c->getValue();
    if (v.length() >= sizeof(Preset)) {
      Preset p;
      memcpy(&p, v.data(), sizeof(Preset));
      presetsSet(selSlot, p);                   // RAM更新（保存はCMD SAVE）
    }
  }
};

// --- State: 現在状態の読み出し ---
class StateCB : public NimBLECharacteristicCallbacks {
  void onRead(NimBLECharacteristic* c) override {
    uint8_t b[5]; fillState(b); c->setValue(b, 5);
  }
};

// --- Command: 操作オペコード ---
class CmdCB : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* c) override {
    NimBLEAttValue v = c->getValue();
    if (v.length() < 1) return;
    const uint8_t* d = v.data();
    uint8_t op = d[0];
    uint8_t a1 = (v.length() >= 2) ? d[1] : 0;
    switch (op) {
      case 0x01: appSetActiveSlot(a1); break;                 // APPLY: スロット適用
      case 0x02: presetsSave(); break;                        // SAVE: 全スロットNVS保存
      case 0x03: presetsFactoryReset();                       // FACTORY: 工場出荷
                 appSetActiveSlot(g_state.activeSlot); break;
      case 0x05: appSetPower(a1 != 0); break;                 // POWER
      case 0x06: appSetBrightness(a1); break;                 // BRIGHTNESS idx
      case 0x07: appSetChannelMask(a1); break;                // CHANNEL mask
      default: break;
    }
  }
};

// --- Realtime: ②リアルタイム制御用の予約（今は未処理） ---
class RtCB : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* /*c*/) override {
    // TODO(段階3): 直接フレーム/色ストリームをここで受けてLEDへ反映
  }
};

void bleInit() {
  NimBLEDevice::init("NuiLED");
  NimBLEServer* srv = NimBLEDevice::createServer();
  NimBLEService* svc = srv->createService(UUID_SVC);

  chState = svc->createCharacteristic(UUID_STATE,
              NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
  chState->setCallbacks(new StateCB());
  { uint8_t b[5]; fillState(b); chState->setValue(b, 5); }

  NimBLECharacteristic* chSlot = svc->createCharacteristic(UUID_SLOT,
              NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  chSlot->setCallbacks(new SlotCB());
  chSlot->setValue(&selSlot, 1);

  chPData = svc->createCharacteristic(UUID_PDATA,
              NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
  chPData->setCallbacks(new PDataCB());
  { Preset p = presetsGet(selSlot); chPData->setValue((const uint8_t*)&p, sizeof(Preset)); }

  NimBLECharacteristic* chCmd = svc->createCharacteristic(UUID_CMD, NIMBLE_PROPERTY::WRITE);
  chCmd->setCallbacks(new CmdCB());

  NimBLECharacteristic* chRt = svc->createCharacteristic(UUID_RT, NIMBLE_PROPERTY::WRITE_NR);
  chRt->setCallbacks(new RtCB());

  svc->start();

  NimBLEAdvertising* adv = NimBLEDevice::getAdvertising();
  adv->addServiceUUID(UUID_SVC);
  adv->setScanResponse(true);
  adv->start();
}

void bleNotifyState() {
  if (!chState) return;
  uint8_t b[5]; fillState(b);
  chState->setValue(b, 5);
  chState->notify();
}
