#pragma once
#include <Windows.h>
#include <Xinput.h>
#include "fzzy/ckf/bindingshooks.h"
#include <vector>

enum InputEventType_t
{
	IE_ButtonPressed = 0, // m_nData contains a ButtonCode_t
	IE_ButtonReleased, // m_nData contains a ButtonCode_t
	IE_ButtonDoubleClicked, // m_nData contains a ButtonCode_t
	IE_AnalogValueChanged, // m_nData contains an AnalogCode_t, m_nData2 contains the value

	IE_FirstSystemEvent = 100,
	IE_Quit = IE_FirstSystemEvent,
	IE_ControllerInserted, // m_nData contains the controller ID
	IE_ControllerUnplugged, // m_nData contains the controller ID

	IE_FirstVguiEvent = 1000, // Assign ranges for other systems that post user events here
	IE_FirstAppEvent = 2000,
};

struct InputHolder
{
	uintptr_t a;
	InputEventType_t nType;
	int nTick;
	ButtonCode_t scanCode;
	ButtonCode_t virtualCode;
	int data3;
	int data4;

	bool waitingToSend;
	long long timestamp;
};

struct handle_data
{
	unsigned long process_id;
	HWND best_handle;
};

typedef unsigned int(__fastcall* POSTEVENT)(uintptr_t, InputEventType_t, int, ButtonCode_t, ButtonCode_t, int);

extern InputHolder jumpPressHolder;
extern InputHolder jumpReleaseHolder;
extern InputHolder crouchPressHolder;
extern InputHolder crouchReleaseHolder;

extern int framesSinceJump;
extern int superglideCrouchFrame;
extern POSTEVENT hookedPostEvent;

const long long CROUCHKICK_BUFFERING = 8;

void spoofPostEvent(InputEventType_t, int, ButtonCode_t, ButtonCode_t, int);
void CKF_FrameUpdate(double flCurrentTime, float flFrameTime);
