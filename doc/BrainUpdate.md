# The Brain Update Process

Obviously in order for your AI agents to make decisions, they need a periodic update.
But, this doesn't have to happen all the time, and indeed the frequency that it
occurs should vary depending on how important the agents are.

So, brain components periodically request that they get an update. The frequency
at which that occurs and quickly it happens can vary. These are mostly controlled
by the values in [Settings](Settings.md).

## Brain Update Frequency

Brains always request that they get an update on a fixed tick period. How frequent
that is mainly depends on how close they are to any player. In [Settings](Settings.md)
there are update request intervals rates for Near, Medium and Far ranges. Anything
beyond "Far" is considered "Out of Bounds" (see below).

In addition, brains can receive early updates if their [perception](Perception.md) 
triggers a change (does not apply when they're out of bounds).

### Out Of Bounds Agents

Agents outside the "Far" range will *never* request an update. If they were running
an action and that had ongoing behaviour they can keep doing it to completion, but they'll never
change their mind and if the action completes they won't do anything further.

The "Out Of Bounds Distance Check Interval" determines how
often the brain checks to see if it's now within a range that it should get an update.


> A brain will also not update if the agent has any of the tags defined in `PreventBrainUpdateIfAnyTags`
> in the [Brain Config](BrainConfig.md); which you can use to stop the brain updating on 
> stun / freeze states

## Overall frame budget

The main hard limiter for the AI update process is the "Brain Update Frame Time Budget Milliseconds"
setting. Brains are queued for update periodically, and every frame that queue
is processed. but as soon as the amount of (real) time taken equals or exceeds
this frame budget (default 0.5ms, or about 3% of a 60Hz frame), then further 
brain updates are deferred until the next frame. 

You can use this value to limit how much time the AI process can take from your
frame budget, heading off spikes.

## What Happens When A Brain Updates

If an action is already running and is *not* interruptible, we abandon the update
and continue to let it run.

All Action Defs in the [Brain Config](BrainConfig.md) are combined into one big 
list, ordered by [Priority Group](Actions.md#priority-group).

For each priority group starting with the highest priority (lowest number), we run [queries](Actions.md#queries) and 
generate [contexts](Contexts.md) for each action, and score 
each using [considerations](Actions.md#considerations). If the action (and context)
matches the currently running action, there could be [inertia](Inertia.md) applied.

If any of the action scores come out as non-zero, then an action is picked from 
that priority group (based on the action choice method e.g. Highest Score) and none of the 
lower priority groups are evaluated.

That action is then chosen to run. If another different action is already running, it is
cancelled. If the one running is the same action (and same context) as has been picked again, then
the action is allowed to continue (and is told of this).



# See Also

* [Home](../README.md)
* [Main classes](doc/MainClasses.md)
* [Settings](Settings.md)
* [Brain Config](BrainConfig.md)

