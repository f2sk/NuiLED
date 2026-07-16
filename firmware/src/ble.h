// BLE(NimBLE) GATT。①プリセット作成・転送（散発R/W）を実装、②リアルタイムは予約スタブ。
#pragma once

void bleInit();
void bleNotifyState();   // 状態変化をStateキャラでNotify
