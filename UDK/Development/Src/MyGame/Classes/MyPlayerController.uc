class MyPlayerController extends UTPlayerController config(MyPlayerInput) DLLBind(UdkSdlDeviceWrapper);

struct SdlDeviceWrapper {
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

dllimport final function int GetNumberOfDevices();
dllimport final function string GetDeviceName(int deviceIndex);
dllimport final function int InitDevice(int deviceIndex);
dllimport final function GetDevices(out SdlDeviceWrapper wrapper);
dllimport final function GetDeviceInputCounts(out SdlDeviceWrapper wrapper);
dllimport final function PollDevice(out SdlDeviceWrapper wrapper);
dllimport final function ReleaseDevice();

simulated Event PostBeginPlay()
{
	local SdlDeviceWrapper wrapper;
	local int numberOfDevices;
	local int deviceIndex;
	local string deviceName;

	Super.PostBeginPlay();

	deviceIndex = 0;
	numberOfDevices = GetNumberOfDevices();

	`log("====== # of devices ====== : " $ numberOfDevices);

	// Prints out a list of available devices.
	do
	{
		deviceName = GetDeviceName(deviceIndex);

		if (deviceName == "")
			break;

		`log("====== Device Name on " $ deviceIndex $ " ====== : " $ GetDeviceName(deviceIndex));

		deviceIndex = deviceIndex + 1;
	}
	until (deviceIndex == numberOfDevices);

	// Just grab the first device for now (testing).
	InitDevice(0);

	// InitDevice() must be invoked first. Populates wrapper.DeviceInputCounts.
	GetDeviceInputCounts(wrapper);

	// First index for axis count.
	if (wrapper.DeviceInputCounts[0] > 0)
	{
		AxisCount = wrapper.DeviceInputCounts[0];
		//`log("====== Axis Count ====== : " $ AxisCount);
	}

	// Second index for hat count.
	if (wrapper.DeviceInputCounts[1] > 0)
	{
		HatCount = wrapper.DeviceInputCounts[1];
		//`log("====== Hat Count ====== : " $ HatCount);
	}

	// Third index for button count.
	if (wrapper.DeviceInputCounts[2] > 0)
	{
		ButtonCount = wrapper.DeviceInputCounts[2];
		//`log("====== Button Count ====== : " $ ButtonCount);
	}

	// Fourth index for ball count.
	if (wrapper.DeviceInputCounts[3] > 0)
	{
		BallCount = wrapper.DeviceInputCounts[3];
		//`log("====== Ball Count ====== : " $ BallCount);
	}
}

simulated event PlayerTick(float DeltaTime)
{
	local SdlDeviceWrapper wrapper;
	local int axisIndex;
	local int hatIndex;
	local int buttonIndex;
	local int ballIndex;

	Super.PlayerTick(DeltaTime);

	SensitivityThrottle = (SensitivityThrottle + 1) % 2;

	if (SensitivityThrottle % 2 == 0)
	{
		PollDevice(wrapper);

		// Axis values will need user-based sensitivity and dead zone ranges applied (later).
		// We don't want an axis value of 0, because the array values (int) default to 0.
		// If the device reports 0, the wrapper overrides it to 1 (which is close enough, given the range of joystick axis values).
		// Why? Well, if this isn't done, the wrapper will output 0 non-stop, until an SDL event is triggered. We don't want that.
		// That's why we check to make sure the value != 0. We're stuck with integer types, and their default value (0). Not much choice.
		for (axisIndex = 0; axisIndex < AxisCount; axisIndex++)
		{
			//if (wrapper.AxisData[axisIndex] != 0)
				//`log("====== Axis " $ axisIndex $ " ======: " $ wrapper.AxisData[axisIndex]);
		}

		// HAT_CENTER = 1
		// HAT_UP = 2
		// HAT_RIGHT = 3
		// HAT_DOWN = 4
		// HAT_LEFT = 5
		// We don't want a hat value of 0, because the array values default to 0.
		for (hatIndex = 0; hatIndex < HatCount; hatIndex++)
		{
			if (wrapper.HatData[hatIndex] > 0)
				`log("====== Hat " $ hatIndex $ " ======: " $ wrapper.HatData[hatIndex]);
		}

		// BUTTON_UP = 1
		// BUTTON_DOWN = 2
		// Same as above. The array values default to 0.
		for (buttonIndex = 0; buttonIndex < ButtonCount; buttonIndex++)
		{
			if (wrapper.ButtonData[buttonIndex] > 0)
				`log("====== Button " $ buttonIndex $ " ======: " $ wrapper.ButtonData[buttonIndex]);
		}

		// Every X and Y index are for the same ball input.
		// Same as above. The array values default to 0.
		for (ballIndex = 0; ballIndex < BallCount; ballIndex++)
		{
			if (wrapper.BallDataForX[ballIndex] > 0)
				`log("====== Ball X " $ ballIndex $ " ======: " $ wrapper.BallDataForX[ballIndex]);

			if (wrapper.BallDataForY[ballIndex] > 0)
				`log("====== Ball Y " $ ballIndex $ " ======: " $ wrapper.BallDataForY[ballIndex]);
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