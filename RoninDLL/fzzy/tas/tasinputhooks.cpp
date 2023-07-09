#include "tasinputhooks.h"
#include "fzzy/ckf/bindingshooks.h"
#include "fzzy/ckf/inputhooks.h"
#include "logging/sourceconsole.h"
#include "fzzy/speedmod.h"
#include <string>

using namespace std;

typedef void(__fastcall* FRICTIONHOOK)();
static FRICTIONHOOK hookedFriction = nullptr;

float* _velX;
float* _velY;
float* _velZ;
MemoryAddress _yaw;
float* _timescale;
bool* _onGround;
bool* _sprint;
ConVar* Cvar_fzzy_enableTas;

auto flipTimestamp = std::chrono::steady_clock::now();
bool flip;

bool forwardPressed;
bool backPressed;
bool rightPressed;
bool leftPressed;
bool crouchPressed;

bool lurchPressed;
bool bumpLaunchPressed;
bool surfStopPressed;

bool isLurching;

uintptr_t lastA;
int lastTick;

int spoofingJump;
int spoofingCrouch;

int spoofingForward;
int spoofingBack;
int spoofingRight;
int spoofingLeft;

float velocity;
float oldvelocity;

bool isBumpLaunching;

int pressJump;

bool doKick;

float* getYaw() {
	if (_yaw.Deref().m_nAddress == 0)
	{
		return 0;
	}
	float* addr = _yaw.Deref().Offset(0x1EAC).As<float*>();
	return addr;
}

