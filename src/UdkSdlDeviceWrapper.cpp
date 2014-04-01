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
	TArray<wchar_t> Devices;
	TArray<int> DeviceInputCounts;
	TArray<int> AxisData;
	TArray<int> HatData;
	TArray<int> ButtonData;
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

	// TODO: I can't get this to work yet. It always prints out an empty string in UDK.
	_declspec(dllexport) void GetDevices(struct SdlDeviceWrapper* x)
	{
		int numDevices = SDL_NumJoysticks();

		x->Devices.Reallocate(numDevices);

		for (int i = 0; i < numDevices; i++)
		{
			const char* deviceName = SDL_JoystickNameForIndex(i);

			if (deviceName == NULL)
			{
				if (SDL_IsGameController(i))
				{
					char* actualDeviceName = SDL_GameControllerMappingForGUID(SDL_JoystickGetDeviceGUID(i));
					SDL_free(actualDeviceName);
					deviceName = actualDeviceName;
				}
				else
				{
					char* deviceNameAsGuid;
					SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(i), deviceNameAsGuid, 64);
					deviceName = deviceNameAsGuid;
				}
			}

			if (deviceName != NULL)
			{
				size_t newsize = (strlen(deviceName) + 1);
				wchar_t* deviceNameAsString = new wchar_t[newsize];
				mbstowcs_s(NULL, deviceNameAsString, newsize, deviceName, _TRUNCATE);

				x->Devices.Data[i] = (wchar_t) deviceNameAsString;
			}
		}

		return;
	}

	_declspec(dllexport) int InitDevice(int deviceIndex)
	{
		device = SDL_JoystickOpen(deviceIndex);
		return 1;
	}

	_declspec(dllexport) void GetDeviceInputCounts(struct SdlDeviceWrapper* x)
	{
		x->DeviceInputCounts.Reallocate(3);
		x->DeviceInputCounts.Data[0] = SDL_JoystickNumAxes(device);
		x->DeviceInputCounts.Data[1] = SDL_JoystickNumHats(device);
		x->DeviceInputCounts.Data[2] = SDL_JoystickNumButtons(device);
		//x->DeviceInputCounts.Data[3] = SDL_JoystickNumBalls(device); // TODO: Not supported yet.
		return;
	}

	_declspec(dllexport) void PollDevice(struct SdlDeviceWrapper* x)
	{
		int axisCount = SDL_JoystickNumAxes(device);
		int hatCount = SDL_JoystickNumHats(device);
		int buttonCount = SDL_JoystickNumButtons(device);
		//int ballCount = SDL_JoystickNumBalls(device); // TODO: Not supported yet.

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

		SDL_JoystickUpdate(); // Note: This may not be necessary.

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_JOYAXISMOTION)
			{
				// Note: We set 0 to 1, since the array values default to 0. The UDK scripts should only read values that do not equal 0.
				// If we didn't handle 0, or ignore 0 on the UDK side, then the UDK script would be flooded with 0 values. This gets around the issue.
				for (int i = 0; i <= axisCount; i++)
					if (event.jaxis.axis == i && event.jaxis.value != x->AxisData.Data[i])
						x->AxisData.Data[i] = (event.jaxis.value == 0) ? 1 : event.jaxis.value;
			}

			if (event.type == SDL_JOYHATMOTION)
			{
				for (int i = 0; i <= hatCount; i++)
				{
					int hatIndex = event.jhat.hat;

					if (hatIndex != i)
						continue;

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
			}

			if (event.type == SDL_JOYBUTTONDOWN)
				x->ButtonData.Data[event.jbutton.button] = BUTTON_DOWN;

			if (event.type == SDL_JOYBUTTONUP)
				x->ButtonData.Data[event.jbutton.button] = BUTTON_UP;

			// TODO: Not supported yet.
			//if (event.type == SDL_JOYBALLMOTION)
			//{
				//int ballStartIndex = (buttonCount + 1);
				//x->DeviceData.Data[(event.jball.ball + ballStartIndex)] = event.jball.xrel;
				// event.jball.yrel
			//}
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

// This was my original code for testing output via a console application (exe).
// If we also want to support "ball" motion, we can do it (see below).
//int main(int argc, char *argv[])
//{
//	initJoystick();
//
//	while (true)
//	{
//		while (SDL_PollEvent(&event))
//		{
//			switch (event.type)
//			{
//				case SDL_JOYAXISMOTION:
//					// 0 == bank
//					// 1 == look up/down
//					// 2 == twist
//					// 3 == throttle
//					printf("%d axis %d value %d\n", event.jaxis.which, event.jaxis.axis, event.jaxis.value);
//					break;
//				case SDL_JOYHATMOTION:
//					printf("hat %d value 0x%02x\n", event.jhat.hat, event.jhat.value);
//					if (event.jhat.value & SDL_HAT_UP)
//						printf(" -- hat up");
//					if (event.jhat.value & SDL_HAT_DOWN)
//						printf(" -- hat down");
//					if (event.jhat.value & SDL_HAT_LEFT)
//						printf(" -- hat left");
//					if (event.jhat.value & SDL_HAT_RIGHT)
//						printf(" -- hat right");
//					break;
//				case SDL_JOYBUTTONDOWN:
//					printf("button %d value %d down\n", event.jbutton.which, event.jbutton.button);
//					break;
//				case SDL_JOYBUTTONUP:
//					printf("button %d value %d up\n", event.jbutton.which, event.jbutton.button);
//					break;
//				case SDL_JOYBALLMOTION:
//					printf("ball %d motion: (%d, %d)\n", event.jball.ball, event.jball.xrel, event.jball.yrel);
//					break;
//				default:
//					break;
//			}
//
//			break;
//		}
//	}
//
//	return 0;
//}