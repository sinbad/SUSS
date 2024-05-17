# Perception

You'll almost always want to add a Perception component to your AI controllers
so that they can see, hear etc.

If you do so, SUSS comes with a number of helpful queries, inputs and utilities
that utilise this perception data.

Here's a taste, check the code for more:

| Tag | Type | Description | 
|--|--|--|
|`Suss.Query.Perception.Targets.HostilesKnown`|Query|Query all hostiles known to this agent's perception system. Optional param 'Sense' to filter ('Sight', 'Hearing' etc)|
|`Suss.Input.Perception.Sight.RangeSelf`| Input |Get the sight range of the agent|
|`Suss.Input.Perception.Sight.LineOfSightToTarget`|Input| if the agent has line of sight to the target context, 0 if not. Optional parameter 'Radius' to perform a sphere trace rather than a line trace.|


# See Also

* [Home](../README.md)
* [Main classes](doc/MainClasses.md)
* [Brain Config](BrainConfig.md)
* [Brain Update Tick](BrainUpdate.md)