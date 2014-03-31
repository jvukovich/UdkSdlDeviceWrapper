#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <SDL.h>
#include <SDL_events.h>

int joystick_instance = -1;
SDL_Joystick * joystick = NULL;
SDL_Event event;

template<typename DataType>
struct TArray
{
	DataType* Data;

	int Num()
	{
		return ArrayNum;
	}

	void Reallocate(int NewNum, bool bCompact = false)
	{
		ArrayNum = NewNum;

		if (ArrayNum > ArrayMax || bCompact)
		{
			ArrayMax = ArrayNum;
			Data = (DataType*) (*ReallocFunctionPtr) (Data, ArrayMax * sizeof(DataType), 8);
		}
	}

	private:
		int ArrayNum;
		int ArrayMax;
};

struct joystickDataArrayContainer
{
	TArray<int> joystickDataArray;
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

	_declspec(dllexport) int initJoystick()
	{
		SDL_Init(SDL_INIT_JOYSTICK);
		atexit(SDL_Quit);
		joystick = SDL_JoystickOpen(0);
		return 1;
	}

	// This outputs an array<int> to UDK. If we can use a better structure, that would be awesome.
	// I couldn't find a better one that works with DLLBind(), and this is just a prototype anyway.
	_declspec(dllexport) void pollJoystick(struct joystickDataArrayContainer* container)
	{
		container->joystickDataArray.Reallocate(17);

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

		container->joystickDataArray.Data[0] = 0; // axis bank
		container->joystickDataArray.Data[1] = 0; // axis up/down
		container->joystickDataArray.Data[2] = 0; // axis twist
		container->joystickDataArray.Data[3] = 0; // axis throttle
		container->joystickDataArray.Data[4] = 0; // hat up
		container->joystickDataArray.Data[5] = 0; // hat down
		container->joystickDataArray.Data[6] = 0; // hat left
		container->joystickDataArray.Data[7] = 0; // hat right
		container->joystickDataArray.Data[8] = 0; // hat centered
		container->joystickDataArray.Data[9] = 0; // button 1 down
		container->joystickDataArray.Data[10] = 0; // button 1 up
		container->joystickDataArray.Data[11] = 0; // button 2 down
		container->joystickDataArray.Data[12] = 0; // button 2 up
		container->joystickDataArray.Data[13] = 0; // button 3 down
		container->joystickDataArray.Data[14] = 0; // button 4 up
		container->joystickDataArray.Data[15] = 0; // button 5 down
		container->joystickDataArray.Data[16] = 0; // button 5 up

		SDL_JoystickUpdate(); // TODO: this may not be necessary.

		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_JOYAXISMOTION)
			{
				if (event.jaxis.axis == 0 && event.jaxis.value != container->joystickDataArray.Data[0])
					container->joystickDataArray.Data[0] = event.jaxis.value;
				if (event.jaxis.axis == 1 && event.jaxis.value != container->joystickDataArray.Data[1])
					container->joystickDataArray.Data[1] = event.jaxis.value;
				if (event.jaxis.axis == 2 && event.jaxis.value != container->joystickDataArray.Data[2])
					container->joystickDataArray.Data[2] = event.jaxis.value;
				if (event.jaxis.axis == 3 && event.jaxis.value != container->joystickDataArray.Data[3])
					container->joystickDataArray.Data[3] = event.jaxis.value;
			}

			if (event.type == SDL_JOYHATMOTION)
			{
				int hatEngaged = 0;

				if (event.jhat.value & SDL_HAT_UP)
				{
					container->joystickDataArray.Data[4] = 1;
					hatEngaged = 1;
				}

				if (event.jhat.value & SDL_HAT_DOWN)
				{
					container->joystickDataArray.Data[5] = 1;
					hatEngaged = 1;
				}

				if (event.jhat.value & SDL_HAT_LEFT)
				{
					container->joystickDataArray.Data[6] = 1;
					hatEngaged = 1;
				}

				if (event.jhat.value & SDL_HAT_RIGHT)
				{
					container->joystickDataArray.Data[7] = 1;
					hatEngaged = 1;
				}

				// Note this is a bit of a workaround, but it works just fine.
				if (hatEngaged == 0 && ((event.jhat.value & SDL_HAT_CENTERED) == SDL_HAT_CENTERED))
					container->joystickDataArray.Data[8] = 1;
			}

			if (event.type == SDL_JOYBUTTONDOWN)
			{
				if (event.jbutton.button == 0)
					container->joystickDataArray.Data[9] = 1; // button 1 down
				if (event.jbutton.button == 1)
					container->joystickDataArray.Data[11] = 1; // button 2 down
				if (event.jbutton.button == 2)
					container->joystickDataArray.Data[13] = 1; // button 3 down
				if (event.jbutton.button == 3)
					container->joystickDataArray.Data[15] = 1; // button 5 down
			}

			if (event.type == SDL_JOYBUTTONUP)
			{
				if (event.jbutton.button == 0)
					container->joystickDataArray.Data[10] = 1; // button 1 up
				if (event.jbutton.button == 1)
					container->joystickDataArray.Data[12] = 1; // button 2 up
				if (event.jbutton.button == 2)
					container->joystickDataArray.Data[14] = 1; // button 4 up
				if (event.jbutton.button == 3)
					container->joystickDataArray.Data[16] = 1; // button 5 up
			}
		}

		return;
	}

	// I don't know where we need to call this within UDK.
	_declspec(dllexport) void releaseJoystick()
	{
		SDL_JoystickClose(joystick);
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