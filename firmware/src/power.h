// 母線EN（P-MOS）制御。突入抑制はハードのゲートRCが担うので、ここはEN GPIOのH/Lのみ。
#pragma once

void powerInit();
void powerSet(bool on);   // true=点灯（母線ON）, false=消灯（母線OFF）
