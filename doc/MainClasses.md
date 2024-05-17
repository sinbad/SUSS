# Main Classes

Here's a quick tour of the main classes in SUSS.

## `USussBrainComponent`

This is the main component of SUSS, it's a subclass of `UBrainComponent` and should be 
created within your `AAIController`. For simplicity, SUSS provides a `ASussAIControllerBase`
which does this for you.

The brain component is passed to all systems within SUSS (Actions, Queries etc)
and is the primary place you access everything else from at runtime. From here you
can access the AI Controller, interact with [Perception](Perception.md), temporarily
mess with scoring and many other things.

The brain component periodically queues itself for an update depending on how close
the Pawn being controlled by it is to any player. See [Brain Updates](BrainUpdate.md) for more details.

## `ASussAIControllerBase`

This is a subclass of the core `AAiController` which automatically creates a
`USussBrainComponent` and so is the quickest way to get up and running. You can 
either use it directly or subclass it to automatically get an AI controller that
works with SUSS.

It also adds some helpful methods, such as extending the `SetFocus` behaviour to
'lead a target' based on their velocity, useful for ranged enemies.

## Pluggable classes
 
There are multiple base classes in SUSS that are the base
classes for the pluggable elements in SUSS, allowing you to supply your own implementations
of Actions, Inputs, Queries, Parameters.

All providers are identified by **Gameplay Tags**. This means when you want to use
one of them in an AI config, you reference the tag and not the class directly.

The pluggable classes are `USussAction`, `USussQueryProvider`, `USussInputProvider`,
and `USussParameterProvider` (parameters can be passed to queries and inputs to modify
their behaviour, and parameter providers let you automatically provide these instead of using literals).

You might wonder why considerations aren't pluggable; that's because they're entirely data-driven
in your [Brain Config](BrainConfig.md) and don't need any custom code.

## `USussGameSubsystem`

This is the system which is responsible for processing queued [brain updates](BrainUpdate.md), and providing
all the pluggable providers for inputs, queries, actions and everything else. 

You only need to interact with this if you're writing these providers in C++, 
and you want to register them in code rather than in [Settings](Settings.md). 
For that you'll see functions like `RegisterQueryProvider`. 

## Asset classes

The remainder of the important classes are to do with configuring the brain via
data, so that's covered in another section, [Brain Config](BrainConfig.md).


## Links:

* [Home](../README.md)
* [Settings](Settings.md)
* [Brain Config](BrainConfig.md)
* [Brain Update Tick](BrainUpdate.md)