class MyPlayerController extends UTPlayerController config(MyPlayerInput) DLLBind(UdkSdlDeviceWrapper);

struct SdlDeviceWrapper {
	var array<string> Devices;
	var array<int> DeviceInputCounts;
	var array<int> AxisData;
	var array<int> HatData;
	var array<int> ButtonData;
	var array<int> BallDataForX;
	var array<int> BallDataForY;
};

var int AxisCount;
var int HatCount;
var int ButtonCount;
var int BallCount;
var int SensitivityThrottle;

dllimport final function int InitDevice(int deviceIndex);
dllimport final function GetDevices(out SdlDeviceWrapper wrapper);
dllimport final function GetDeviceInputCounts(out SdlDeviceWrapper wrapper);
dllimport final function PollDevice(out SdlDeviceWrapper wrapper);
dllimport final function ReleaseDevice();

simulated Event PostBeginPlay()
{
	local SdlDeviceWrapper wrapper;

	Super.PostBeginPlay();

	// Populates wrapper.Devices, by index, ascending. So, the first device listed is for device index 0. Second = device index 1. Third = device index 2. Etc.
	// TODO: Device names are not returning yet. Not sure if it's a problem with SDL, or C -> UDK casting issues. Need to do more research.
	//GetDevices(wrapper);

	// Just grab the first device for now (testing).
	InitDevice(0);

	// InitDevice() must be invoked first. Populates wrapper.DeviceInputCounts.
	GetDeviceInputCounts(wrapper);

	// First index for axis count.
	if (wrapper.DeviceInputCounts[0] > 0)
	{
		AxisCount = wrapper.DeviceInputCounts[0];
		`log("====== Axis Count ====== : " $ AxisCount);
	}

	// Second index for hat count.
	if (wrapper.DeviceInputCounts[1] > 0)
	{
		HatCount = wrapper.DeviceInputCounts[1];
		`log("====== Hat Count ====== : " $ HatCount);
	}

	// Third index for button count.
	if (wrapper.DeviceInputCounts[2] > 0)
	{
		ButtonCount = wrapper.DeviceInputCounts[2];
		`log("====== Button Count ====== : " $ ButtonCount);
	}

	// Fourth index for ball count.
	if (wrapper.DeviceInputCounts[3] > 0)
	{
		BallCount = wrapper.DeviceInputCounts[3];
		`log("====== Ball Count ====== : " $ BallCount);
	}
}

simulated event PlayerTick(float DeltaTime)
{
	local SdlDeviceWrapper wrapper;
	local int axisCounter;
	local int hatCounter;
	local int buttonCounter;
	local int ballCounter;

	Super.PlayerTick(DeltaTime);

	SensitivityThrottle = (SensitivityThrottle + 1) % 2;

	if (SensitivityThrottle % 2 == 0)
	{
		PollDevice(wrapper);

		// Axis values will need user-based sensitivity and dead zone ranges applied (later).
		// We don't want an axis value of 0, because the array values default to 0.
		// If the device reports 0, the wrapper overrides it to 1 (which is close enough, given the range of joystick axis values).
		for (axisCounter = 0; axisCounter < AxisCount; axisCounter++)
		{
			if (wrapper.AxisData[axisCounter] != 0)
				`log("====== Axis " $ axisCounter $ " ======: " $ wrapper.AxisData[axisCounter]);
		}

		// HAT_CENTER = 1
		// HAT_UP = 2
		// HAT_RIGHT = 3
		// HAT_DOWN = 4
		// HAT_LEFT = 5
		// We don't want a hat value of 0, because the array values default to 0.
		for (hatCounter = 0; hatCounter < HatCount; hatCounter++)
		{
			if (wrapper.HatData[hatCounter] > 0)
				`log("====== Hat " $ hatCounter $ " ======: " $ wrapper.HatData[hatCounter]);
		}

		// BUTTON_UP = 1
		// BUTTON_DOWN = 2
		// Same as hats. The array values default to 0.
		for (buttonCounter = 0; buttonCounter < ButtonCount; buttonCounter++)
		{
			if (wrapper.ButtonData[buttonCounter] > 0)
				`log("====== Button " $ buttonCounter $ " ======: " $ wrapper.ButtonData[buttonCounter]);
		}

		// Every X and Y index are for the same ball input.
		// Same as above. The array values default to 0.
		for (ballCounter = 0; ballCounter < BallCount; ballCounter++)
		{
			if (wrapper.BallDataForX[ballCounter] > 0)
				`log("====== Ball X " $ ballCounter $ " ======: " $ wrapper.BallDataForX[ballCounter]);

			if (wrapper.BallDataForY[ballCounter] > 0)
				`log("====== Ball Y " $ ballCounter $ " ======: " $ wrapper.BallDataForY[ballCounter]);
		}
	}
}

event Possess(Pawn inPawn, bool bVehicleTransition)
{
	Super.Possess(inPawn, bVehicleTransition);
	SetBehindView(true);
}

simulated event Destroyed()
{
	ReleaseDevice();
}

defaultproperties
{
	Name="Default__MyPlayerController"
	InputClass=Class'MyGame.MyPlayerInput'
	SensitivityThrottle = 0
}