void TAS_FrameUpdate(double flCurrentTime, float flFrameTime)
{
	if (Cvar_fzzy_enableTas != nullptr && !Cvar_fzzy_enableTas->GetBool())
		return;
	if (_timescale == 0)
		return;
	float timescale = *_timescale;
	if (timescale >= 0.9f)
		return;
	if (_velX == 0 || _velY == 0 || _velZ == 0)
		return;
	float velX = *_velX;
	float velY = *_velY;
	float velZ = *_velZ;

	float vel = (float)sqrtf(pow(velX, 2) + pow(velY, 2) + pow(velZ, 2));
	oldvelocity = velocity;
	velocity = vel;

	if (bumpLaunchPressed)
	{
		if (isBumpLaunching)
		{
			spoofPostEvent(IE_ButtonReleased, lastTick, (ButtonCode_t)jumpBinds[0], (ButtonCode_t)jumpBinds[0], 0);
			spoofPostEvent(IE_ButtonReleased, lastTick, (ButtonCode_t)backBinds[0], (ButtonCode_t)backBinds[0], 0);
			isBumpLaunching = false;
		}
		else
		{
			spoofPostEvent(IE_ButtonPressed, lastTick, (ButtonCode_t)jumpBinds[0], (ButtonCode_t)jumpBinds[0], 0);
			spoofPostEvent(IE_ButtonPressed, lastTick, (ButtonCode_t)backBinds[0], (ButtonCode_t)backBinds[0], 0);
			isBumpLaunching = true;
		}
	}
	else if (isBumpLaunching)
	{
		spoofPostEvent(IE_ButtonReleased, lastTick, (ButtonCode_t)jumpBinds[0], (ButtonCode_t)jumpBinds[0], 0);
		spoofPostEvent(IE_ButtonReleased, lastTick, (ButtonCode_t)backBinds[0], (ButtonCode_t)backBinds[0], 0);
		isBumpLaunching = false;
	}

	if (lurchPressed)
	{
		if (isLurching)
		{
			spoofPostEvent(IE_ButtonReleased, lastTick, (ButtonCode_t)forwardBinds[0], (ButtonCode_t)forwardBinds[0], 0);
			isLurching = false;
		}
		else
		{
			spoofPostEvent(IE_ButtonPressed, lastTick, (ButtonCode_t)forwardBinds[0], (ButtonCode_t)forwardBinds[0], 0);
			isLurching = true;
		}
	}
	else if (isLurching)
	{
		spoofPostEvent(IE_ButtonReleased, lastTick, (ButtonCode_t)forwardBinds[0], (ButtonCode_t)forwardBinds[0], 0);
		isLurching = false;
	}

	if (pressJump > 0)
	{
		pressJump--;
		if (pressJump == 1)
		{
			spoofPostEvent(IE_ButtonPressed, lastTick, (ButtonCode_t)jumpBinds[0], (ButtonCode_t)jumpBinds[0], 0);
			spoofingJump = 2;
		}
	}

	if (spoofingJump > 0)
	{
		spoofingJump--;
		if (spoofingJump == 0)
		{
			spoofPostEvent(IE_ButtonReleased, lastTick, (ButtonCode_t)jumpBinds[0], (ButtonCode_t)jumpBinds[0], 0);
		}
	}

	if (spoofingCrouch > 0)
	{
		spoofingCrouch--;
		if (spoofingCrouch == 0)
		{
			spoofPostEvent(IE_ButtonReleased, lastTick, (ButtonCode_t)crouchBinds[0], (ButtonCode_t)crouchBinds[0], 0);
		}
	}

	if (spoofingForward > 0)
	{
		spoofingForward--;
		if (spoofingForward == 0)
		{
			spoofPostEvent(IE_ButtonReleased, lastTick, (ButtonCode_t)forwardBinds[0], (ButtonCode_t)forwardBinds[0], 0);
		}
	}

	if (spoofingBack > 0)
	{
		spoofingBack--;
		if (spoofingBack == 0)
		{
			spoofPostEvent(IE_ButtonReleased, lastTick, (ButtonCode_t)backBinds[0], (ButtonCode_t)backBinds[0], 0);
		}
	}

	if (spoofingRight > 0)
	{
		spoofingRight--;
		if (spoofingRight == 0)
		{
			spoofPostEvent(IE_ButtonReleased, lastTick, (ButtonCode_t)rightBinds[0], (ButtonCode_t)rightBinds[0], 0);
		}
	}

	if (spoofingLeft > 0)
	{
		spoofingLeft--;
		if (spoofingLeft == 0)
		{
			spoofPostEvent(IE_ButtonReleased, lastTick, (ButtonCode_t)leftBinds[0], (ButtonCode_t)leftBinds[0], 0);
		}
	}

	if (doKick)
	{
		doKick = false;
		if (spoofingJump == 0)
		{
			spoofPostEvent(IE_ButtonPressed, lastTick, (ButtonCode_t)jumpBinds[0], (ButtonCode_t)jumpBinds[0], 0);
			spoofingJump = 1;
		}
		if (spoofingCrouch == 0 && !crouchPressed && velZ != 0)
		{
			spoofPostEvent(IE_ButtonPressed, lastTick, (ButtonCode_t)crouchBinds[0], (ButtonCode_t)crouchBinds[0], 0);
			spoofingCrouch = 1;
		}
	}
}

