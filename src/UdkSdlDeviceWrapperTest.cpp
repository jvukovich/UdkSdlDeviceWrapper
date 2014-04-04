#include <stdio.h>
#include <wchar.h>
#include <assert.h>
#include <iostream>
#include <SDL.h>
#include <SDL_events.h>

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_JOYSTICK);
	atexit(SDL_Quit);

	int numDevices = SDL_NumJoysticks();

	for (int i = 0; i < numDevices; i++)
	{
		const char* deviceName = SDL_JoystickNameForIndex(i);
		printf("Joystick index %d: %s\n", i, deviceName);
	}

	//SDL_Joystick* device = SDL_JoystickOpen(0);
	//SDL_Event event;

	//while (true)
	//{
	//	while (SDL_PollEvent(&event))
	//	{
	//		switch (event.type)
	//		{
	//			case SDL_JOYAXISMOTION:
	//				// 0 == bank
	//				// 1 == look up/down
	//				// 2 == twist
	//				// 3 == throttle
	//				printf("%d axis %d value %d\n", event.jaxis.which, event.jaxis.axis, event.jaxis.value);
	//				break;
	//			case SDL_JOYHATMOTION:
	//				printf("hat %d value 0x%02x\n", event.jhat.hat, event.jhat.value);
	//				if (event.jhat.value & SDL_HAT_UP)
	//					printf(" -- hat up");
	//				if (event.jhat.value & SDL_HAT_DOWN)
	//					printf(" -- hat down");
	//				if (event.jhat.value & SDL_HAT_LEFT)
	//					printf(" -- hat left");
	//				if (event.jhat.value & SDL_HAT_RIGHT)
	//					printf(" -- hat right");
	//				break;
	//			case SDL_JOYBUTTONDOWN:
	//				printf("button %d value %d down\n", event.jbutton.which, event.jbutton.button);
	//				break;
	//			case SDL_JOYBUTTONUP:
	//				printf("button %d value %d up\n", event.jbutton.which, event.jbutton.button);
	//				break;
	//			case SDL_JOYBALLMOTION:
	//				printf("ball %d motion: (%d, %d)\n", event.jball.ball, event.jball.xrel, event.jball.yrel);
	//				break;
	//			default:
	//				break;
	//		}

	//		break;
	//	}
	//}

	//SDL_JoystickClose(device);
	SDL_Quit();

	return 0;
}