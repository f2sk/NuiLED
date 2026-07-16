// プリセットスロット管理（工場出荷値・NVS保存/復元・取得/設定）
#pragma once
#include "config.h"

void presetsLoad();                        // NVSから復元（無ければ工場出荷）
void presetsSave();                        // 全スロットをNVSへ保存
void presetsFactoryReset();                // 工場出荷値へ戻して保存
const Preset& presetsGet(uint8_t i);       // スロット取得
void presetsSet(uint8_t i, const Preset& p); // スロット更新（保存はしない）
