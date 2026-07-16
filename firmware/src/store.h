// NVS(ESP32内蔵Flash)への状態保存/復元。単体運用で前回状態を復元するため。
#pragma once
#include "config.h"

void storeLoad(AppState& s);
void storeSave(const AppState& s);
