class MyPlayerInput extends UDKPlayerInput within MyPlayerController
config(Input);

event PlayerInput(float DeltaTime)
{
    super.PlayerInput(DeltaTime);

    // Not in use. I was messing around with it, and later rolled back my changes.
    // Leaving in place because I'm lazy. :)
}