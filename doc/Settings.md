# Settings

Under Project Settings > Plugins > SUSS you'll find these settings. 

![Settings](img/Settings.png)

## Base Configuration

### Classes & Class Paths

This applies to settings for "Action Classes", "Action Class Paths", "Input Providers",
"Input Provider Paths" and so on for each of the [pluggable types](MainClasses.md).

There are 3 ways to register pluggable classes:

1. Explicitly add custom classes to the "Classes" or "Providers" list. 
2. Add the parent content path to "Class Paths" / "Provider Paths", for Blueprints
    > Note: you'll need to make sure those Blueprints are always cooked since they're not
    > directly referenced, using Primary Asset Labels is a good way to do that.
3. [Manually register](MainClasses.md#usussgamesubsystem) via `USussGameSubsystem` 

This section of settings covers 1. and 2.

### Disabled Action Tags

Use this to temporarily disable Actions with these tags. This can be useful for 
testing since you can stop your agents from doing certain actions in order to make
other ones "win" for testing. Or you can disable actions which are not quite ready yet
so they can be in the codebase but not actually picked until you've sorted out the kinks.

## Optimisation

### Brain Update settings

See the [Brain Update](BrainUpdate.md) section for more details.

## Collision

### Line Of Sight Trace Channel

For utility functions associated with Line Of Sight such as the built-in input
`Suss.Input.Perception.Sight.LineOfSightToTarget`, which channel to test.
Defaults to Visibility but you might want to customise that, as I have in this case.

# See Also

* [Home](../README.md)
* [Main classes](doc/MainClasses.md)
* [Brain Config](BrainConfig.md)
* [Brain Update Tick](BrainUpdate.md)