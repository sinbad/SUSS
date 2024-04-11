# Steve's UtilityAi SubSystem for Unreal Engine 5

**This is a work in progress, full documentation to come**

## Principles

[Introduction to Utility AI](https://www.gdcvault.com/play/1012410/Improving-AI-Decision-Modeling-Through)

## Definitions

* Context: A series of values defining a unique context in which a decision is made (e.g. with respect to target A, and ability B)
* Action: A decision the AI has made to do something in a particular context
* Query: A generator of context values. Queries produce a matrix of contexts of every combination of results
* Input: A floating point value, sourced from elsewhere in the system, within a given context (e.g. health, distance to target, ammo)
* Consideration: A calculation that takes an input, maps it to 0..1 via bookends, and applies a curve function to transform it to a score