bool TASProcessInput(uintptr_t& a, InputEventType_t& nType, int& nTick, ButtonCode_t& scanCode, ButtonCode_t& virtualCode, int& data3)
{
	if (Cvar_fzzy_enableTas != nullptr && !Cvar_fzzy_enableTas->GetBool())
		return false;
	lastA = a;
	lastTick = nTick;
	ButtonCode_t key = scanCode;
	if (_timescale == 0)
		return false;
	float timescale = *_timescale;
	if (timescale >= 0.9f)
		return false;
	if (nType == IE_ButtonPressed)
	{
		if (key == jumpBinds[0] || key == jumpBinds[1])
		{
			pressJump = 2;
			return true;
		}
		if (key == tasBumpLaunchBinds[0] || key == tasBumpLaunchBinds[1])
		{
			bumpLaunchPressed = true;
			return true;
		}
		if (key == tasSurfStopBinds[0] || key == tasSurfStopBinds[1])
		{
			surfStopPressed = true;
			return true;
		}
		if (key == tasLurchBinds[0] || key == tasLurchBinds[1])
		{
			float velX = *_velX;
			float velY = *_velY;

			lurchPressed = true;
				float wishYaw = (float)((180 / 3.14159265f) * -atan2f(velX, velY)) + 90;

			if (getYaw() != 0)
			{
				*getYaw() = wishYaw;
			}

			return true;
		}
		if (key == forwardBinds[0] || key == forwardBinds[1])
		{
			forwardPressed = true;
			return true;
		}
		if (key == backBinds[0] || key == backBinds[1])
		{
			backPressed = true;
			return true;
		}
		if (key == rightBinds[0] || key == rightBinds[1])
		{
			rightPressed = true;
			return true;
		}
		if (key == leftBinds[0] || key == leftBinds[1])
		{
			leftPressed = true;
			return true;
		}

		if (key == crouchBinds[0] || key == crouchBinds[1])
			crouchPressed = true;
	}
	if (nType == IE_ButtonReleased)
	{
		if (key == tasSurfStopBinds[0] || key == tasSurfStopBinds[1])
		{
			surfStopPressed = false;
			return true;
		}
		if (key == tasBumpLaunchBinds[0] || key == tasBumpLaunchBinds[1])
		{
			bumpLaunchPressed = false;
			return true;
		}
		if (key == tasLurchBinds[0] || key == tasLurchBinds[1])
		{
			lurchPressed = false;
			return true;
		}
		if (key == forwardBinds[0] || key == forwardBinds[1])
		{
			forwardPressed = false;
			return true;
		}
		if (key == backBinds[0] || key == backBinds[1])
		{
			backPressed = false;
			return true;
		}
		if (key == rightBinds[0] || key == rightBinds[1])
		{
			rightPressed = false;
			return true;
		}
		if (key == leftBinds[0] || key == leftBinds[1])
		{
			leftPressed = false;
			return true;
		}
		if (key == crouchBinds[0] || key == crouchBinds[1])
			crouchPressed = false;
	}
	return false;
}

bool pressingLurch = false;

int packet = 0;

