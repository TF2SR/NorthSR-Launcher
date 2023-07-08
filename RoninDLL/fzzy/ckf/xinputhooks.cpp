#include "fzzy/ckf/xinputhooks.h"
#include "fzzy/ckf/inputhooks.h"
#include "fzzy/tas/tasinputhooks.h"

ControllerInputHolder controllerJumpPressHolder;
ControllerInputHolder controllerCrouchPressHolder;
ControllerInputHolder controllerJumpReleaseHolder;
ControllerInputHolder controllerCrouchReleaseHolder;

bool controllerJumpWasPressed;
bool controllerCrouchWasPressed;

AUTOHOOK_INIT()

AUTOHOOK_PROCADDRESS(_XInputGetState, XInput1_4.dll, XInputGetState, DWORD, WINAPI, (DWORD dwUserIndex, XINPUT_STATE* pState))
{
	DWORD toReturn = _XInputGetState(dwUserIndex, pState);

	// if (SRMM_GetSetting(SRMM_TAS_MODE))
	TASProcessXInput(pState);

	// pState->Gamepad.sThumbLX = (short)32767;
	// m_sourceConsole->Print(("LY IMMEDIATE: " + to_string(pState->Gamepad.sThumbLY) + "\n").c_str());

	WORD og = pState->Gamepad.wButtons;
	// Convert wButtons to button code so i can match up binds
	int jumpButtonIndex = 0;
	int crouchButtonIndex = 0;
	for (int i = 0; i < 14; i++)
	{
		bool bDown = (pState->Gamepad.wButtons & (1 << i)) != 0;
		switch (i)
		{
		case 0:
			if (controllerJump == KEY_XBUTTON_UP)
				jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_UP)
				crouchButtonIndex = i;
			break;
		case 1:
			if (controllerJump == KEY_XBUTTON_DOWN)
				jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_DOWN)
				crouchButtonIndex = i;
			break;
		case 2:
			if (controllerJump == KEY_XBUTTON_LEFT)
				jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_LEFT)
				crouchButtonIndex = i;
			break;
		case 3:
			if (controllerJump == KEY_XBUTTON_RIGHT)
				jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_RIGHT)
				crouchButtonIndex = i;
			break;
		case 4:
			if (controllerJump == KEY_XBUTTON_START)
				jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_START)
				crouchButtonIndex = i;
			break;
		case 5:
			if (controllerJump == KEY_XBUTTON_BACK)
				jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_BACK)
				crouchButtonIndex = i;
			break;
		case 6:
			if (controllerJump == KEY_XBUTTON_STICK1)
				jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_STICK1)
				crouchButtonIndex = i;
			break;
		case 7:
			if (controllerJump == KEY_XBUTTON_STICK2)
				jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_STICK2)
				crouchButtonIndex = i;
			break;
		case 8:
			if (controllerJump == KEY_XBUTTON_LEFT_SHOULDER)
				jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_LEFT_SHOULDER)
				crouchButtonIndex = i;
			break;
		case 9:
			if (controllerJump == KEY_XBUTTON_RIGHT_SHOULDER)
				jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_RIGHT_SHOULDER)
				crouchButtonIndex = i;
			break;
		case 10:
			if (controllerJump == KEY_XBUTTON_A)
				jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_A)
				crouchButtonIndex = i;
			break;
		case 11:
			if (controllerJump == KEY_XBUTTON_B)
				jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_B)
				crouchButtonIndex = i;
			break;
		case 12:
			if (controllerJump == KEY_XBUTTON_X)
				jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_X)
				crouchButtonIndex = i;
			break;
		case 13:
			if (controllerJump == KEY_XBUTTON_Y)
				jumpButtonIndex = i;
			if (controllerCrouch == KEY_XBUTTON_Y)
				crouchButtonIndex = i;
			break;
		}
	}

	// Handle controller jump/crouch buffering
	bool jumpDown = (pState->Gamepad.wButtons & (1 << jumpButtonIndex)) != 0;
	bool crouchDown = (pState->Gamepad.wButtons & (1 << crouchButtonIndex)) != 0;

	// Jump Press
	if (!controllerJumpWasPressed && jumpDown)
	{
		if (controllerCrouchPressHolder.waitingToSend)
		{
			controllerCrouchPressHolder.waitingToSend = false;
			// playSound();
			struct timespec ts;
			timespec_get(&ts, TIME_UTC);
			long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
			long sinceCrouch = real - controllerCrouchPressHolder.timestamp;

			RN::log::FZZY->info(("crouchkick: " + std::to_string(sinceCrouch) + "ms CROUCH IS EARLY").c_str());
			// m_sourceConsole->Print(("crouchkick: " + to_string(sinceCrouch) + "\n").c_str());
		}
		else
		{
			controllerJumpPressHolder.waitingToSend = true;
			struct timespec ts;
			timespec_get(&ts, TIME_UTC);
			long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
			controllerJumpPressHolder.timestamp = real;
		}
	}
	// Jump Release
	if (controllerJumpWasPressed && !jumpDown)
	{
		controllerJumpReleaseHolder.waitingToSend = true;
		struct timespec ts;
		timespec_get(&ts, TIME_UTC);
		long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
		controllerJumpReleaseHolder.timestamp = real;
	}

	// Crouch Press
	if (!controllerCrouchWasPressed && crouchDown)
	{
		if (controllerJumpPressHolder.waitingToSend)
		{
			controllerJumpPressHolder.waitingToSend = false;
			// playSound();
			struct timespec ts;
			timespec_get(&ts, TIME_UTC);
			long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
			long sinceJump = real - controllerJumpPressHolder.timestamp;

			RN::log::FZZY->info(("crouchkick: " + std::to_string(sinceJump) + "ms JUMP IS EARLY").c_str());
			// m_sourceConsole->Print(("crouchkick: " + to_string(sinceJump) + "\n").c_str());
		}
		else
		{
			controllerCrouchPressHolder.waitingToSend = true;
			struct timespec ts;
			timespec_get(&ts, TIME_UTC);
			long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
			controllerCrouchPressHolder.timestamp = real;
		}
	}
	// Crouch Release
	if (controllerCrouchWasPressed && !crouchDown)
	{
		controllerCrouchReleaseHolder.waitingToSend = true;
		struct timespec ts;
		timespec_get(&ts, TIME_UTC);
		long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
		controllerCrouchReleaseHolder.timestamp = real;
	}

	if (controllerJumpReleaseHolder.waitingToSend)
	{
		struct timespec ts;
		timespec_get(&ts, TIME_UTC);
		long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
		long sinceJump = real - controllerJumpReleaseHolder.timestamp;

		if (sinceJump > CROUCHKICK_BUFFERING)
		{
			controllerJumpReleaseHolder.waitingToSend = false;
		}
		else
		{
			pState->Gamepad.wButtons = pState->Gamepad.wButtons | (1 << jumpButtonIndex);
		}
	}
	if (controllerCrouchReleaseHolder.waitingToSend)
	{
		struct timespec ts;
		timespec_get(&ts, TIME_UTC);
		long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
		long sinceCrouch = real - controllerCrouchReleaseHolder.timestamp;

		if (sinceCrouch > CROUCHKICK_BUFFERING)
		{
			controllerCrouchReleaseHolder.waitingToSend = false;
		}
		else
		{
			pState->Gamepad.wButtons = pState->Gamepad.wButtons | (1 << crouchButtonIndex);
		}
	}

	if (controllerJumpPressHolder.waitingToSend)
	{
		struct timespec ts;
		timespec_get(&ts, TIME_UTC);
		long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
		long sinceJump = real - controllerJumpPressHolder.timestamp;

		if (sinceJump > CROUCHKICK_BUFFERING)
		{
			controllerJumpPressHolder.waitingToSend = false;
		}
		else
		{
			pState->Gamepad.wButtons = pState->Gamepad.wButtons & ~(1 << jumpButtonIndex);
		}
	}
	if (controllerCrouchPressHolder.waitingToSend)
	{
		struct timespec ts;
		timespec_get(&ts, TIME_UTC);
		long long real = (ts.tv_nsec / 1000000) + (ts.tv_sec * 1000);
		long sinceCrouch = real - controllerCrouchPressHolder.timestamp;

		if (sinceCrouch > CROUCHKICK_BUFFERING)
		{
			controllerCrouchPressHolder.waitingToSend = false;
		}
		else
		{
			pState->Gamepad.wButtons = pState->Gamepad.wButtons & ~(1 << crouchButtonIndex);
		}
	}

	controllerJumpWasPressed = jumpDown;
	controllerCrouchWasPressed = crouchDown;

	// m_sourceConsole->Print(("LX END: " + to_string(pState->Gamepad.sThumbLX) + "\n").c_str());

	return toReturn;
}

ON_DLL_LOAD_CLIENT("XInput1_4.dll", XInputHooks, (CModule module))
{
	AUTOHOOK_DISPATCH()
}
