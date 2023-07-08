#include "speedmod.h"
#include "logging/sourceconsole.h"

uintptr_t _engineModule;

float* _speedmodVelX;
float* _speedmodVelY;
float* _lurchMax;
bool* _isSpeedmodEnabled;
int* _slideStepVelocityReduction;
float* _slideBoostCooldown;
int* _isInBT;

MemoryAddress _invincibilityPatch1;
MemoryAddress _invincibilityPatch2;

MemoryAddress _sideJumpPatch1;
MemoryAddress _sideJumpPatch2;

MemoryAddress _frictionPatch1;
MemoryAddress _frictionPatch2;

ConVar* Cvar_fzzy_enableSpeedmod;

uintptr_t ReadOffset(uintptr_t ptr, std::vector<unsigned int> offsets)
{
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i)
	{
		addr += offsets[i];
		if (i < offsets.size() - 1)
			addr = *reinterpret_cast<uintptr_t*>(addr);
		if (addr == 0)
			return 0;
	}
	return addr;
}

bool IsSpeedmodEnabled() {
	return *_isSpeedmodEnabled == 3;
}

void DisableSpeedmod()
{
	float* airAcceleration = reinterpret_cast<float*>(ReadOffset(_engineModule, {0x13084248, 0x2564}));
	if (airAcceleration == 0)
		return;
	*airAcceleration = 500.0f;
	float* airSpeed =
		reinterpret_cast<float*>(ReadOffset(_engineModule, {0x13084248, 0xEA8, 0x1008, 0x1038, 0x390, 0x48, 0x18, 0xA30, 0x10, 0x2218}));
	if (airSpeed == 0)
		return;
	*airSpeed = 60.0f;
	*_lurchMax = 0.7f;
	*_slideStepVelocityReduction = 10;
	*_slideBoostCooldown = 2.0f;

	_invincibilityPatch1.Patch("89 3B 48 8B 5C 24 30 48 83 C4 20 5F C3 CC CC CC CC CC CC CC CC CC");
	_invincibilityPatch2.Patch("74 15");

	_frictionPatch1.Patch("F3 0F 11 81 8C 00 00 00 F3 0F 59 89 90 00 00 00 F3 0F 11 89 90 00 00 00");
	_frictionPatch2.Patch("F3 0F 11 81 8C 00 00 00 F3 0F 59 89 90 00 00 00 F3 0F 11 89 90 00 00 00");

	_sideJumpPatch1.Patch("F3 0F 11 81 8C 00 00 00 48 8B 43 10 F3 44 0F 59 80 90 00 00 00 F3 44 0F 58 C7 F3 44 0F 11 80 90 00 00 00");
	_sideJumpPatch2.Patch("F3 0F 11 81 8C 00 00 00 48 8B 47 10 F3 44 0F 59 80 90 00 00 00 F3 44 0F 58 C7 F3 44 0F 11 80 90 00 00 00");
}

void Speedmod_FrameUpdate(double flCurrentTime, float flFrameTime)
{
	if (Cvar_fzzy_enableSpeedmod->GetBool())
	{
		float* airAcceleration = reinterpret_cast<float*>(ReadOffset(_engineModule, {0x13084248, 0x2564}));
		if (airAcceleration == 0)
			return;
		*airAcceleration = 10000.0f;
		float* airSpeed = reinterpret_cast<float*>(
			ReadOffset(_engineModule, {0x13084248, 0xEA8, 0x1008, 0x1038, 0x390, 0x48, 0x18, 0xA30, 0x10, 0x2218}));
		if (airSpeed == 0)
			return;
		*airSpeed = 40.0f;
		if (_lurchMax == 0)
			return;
		*_lurchMax = 0.0f;
		if (_slideStepVelocityReduction == 0)
			return;
		*_slideStepVelocityReduction = 0;
		if (_slideBoostCooldown == 0)
			return;
		*_slideBoostCooldown = 0.0f;

		_invincibilityPatch1.Patch("83 BB 10 01 00 00 03 74 02 89 3B 48 8B 5C 24 30 48 83 C4 20 5F C3");
		_invincibilityPatch2.Patch("74 1E");

		_frictionPatch1.Patch("90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90");
		_frictionPatch2.Patch("90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90");

		if (_isInBT == 0)
			return;
		if (_speedmodVelX == 0)
			return;
		if (_speedmodVelY == 0)
			return;
		float speedSquared = pow(*_speedmodVelX, 2) + pow(*_speedmodVelY, 2);
		if (*_isInBT != 2 && speedSquared > pow(181, 2))
		{
			_sideJumpPatch1.Patch(
				"90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90");
			_sideJumpPatch2.Patch(
				"90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90");
		}
		else
		{
			_sideJumpPatch1.Patch(
				"F3 0F 11 81 8C 00 00 00 48 8B 43 10 F3 44 0F 59 80 90 00 00 00 F3 44 0F 58 C7 F3 44 0F 11 80 90 00 00 00");
			_sideJumpPatch2.Patch(
				"F3 0F 11 81 8C 00 00 00 48 8B 47 10 F3 44 0F 59 80 90 00 00 00 F3 44 0F 58 C7 F3 44 0F 11 80 90 00 00 00");
		}
	}
	else
	{
		if (IsSpeedmodEnabled())
		{
			DisableSpeedmod();
		}
	}
}

ON_DLL_LOAD_CLIENT("engine.dll", SpeedmodEngineHooks, (CModule module))
{
	_engineModule = module.m_nAddress;

	Cvar_fzzy_enableSpeedmod = new ConVar("fzzy_enableSpeedmod", "0", FCVAR_NONE, "Controls whether Speedmod is enabled");
}

ON_DLL_LOAD_CLIENT("client.dll", SpeedmodClientHooks, (CModule module))
{
	_speedmodVelX = module.Offset(0xB34C2C).As<float*>();
	_speedmodVelY = module.Offset(0xB34C30).As<float*>();
	_lurchMax = module.Offset(0x11B0308).As<float*>();
	_slideStepVelocityReduction = module.Offset(0x11B0D28).As<int*>();
	_slideBoostCooldown = module.Offset(0x11B3AD8).As<float*>();
	_frictionPatch1 = module.Offset(0x20D6E5);
	_sideJumpPatch1 = module.Offset(0x201017);
	_isInBT = module.Offset(0x21720F0).As<int*>();
}

ON_DLL_LOAD_CLIENT("server.dll", SpeedmodServerHooks, (CModule module))
{
	_invincibilityPatch1 = module.Offset(0x43373A);
	_invincibilityPatch2 = module.Offset(0x433725);
	_frictionPatch2 = module.Offset(0x185D36);
	_sideJumpPatch2 = module.Offset(0x1737A6);

	_isSpeedmodEnabled = module.Offset(0x433740).As<bool*>();
}