void TASProcessXInput(XINPUT_STATE*& pState)
{
	if (Cvar_fzzy_enableTas != nullptr && !Cvar_fzzy_enableTas->GetBool())
		return;
	if (_timescale == 0)
		return;
	if (_velX == 0 || _velY == 0 || _velZ == 0)
		return;
	float timescale = *_timescale;

	float velX = *_velX;
	float velY = *_velY;
	float velZ = *_velZ;
	float velocity = (float)sqrtf(pow(velX, 2) + pow(velY, 2) + pow(velZ, 2));

	pState->dwPacketNumber = packet;
	if (packet == 0)
		packet = 1;
	else
		packet = 0;

	pState->Gamepad.sThumbLY = (short)0;
	if (forwardPressed)
	{
		if (!backPressed)
		{
			pState->Gamepad.sThumbLY = (short)32767;
		}
	}
	if (backPressed)
	{
		if (!forwardPressed)
		{
			pState->Gamepad.sThumbLY = (short)-32767;
		}
	}

	pState->Gamepad.sThumbLX = (short)0;
	if (pressJump == 0)
	{
		if (rightPressed)
		{
			if (!leftPressed && !lurchPressed && !spoofingRight)
			{
				pState->Gamepad.sThumbLX = (short)32767;

				if (timescale < 1.0f && !forwardPressed && !backPressed)
				{
					float margin = 10.0f;
					float airspeed = 60.0f;
					float speedmodairspeed = 40.0f;
					if (velZ != 0)
					{
						float offset = (float)((180 / 3.14159265f) * abs(asinf(((IsSpeedmodEnabled() ? speedmodairspeed : airspeed) - margin) / velocity)));
						float wishYaw = (float)((180 / 3.14159265f) * -atan2f(velX, velY)) + 90 + offset;

						if (getYaw() != 0)
							*getYaw() = wishYaw;
					}
				}
			}
		}
		if (leftPressed)
		{
			if (!rightPressed && !lurchPressed && !spoofingLeft)
			{
				pState->Gamepad.sThumbLX = (short)-32767;

				if (timescale < 1.0f && !forwardPressed && !backPressed)
				{
					float margin = 10.0f;
					float airspeed = 60.0f;
					float speedmodairspeed = 40.0f;
					if (velZ != 0)
					{
						float offset =
							(float)((180 / 3.14159265f) * abs(asinf(((IsSpeedmodEnabled() ? speedmodairspeed : airspeed) - margin) / velocity)));
						float wishYaw = (float)((180 / 3.14159265f) * -atan2f(velX, velY)) + 90 - offset;

						if (getYaw() != 0)
							*getYaw() = wishYaw;
					}
				}
			}
		}
	}

	if (timescale < 1.0f && _sprint != 0 && _onGround != 0)
	{
		bool sprintPressed = *_sprint;
		if (velocity < oldvelocity - 0.1f && velocity > 300 && !sprintPressed)
		{
			bool onGround = *_onGround;
			if (onGround)
			{
				doKick = true;
			}
		}
		if (doKick)
		{
			pState->Gamepad.sThumbLX = (short)0;
			pState->Gamepad.sThumbLY = (short)32767;
		}
	}
	oldvelocity = velocity;
	/*if (!forwardPressed && !backPressed && !rightPressed && !leftPressed) {
		float velX = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C2C);
		float velY = *(float*)((uintptr_t)GetModuleHandle("client.dll") + 0xB34C30);
		float yaw = *(float*)((uintptr_t)GetModuleHandle("engine.dll") + 0x7B6668);
		float toDegrees = (180.0f / 3.14159265f);
		float toRadians = (3.14159265f / 180.0f);
		float magnitude = sqrt(pow(velX, 2) + pow(velY, 2));
		auto elapsed = chrono::steady_clock::now() - flipTimestamp;
		long long since = chrono::duration_cast<chrono::milliseconds>(elapsed).count();
		if (since > 100) {
			//flip = !flip;
		}
		if (magnitude > 100) {
			float airSpeed = 20;
			float velDegrees = atan2f(velY, velX) * toDegrees;
			float velDirection = yaw - velDegrees;
			float rightx = sinf((velDirection + 90) * toRadians);
			float righty = cosf((velDirection + 90) * toRadians);
			float offsetx = velX + rightx * airSpeed;
			float offsety = velY + righty * airSpeed;
			float angle = atan2f(offsety, offsetx) - atan2f(velY, velX);
			float offsetAngle = atan2f(righty, rightx) - angle;
			short tx = cosf(offsetAngle * toRadians) * 32767.0f;
			short ty = sinf(offsetAngle * toRadians) * 32767.0f;
			pState->Gamepad.sThumbLX = tx;
			pState->Gamepad.sThumbLY = ty;
		}
	}*/
}

ON_DLL_LOAD_CLIENT("client.dll", TASClientHooks, (CModule module))
{
	_velX = module.Offset(0xB34C2C).As<float*>();
	_velY = module.Offset(0xB34C30).As<float*>();
	_velZ = module.Offset(0xB34C34).As<float*>();
	_onGround = module.Offset(0x11EED78).As<bool*>();
	_yaw = module.Offset(0xB18748);

	Cvar_fzzy_enableTas = new ConVar("fzzy_enableTas", "0", FCVAR_NONE, "Controls whether TAS tools will override inputs");
}

ON_DLL_LOAD_CLIENT("engine.dll", TASEngineHooks, (CModule module))
{
	_timescale = module.Offset(0x1315A2C8).As<float*>();
	_sprint = module.Offset(0x1396CAB8).As<bool*>();
}
