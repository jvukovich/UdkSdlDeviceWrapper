#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <iostream>
#include <SDL.h>
#include <SDL_events.h>

int defaultDeviceIndex = -1;
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

struct SdlWrapper
{
	TArray<wchar_t*> Devices;
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

	_declspec(dllexport) void GetDevices(struct SdlWrapper* sdlWrapper)
	{
		sdlWrapper->DeviceData.Reallocate(0);

		int numDevices = SDL_NumJoysticks();

		sdlWrapper->Devices.Reallocate(numDevices);

		for (int i = 0; i < numDevices; i++)
		{ 
			const char* originalDeviceName = SDL_JoystickNameForIndex(i);

			size_t newsize = (strlen(originalDeviceName) + 1);
			wchar_t * deviceName = new wchar_t[newsize];
			size_t convertedChars = 0;
			mbstowcs_s(&convertedChars, deviceName, newsize, originalDeviceName, _TRUNCATE);

			sdlWrapper->Devices.Data[i] = deviceName;

			// meh, for reference...
			
			//if (SDL_IsGameController(i))
			//{
			//	char *mapping = SDL_GameControllerMappingForGUID(SDL_JoystickGetDeviceGUID(i));
			//	SDL_free(mapping);
			//}
			//else {
			//	char guid[64];
			//	SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(i), guid, sizeof (guid));
			//}

			//SDL_Joystick* deviceInfo = SDL_JoystickOpen(i);

			//if (deviceInfo != NULL)
			//{
			//	printf(" instance id: %d\n", SDL_JoystickInstanceID(deviceInfo));
			//	printf(" axes: %d\n", SDL_JoystickNumAxes(deviceInfo));
			//	printf(" hats: %d\n", SDL_JoystickNumHats(deviceInfo));
			//	printf(" buttons: %d\n", SDL_JoystickNumButtons(deviceInfo));
			//	printf(" trackballs: %d\n", SDL_JoystickNumBalls(deviceInfo)); // I've _never_ seen this non-zero, if anyone has lemme know!

			//	SDL_JoystickClose(deviceInfo);
			//}
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

	// This outputs an array<int> to UDK. If we can use a better structure, that would be awesome.
	// I couldn't find a better one that works with DLLBind(), and this is just a prototype anyway.
	_declspec(dllexport) void PollDevice(struct SdlWrapper* sdlWrapper)
	{
		sdlWrapper->DeviceData.Reallocate(17);

		// A thought: we could allocate sets of indices for specific joystick functions.
		// i.e. Supporting 6 joystick axies, we could allocate array[0] through array[5] JUST for axis data.
		// i.e. Supporting 16 buttons, we could allocate array[6] through array[38] (you need two data points per button, one for button "down" and one for button "up").
		// We could treat hats as buttons as well and allocate additional space to the array if needed.
		// SDL has joystick device detection and is able to determine the number of axis and buttons on a joystick.
		// We can simply map each type to the array as necessary.
		// It is then up to the user the bind the correct axis and buttons to their game configuration (via the settings interface, which we may also have to build a device integration for).
		// Since UDK handles XInput natively, we may only want to use this wrapper for DirectInput-only devices.
		// We may want to create an option for the user to select "XInput" vs "DirectInput" in the settings interface.

		// For the time being, I've just mapped my Sidewinder 3D Pro directly, purely for testing purposes.

		/*
		sdlWrapper->DeviceData.Data[0] = 0; // axis bank
		sdlWrapper->DeviceData.Data[1] = 0; // axis up/down
		sdlWrapper->DeviceData.Data[2] = 0; // axis twist
		sdlWrapper->DeviceData.Data[3] = 0; // axis throttle
		sdlWrapper->DeviceData.Data[4] = 0; // hat up
		sdlWrapper->DeviceData.Data[5] = 0; // hat down
		sdlWrapper->DeviceData.Data[6] = 0; // hat left
		sdlWrapper->DeviceData.Data[7] = 0; // hat right
		sdlWrapper->DeviceData.Data[8] = 0; // hat centered
		sdlWrapper->DeviceData.Data[9] = 0; // button 1 down
		sdlWrapper->DeviceData.Data[10] = 0; // button 1 up
		sdlWrapper->DeviceData.Data[11] = 0; // button 2 down
		sdlWrapper->DeviceData.Data[12] = 0; // button 2 up
		sdlWrapper->DeviceData.Data[13] = 0; // button 3 down
		sdlWrapper->DeviceData.Data[14] = 0; // button 4 up
		sdlWrapper->DeviceData.Data[15] = 0; // button 5 down
		sdlWrapper->DeviceData.Data[16] = 0; // button 5 up
		*/

		int numOfAxis = SDL_JoystickNumAxes(device);
		int numOfHats = SDL_JoystickNumHats(device);
		int numOfButtons = SDL_JoystickNumButtons(device);
		int numOfBalls = SDL_JoystickNumBalls(device);

		SDL_JoystickUpdate(); // TODO: this may not be necessary.

		// todo: make buttons 1 for down, 0 for up
		// todo: make hats 0 for centered; 1 for up; 2 for right; 3 for down; 4 for left
		// what if there is more than one hat?

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_JOYAXISMOTION)
			{
				for (int i = 0; i < numOfAxis; i++)
				{
					if (event.jaxis.axis == i && event.jaxis.value != sdlWrapper->DeviceData.Data[i])
						sdlWrapper->DeviceData.Data[i] = event.jaxis.value;

					// break up to the # of axis we're going to support
					if (i >= 5)
						break;
				}

				// meh, for reference...
				/*if (event.jaxis.axis == 0 && event.jaxis.value != sdlWrapper->DeviceData.Data[0])
					sdlWrapper->DeviceData.Data[0] = event.jaxis.value;
				if (event.jaxis.axis == 1 && event.jaxis.value != sdlWrapper->DeviceData.Data[1])
					sdlWrapper->DeviceData.Data[1] = event.jaxis.value;
				if (event.jaxis.axis == 2 && event.jaxis.value != sdlWrapper->DeviceData.Data[2])
					sdlWrapper->DeviceData.Data[2] = event.jaxis.value;
				if (event.jaxis.axis == 3 && event.jaxis.value != sdlWrapper->DeviceData.Data[3])
					sdlWrapper->DeviceData.Data[3] = event.jaxis.value;*/
			}

			if (event.type == SDL_JOYHATMOTION)
			{
				int hatEngaged = 0;

				// todo: too fucking tired right now, time for bed
				/*for (int i = 0; i < numOfHats; i++)
				{
				}*/

				if (event.jhat.value & SDL_HAT_UP)
				{
					sdlWrapper->DeviceData.Data[4] = 1;
					hatEngaged = 1;
				}

				if (event.jhat.value & SDL_HAT_DOWN)
				{
					sdlWrapper->DeviceData.Data[5] = 1;
					hatEngaged = 1;
				}

				if (event.jhat.value & SDL_HAT_LEFT)
				{
					sdlWrapper->DeviceData.Data[6] = 1;
					hatEngaged = 1;
				}

				if (event.jhat.value & SDL_HAT_RIGHT)
				{
					sdlWrapper->DeviceData.Data[7] = 1;
					hatEngaged = 1;
				}

				// Note this is a bit of a workaround, but it works just fine.
				if (hatEngaged == 0 && ((event.jhat.value & SDL_HAT_CENTERED) == SDL_HAT_CENTERED))
					sdlWrapper->DeviceData.Data[8] = 1;
			}

			// Hi. I am not a C++ programmer. We're just prototyping this crap. Thanks!

			if (event.type == SDL_JOYBUTTONDOWN)
			{
				if (event.jbutton.button == 0)
					sdlWrapper->DeviceData.Data[9] = 1; // button 1 down
				if (event.jbutton.button == 1)
					sdlWrapper->DeviceData.Data[11] = 1; // button 2 down
				if (event.jbutton.button == 2)
					sdlWrapper->DeviceData.Data[13] = 1; // button 3 down
				if (event.jbutton.button == 3)
					sdlWrapper->DeviceData.Data[15] = 1; // button 5 down
			}

			if (event.type == SDL_JOYBUTTONUP)
			{
				if (event.jbutton.button == 0)
					sdlWrapper->DeviceData.Data[10] = 1; // button 1 up
				if (event.jbutton.button == 1)
					sdlWrapper->DeviceData.Data[12] = 1; // button 2 up
				if (event.jbutton.button == 2)
					sdlWrapper->DeviceData.Data[14] = 1; // button 4 up
				if (event.jbutton.button == 3)
					sdlWrapper->DeviceData.Data[16] = 1; // button 5 up
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