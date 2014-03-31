#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <iostream>
#include <SDL.h>
#include <SDL_events.h>

const int HAT_CENTER = 0;
const int HAT_UP = 1;
const int HAT_RIGHT = 2;
const int HAT_DOWN = 3;
const int HAT_LEFT = 4;

const int BUTTON_UP = 0;
const int BUTTON_DOWN = 1;

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

	private:
		int ArrayNum;
		int ArrayMax;
};

struct SdlDeviceWrapper
{
	TArray<wchar_t*> Devices;
	TArray<int> DeviceInputCounts;
	TArray<int> DeviceData;
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
	}

	_declspec(dllexport) void GetDevices(struct SdlDeviceWrapper* x)
	{
		int numDevices = SDL_NumJoysticks();

		x->Devices.Reallocate(numDevices);

		for (int i = 0; i < numDevices; i++)
		{ 
			const char* originalDeviceName = SDL_JoystickNameForIndex(i);

			size_t newsize = (strlen(originalDeviceName) + 1);
			wchar_t * deviceName = new wchar_t[newsize];
			size_t convertedChars = 0;
			mbstowcs_s(&convertedChars, deviceName, newsize, originalDeviceName, _TRUNCATE);

			x->Devices.Data[i] = deviceName;
		}

		return;
	}

	_declspec(dllexport) int InitDevice(int deviceIndex)
	{
		SDL_Init(SDL_INIT_JOYSTICK);
		atexit(SDL_Quit);
		device = SDL_JoystickOpen(deviceIndex);
		return 1;
	}

	_declspec(dllexport) void GetDeviceInputCounts(struct SdlDeviceWrapper* x)
	{
		x->DeviceInputCounts.Reallocate(3);
		x->DeviceInputCounts.Data[0] = SDL_JoystickNumAxes(device);
		x->DeviceInputCounts.Data[1] = SDL_JoystickNumHats(device);
		x->DeviceInputCounts.Data[2] = SDL_JoystickNumButtons(device);
		//x->DeviceInputCounts.Data[3] = SDL_JoystickNumBalls(device); // Not supported yet.
		return;
	}

	_declspec(dllexport) void PollDevice(struct SdlDeviceWrapper* x)
	{
		int axisCount = SDL_JoystickNumAxes(device);
		int hatCount = SDL_JoystickNumHats(device);
		int buttonCount = SDL_JoystickNumButtons(device);
		//int ballCount = SDL_JoystickNumBalls(device); // Not supported yet.

		int deviceInputCount = (axisCount + hatCount + buttonCount);
		
		x->DeviceData.Reallocate(deviceInputCount);

		for (int i = 0; i < deviceInputCount; i++)
			x->DeviceData.Data[i] = 0;

		int axisStartIndex = 0;
		int hatStartIndex = axisCount;
		int buttonStartIndex = hatCount;
		//int ballStartIndex = buttonCount;; // Not supported yet.

		//SDL_JoystickUpdate(); // This may not be necessary.

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_JOYAXISMOTION)
			{
				for (int i = axisStartIndex; i < axisCount; i++)
					if (event.jaxis.axis == i && event.jaxis.value != x->DeviceData.Data[i])
						x->DeviceData.Data[i] = event.jaxis.value;
			}

			if (event.type == SDL_JOYHATMOTION)
			{
				int hatIndex = (event.jhat.hat + hatStartIndex);
				int hatEngaged = 0;

				if (event.jhat.value & SDL_HAT_UP)
				{
					x->DeviceData.Data[hatIndex] = HAT_UP;
					hatEngaged = 1;
				}

				if (event.jhat.value & SDL_HAT_RIGHT)
				{
					x->DeviceData.Data[hatIndex] = HAT_RIGHT;
					hatEngaged = 1;
				}

				if (event.jhat.value & SDL_HAT_DOWN)
				{
					x->DeviceData.Data[hatIndex] = HAT_DOWN;
					hatEngaged = 1;
				}

				if (event.jhat.value & SDL_HAT_LEFT)
				{
					x->DeviceData.Data[hatIndex] = HAT_LEFT;
					hatEngaged = 1;
				}

				if (hatEngaged == 0 && ((event.jhat.value & SDL_HAT_CENTERED) == SDL_HAT_CENTERED))
					x->DeviceData.Data[hatIndex] = HAT_CENTER;
			}

			if (event.type == SDL_JOYBUTTONDOWN)
				x->DeviceData.Data[(event.jbutton.button + buttonStartIndex)] = BUTTON_DOWN;

			if (event.type == SDL_JOYBUTTONUP)
				x->DeviceData.Data[(event.jbutton.button + buttonStartIndex)] = BUTTON_UP;

			// Not supported yet. Need to decide how to index x and y values.
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