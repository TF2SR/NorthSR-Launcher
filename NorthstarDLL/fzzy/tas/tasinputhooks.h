#pragma once
#include <Xinput.h>
#include "fzzy/ckf/inputhooks.h"

void TASProcessXInput(XINPUT_STATE*& pState);

bool TASProcessInput(uintptr_t& a, InputEventType_t& nType, int& nTick, ButtonCode_t& scanCode, ButtonCode_t& virtualCode, int& data3);

void TAS_FrameUpdate(double flCurrentTime, float flFrameTime);
