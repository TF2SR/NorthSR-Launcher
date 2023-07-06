#pragma once

struct ControllerInputHolder
{
	int buttonIndex;

	bool waitingToSend;
	long long timestamp;
};
