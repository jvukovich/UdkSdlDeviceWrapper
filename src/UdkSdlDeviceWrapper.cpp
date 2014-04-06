#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <iostream>
#include <SDL.h>
#include <SDL_events.h>

const int HAT_CENTER = 1;
const int HAT_UP = 2;
const int HAT_RIGHT = 3;
const int HAT_DOWN = 4;
const int HAT_LEFT = 5;

const int BUTTON_UP = 1;
const int BUTTON_DOWN = 2;

SDL_Joystick* device = NULL;
SDL_Event event;

template<typename DataType>
struct TArray
{
	DataType* Data;

	int Num()
	{
		return ArrayNum;
	}

	void Reallocate(int newNum, bool compact = false)
	{
		ArrayNum = newNum;

		if (ArrayNum > ArrayMax || compact)
		{
			ArrayMax = ArrayNum;
			Data = (DataType*) (*ReallocFunctionPtr) (Data, ArrayMax * sizeof(DataType), 8);
		}
	}

	int ArrayNum;
	int ArrayMax;
};

struct SdlDeviceWrapper
{
	TArray<int> AxisData;
	TArray<int> HatData;
	TArray<int> ButtonData;
	TArray<int> BallDataForX;
	TArray<int> BallDataForY;
};

extern "C"
{
	typedef void* (*ReallocFunctionPtrType) (void* Original, unsigned int Count, unsigned int Alignment);

	ReallocFunctionPtrType ReallocFunctionPtr = NULL;

	struct FDLLBindInitData
	{
		int Version;
		ReallocFunctionPtrType ReallocFunctionPtr;
	};

	__declspec(dllexport) void DLLBindInit(FDLLBindInitData* InitData)
	{
		ReallocFunctionPtr = InitData->ReallocFunctionPtr;

		SDL_Init(SDL_INIT_JOYSTICK);
		atexit(SDL_Quit);
	}

	_declspec(dllexport) int GetNumberOfDevices()
	{
		return SDL_NumJoysticks();
	}

	_declspec(dllexport) wchar_t* GetDeviceName(int deviceIndex)
	{
		const char* deviceName = SDL_JoystickNameForIndex(deviceIndex);

		if (deviceName == NULL)
		{
			if (SDL_IsGameController(deviceIndex))
			{
				char* actualDeviceName = SDL_GameControllerMappingForGUID(SDL_JoystickGetDeviceGUID(deviceIndex));
				SDL_free(actualDeviceName);
				deviceName = actualDeviceName;
			}
			else
			{
				char* deviceNameAsGuid;
				SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(deviceIndex), deviceNameAsGuid, 64);
				deviceName = deviceNameAsGuid;
			}
		}

		if (deviceName == NULL)
			deviceName = "";

		size_t deviceNameLength = (strlen(deviceName) + 1);
		wchar_t* deviceNameStr = new wchar_t[deviceNameLength];
		mbstowcs_s(NULL, deviceNameStr, deviceNameLength, deviceName, _TRUNCATE);

		return deviceNameStr;
	}

	_declspec(dllexport) int InitDevice(int deviceIndex)
	{
		device = SDL_JoystickOpen(deviceIndex);
		return 1;
	}

	_declspec(dllexport) int GetAxisCount()
	{
		if (device == NULL)
			return 0;

		return SDL_JoystickNumAxes(device);
	}

	_declspec(dllexport) int GetHatCount()
	{
		if (device == NULL)
			return 0;

		return SDL_JoystickNumHats(device);
	}

	_declspec(dllexport) int GetButtonCount()
	{
		if (device == NULL)
			return 0;

		return SDL_JoystickNumButtons(device);
	}

	_declspec(dllexport) int GetBallCount()
	{
		if (device == NULL)
			return 0;

		return SDL_JoystickNumBalls(device);
	}

	_declspec(dllexport) void PollDevice(struct SdlDeviceWrapper* x)
	{
		if (device == NULL)
			return;

		int axisCount = SDL_JoystickNumAxes(device);
		int hatCount = SDL_JoystickNumHats(device);
		int buttonCount = SDL_JoystickNumButtons(device);
		int ballCount = SDL_JoystickNumBalls(device);

		if (axisCount > 0 && x->AxisData.ArrayMax == 0)
		{
			x->AxisData.Reallocate(axisCount + 1);

			for (int i = 0; i <= axisCount; i++)
				x->AxisData.Data[i] = 0;
		}

		if (hatCount > 0 && x->HatData.ArrayMax == 0)
		{
			x->HatData.Reallocate(hatCount + 1);

			for (int i = 0; i <= hatCount; i++)
				x->HatData.Data[i] = 0;
		}

		if (buttonCount > 0 && x->ButtonData.ArrayMax == 0)
		{
			x->ButtonData.Reallocate(buttonCount + 1);

			for (int i = 0; i <= buttonCount; i++)
				x->ButtonData.Data[i] = 0;
		}

		if (ballCount > 0 && x->BallDataForX.ArrayMax == 0 && x->BallDataForY.ArrayMax == 0)
		{
			x->BallDataForX.Reallocate(ballCount + 1);
			x->BallDataForY.Reallocate(ballCount + 1);

			for (int i = 0; i <= ballCount; i++)
			{
				x->BallDataForX.Data[i] = 0;
				x->BallDataForY.Data[i] = 0;
			}
		}

		SDL_JoystickUpdate(); // Note: This may not be necessary.

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_JOYAXISMOTION)
			{
				int axisIndex = event.jaxis.axis;
				int axisValue = event.jaxis.value;

				// Note: We set 0 to 1, since the array values default to 0. The UDK scripts should only read values that do not equal 0.
				// If we didn't handle 0, or ignore 0 on the UDK side, then the UDK script would be flooded with 0 values. This gets around the issue.
				if (x->AxisData.Data[axisIndex] != axisValue)
					x->AxisData.Data[axisIndex] = (axisValue == 0) ? 1 : axisValue;
			}

			if (event.type == SDL_JOYHATMOTION)
			{
				int hatIndex = event.jhat.hat;
				int hatEngaged = 0;

				if (event.jhat.value & SDL_HAT_UP)
				{
					x->HatData.Data[hatIndex] = HAT_UP;
					hatEngaged = 1;
				}

				if (event.jhat.value & SDL_HAT_RIGHT)
				{
					x->HatData.Data[hatIndex] = HAT_RIGHT;
					hatEngaged = 1;
				}

				if (event.jhat.value & SDL_HAT_DOWN)
				{
					x->HatData.Data[hatIndex] = HAT_DOWN;
					hatEngaged = 1;
				}

				if (event.jhat.value & SDL_HAT_LEFT)
				{
					x->HatData.Data[hatIndex] = HAT_LEFT;
					hatEngaged = 1;
				}

				if (hatEngaged == 0 && ((event.jhat.value & SDL_HAT_CENTERED) == SDL_HAT_CENTERED))
					x->HatData.Data[hatIndex] = HAT_CENTER;
			}

			if (event.type == SDL_JOYBUTTONDOWN)
				x->ButtonData.Data[event.jbutton.button] = BUTTON_DOWN;

			if (event.type == SDL_JOYBUTTONUP)
				x->ButtonData.Data[event.jbutton.button] = BUTTON_UP;

			if (event.type == SDL_JOYBALLMOTION)
			{
				int ballIndex = event.jball.ball;
				int ballXVal = event.jball.xrel;
				int ballYVal = event.jball.yrel;

				// Same deal as axis data.
				if (x->BallDataForX.Data[ballIndex] != ballXVal)
					x->BallDataForX.Data[ballIndex] = (ballXVal == 0) ? 1 : ballXVal;

				if (x->BallDataForY.Data[ballIndex] != ballYVal)
					x->BallDataForY.Data[ballIndex] = (ballYVal == 0) ? 1 : ballYVal;
			}
		}

		return;
	}

	_declspec(dllexport) void ReleaseDevice()
	{
		SDL_JoystickClose(device);
		SDL_Quit();
		return;
	}
}