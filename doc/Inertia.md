# Inertia and Score Cooldown

It's important that agents don't change their mind too often. You don't want a
case where an agent flip-flops between chasing 2 different players because they're right on 
the boundary of which is closer.

## Inertia

Inertia is a score adjustment that can be added to future evaluations of an 
action (in a context) once you've decided to perform it once. This makes it less
likely that in future updates the decision changes. The inertia value reduces
over time so that eventually the action score is unmodified.

You can specify the inertia value for each [Action Def](Actions.md) in a [Brain Config](BrainConfig.md).

## Score Cooldown

Another source of indecision is if running an action is dependent on a condition
which then gets changed by running the action, meaning that subsequent evaluations
suddenly rule it out, and therefore that action is liable to be interrupted.

To make this clearer, imagine that we use the "Creature Mood" as an input value
for an action of "Alert". We want to execute the "Alert" action when the creature
is "Passive", and that involves playing an animation, making a sound, that kind of
thing. 

However, the Alert action probably also changes the creature's mood! Say it changes
it to "Alerted", but it takes a second or two to run its course (because of animations).
If that brain updates again before the animation is finished, the action is going
to score terribly because the creature is no longer "Passive", so it will get
interrupted and not finish the alert animation.

Now, you could set this Alert action to be non-interruptible. However, this is a 
blunt instrument; it's nice to be able to allow interruptions always, just to *prefer*
not to unless it's urgent. 

You coudl also delay changing the mood until the end of the action. That would
work but it's a bit unintuitive. 

SUSS helps you address this by having a "Score Cooldown". This means that when a decision is
made to run an action, the score (including inertia) is considered the *minimum score*
for that action, which then reduces over the cooldown time. So if that action
scores lower than the minimum over that time, it'll be bumped up so that it won't
get interrupted, until it reduces enough to be outbid. It just makes it easier
to avoid prevarication in your agents - you can set it to 0 if you don't want it, 
but generally it makes your agents behave more naturally.

* [Home](../README.md)
* [Main classes](doc/MainClasses.md)
* [Brain Config](BrainConfig.md)
* [Brain Update Tick](BrainUpdate.md)