class MyPlayerController extends UTPlayerController
config(MyPlayerInput)
DLLBind(SdofJoystick);

struct joystickDataArrayContainer {
	var array<int> joystickDataArray;
};

var int sensitivityThrottle;

dllimport final function int initJoystick();
dllimport final function pollJoystick(out joystickDataArrayContainer container);
dllimport final function releaseJoystick();

simulated Event PostBeginPlay() {
	super.PostBeginPlay();
	initJoystick();
}

simulated event PlayerTick(float DeltaTime)
{
	local joystickDataArrayContainer container;

	super.PlayerTick(DeltaTime);

	sensitivityThrottle = (sensitivityThrottle + 1) % 2;

	if (sensitivityThrottle % 2 == 0)
	{
		pollJoystick(container);

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

		// axis bank
		if (container.joystickDataArray[0] > 0)
			`log("== joystick ==: axisBank " $ container.joystickDataArray[0]);

		// axis up/down
		if (container.joystickDataArray[1] > 0)
			`log("== joystick ==: axisUpDown " $ container.joystickDataArray[1]);

		// axis twist
		if (container.joystickDataArray[2] > 0)
			`log("== joystick ==: axisTwist " $ container.joystickDataArray[2]);

		// axis throttle
		if (container.joystickDataArray[3] > 0)
			`log("== joystick ==: axisThrottle " $ container.joystickDataArray[3]);

		if (container.joystickDataArray[4] > 0)
		{
			`log("== joystick ==: hat up");

			// Simple integration test: would be in PlayerInput
			//aLookUp += 20;
		}

		if (container.joystickDataArray[5] > 0)
		{
			`log("== joystick ==: hat down");

			// Simple integration test: would be in PlayerInput
			//aLookUp -= 20;
		}

		if (container.joystickDataArray[6] > 0)
			`log("== joystick ==: hat left");

		if (container.joystickDataArray[7] > 0)
			`log("== joystick ==: hat right");

		if (container.joystickDataArray[8] > 0)
			`log("== joystick ==: hat centered");

		if (container.joystickDataArray[9] > 0)
			`log("== joystick ==: button 1 down");

		if (container.joystickDataArray[10] > 0)
			`log("== joystick ==: button 1 up");

		if (container.joystickDataArray[11] > 0)
			`log("== joystick ==: button 2 down");

		if (container.joystickDataArray[12] > 0)
			`log("== joystick ==: button 2 up");

		if (container.joystickDataArray[13] > 0)
			`log("== joystick ==: button 3 down");

		if (container.joystickDataArray[14] > 0)
			`log("== joystick ==: button 3 up");

		if (container.joystickDataArray[15] > 0)
			`log("== joystick ==: button 4 down");

		if (container.joystickDataArray[16] > 0)
			`log("== joystick ==: button 4 up");
	}
}

event Possess(Pawn inPawn, bool bVehicleTransition)
{
	Super.Possess(inPawn, bVehicleTransition);
	SetBehindView(true);
}

simulated event Destroyed()
{
	releaseJoystick(); // Not sure if this is the right place for this.
}

defaultproperties
{
	Name="Default__MyPlayerController"
	InputClass=Class'MyGame.MyPlayerInput'
	sensitivityThrottle = 0
}