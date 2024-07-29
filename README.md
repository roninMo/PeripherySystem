# Overview

The `Periphery System` is a component that helps with interacting with things within the player's periphery. It has built in logic you can adjust for finding items based on Objects/Channels, class references, and delegate functions for when you find different items for retrieval and other actions. It has built in networking for Client and Server or both, and debugging to make things easy. 

![Periphery System](/images/PeripherySystemImage.png)

It should save you a lot of time when you're trying to create things like highlighting other characters, keeping track of target lock characters, npc logic, quests and storing other objects and values for different logic, and plenty of other things you can think of

`Hopefully this saves you time and effort while you're developing things`




<br><br/>
# Setup

  - add the `PeripherySystemComponent` to your character
  - adjust the kinds of periphery you want to use, their detection, and check that you run the `InitPeripheryInformation()` or have set `bInitPeripheryDuringBeginPlay` to true
  - create the object periphery delegates for adding your own logic when a player finds an object




<br><br/>
# Tutorial 

## Add the Player Peripheries component to the character
The `PlayerPeripheriesComponent` needs to be added to character in order to use the periphery functions. Click on Add Components and search for "**Player Peripheries**"

![Periphery System](/images/PeripheryTutorial_1.png)



<br><br/>
## Adjust the settings of the periphery component
There's multiple peripheries you can use to create different functionality for multiple scenarios. Once you enable one of the peripheries, a dropdown opens with settings to configure each of the peripheries, and the physics object that actually handles the overlap logic.

![Periphery System](/images/PeripheryTutorial_2.png)

After you've configured the periphery settings, add delegates for the specific periphery to retrieve information when the player finds something within it's periphery.


<br><br/>
### Init periphery values during `BeginPlay`
The component automatically handles initializing the periphery components and binding the overlap events, but you might not want to activate this function during `BeginPlay`, so there's a value for this. If it isn't set to true, you'll just need to call the `InitPeripheryInformation()` function  before the periphery logic is activated

![Periphery System](/images/PeripheryTutorial_3.png)


<br><br/>
### Player Peripheries Activation Phases
The Player Peripheries component can run it's logic on client, server, or both, and it's determined by the `ActivationPhase` value in the details panel. Once you've enabled one of the periphery values, just choose one of the options

![Periphery System](/images/PeripheryTutorial_4.png)

<br><br/>
### Attach the periphery components to the character
Physics components that aren't handled in the constructor need to be attached to the proper components at `BeginPlay`. This is unique to each character you create, and you'll have to handle this on your own. The Periphery character blueprints have examples for how to handle different camera perspective logic, so if you need reference I've got you. 

![Periphery System](/images/PeripheryTutorial_5.png)




<br><br/>
## Periphery Function Callbacks
Once the Periphery component finds something, it'll call the appropriate callback function and let you handle everything else. Here's a list of the functions for having the player handle objects within the player's periphery.

![Periphery System](/images/PeripheryTutorial_7.png)
![Periphery System](/images/PeripheryTutorial_6.png)




<br><br/>
## Periphery Object Interface Functions
Additionally there's a PeripheryObjectInterface function for creating logic on the object when a player finds it within his periphery. Here's a list of the functions for this. There's also an example blueprint for how to create one

![Periphery System](/images/PeripheryTutorial_8.png)




<br><br/>
## Periphery Function List Reference
Here's a list of the periphery functions

#### Player Periphery Functions
![Periphery System](/images/PeripheryTutorial_6.png)


#### Periphery Interface Functions
![Periphery System](/images/PeripheryTutorial_12.png)




<br><br/>
## Debugging
Every periphery component has debugging which helps you know things it detected, so if you're having trouble finding things that are being detected check the collision settings (which are initialized during `InitPeripheryInformation()`, we just enable overlap events and adjust it to QueryOnly), and for reference here's the settings for making the periphery components visible in game

![Periphery System](/images/PeripheryTutorial_10.png)





<br><br/>
## Reference

Trace
  - ObjectTypes and class reference
  - Adjust the distance and trace offset, and whether it should ignore owner actors
  - debugging, trace debugging, and duration

Radius
  - Channel and class reference
  - Adjust the trace radius
  - debugging

Cone
  - Channel and class reference
  - Adjust the object and it's relative location, and reference the blueprint functions that moving this in constructor and during play
  - debugging




Periphery Delegates
  - Radius
    - ObjectInPlayerRadius
    - ObjectOutsideOfPlayerRadius

  - Trace
    - ObjectInPeripheryTrace
    - ObjectOutsideOfPeripheryTrace

  - Cone 
    - ObjectInPeripheryCone
    - ObjectOutsideOfPeripheryCone




Customizing the component
  - HandleLineTracePeriphery
  - Periphery IsValid functions
  - external references that handle the objects

