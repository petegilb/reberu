<p align="center">
  <a href="" rel="noopener">
 <img width=200px height=200px src="https://i.imgur.com/G3yTINe.png" alt="Project logo"></a>
</p>

<div align="center">

  ## Reberu, A Dungeon-like Level Generator for Unreal Engine!

  [![Status](https://img.shields.io/badge/status-active-success.svg)]()
  [![GitHub Issues](https://img.shields.io/github/issues/petegilb/reberu.svg)](https://github.com/petegilb/reberu/issues)
  [![GitHub Pull Requests](https://img.shields.io/github/issues-pr/petegilb/reberu.svg)](https://github.com/petegilb/reberu/pulls)
  [![License](https://img.shields.io/badge/license-MIT-blue.svg)](/LICENSE)

</div>

<div id="table_of_contents" align="center">

### Table of Contents
[Showcase](#showcase)
[About](#about)
[Features](#features)
[Getting Started](#getting-started)
[Core Concepts](#core-concepts)
[Configuration](#configuration)
[Extending the Plugin](#extending-the-plugin)
[Credits](#credits)

</div>

<div id="showcase" align="center">

  ### Showcase

  [![](https://i.imgur.com/mUzxB0v.gif)]()

</div>

<div id="about" align="center">

  ### About

  <p>
  Reberu (レベル) aka Level is a simple, extendable plugin for Unreal Engine 5 that procedurally generates dungeon-like levels by stitching together pre-made rooms based on collision detection and door locations. Users define "rooms" with a bounding box and doors; the generator handles placement, connection, and backtracking automatically. All major generation steps are overridable in C++ or Blueprint, allowing for custom rules and fully custom level layouts.
  </p>

</div>

---

<div id="features">

### Features

- **Room-based procedural generation** — place pre-made UE5 levels as rooms and let Reberu connect them automatically
- **Door system with Gameplay Tags** — tag doors to control which rooms can connect to each other
- **Backtracking** — the generator backtracks when it gets stuck, with configurable backtrack strategies
- **Configurable room selection** — choose between Breadth, Depth, Random, or custom room selection methods
- **Custom rules** — create Blueprint or C++ `UReberuRule` subclasses to control whether any two rooms can connect
- **Door & blocked door actors** — specify per-tag actor classes to spawn at open and blocked doorways
- **Editor tooling** — `BP_RoomBounds` actor with in-editor gizmos for visually placing and editing doors
- **Replication support** — spawned room levels are replicated to clients via `OnRep_SpawnedRoomLevels`
- **Blueprint-exposed hooks** — `K2_StartGeneration`, `K2_PostProcessing`, and `OnGenerationCompleted` delegate for easy BP integration

</div>

---

<div id="getting-started">

### Getting Started

1. **Install the plugin** — copy the `Reberu` folder into your project's `Plugins/` directory and enable it in the Plugins menu.

2. **Create rooms** — open or create a level for each room. In the room level, place a `BP_RoomBounds` actor. Use the **DoorEditor** panel on the actor to:
   - Set the bounding box size (used for collision checks during generation)
   - Add doors with `Create New Door` and position them using the in-editor gizmo
   - Assign Gameplay Tags to doors to control connectivity

3. **Create a `UReberuRoomData` data asset** — for each room, create a `ReberuRoomData` data asset and point it to the room's level. The room bounds and door data are saved here.

4. **Create a `UReberuData` data asset** — add your room data assets to the `ReberuRooms` array. Assign a `StartingRoom`, set `TargetRoomAmount`, `MinRoomAmount`, and configure selection/backtrack methods.

5. **Place a `LevelGeneratorActor`** — add the actor to your main level. Assign your `ReberuData` asset and optionally enable `bStartOnBeginPlay`. Call `StartGeneration()` from Blueprint or C++ to kick off generation.

</div>

---

<div id="core-concepts">

### Core Concepts

#### `LevelGeneratorActor`
The main actor responsible for orchestrating generation. It maintains a doubly-linked list of `FReberuMove` nodes (one per placed room) which enables efficient backtracking. Key overridable methods:

| Method | Description |
|---|---|
| `ChooseSourceRoom` | Picks which already-placed room to extend from |
| `ChooseTargetRoom` | Picks which room type to place next |
| `ChooseSourceDoor` | Filters available doors on the source room |
| `ChooseTargetDoor` | Filters available doors on the candidate room |
| `PreProcessing` | Runs before finalization; return `false` to abort |
| `PostProcessing` | Runs after generation completes |
| `BacktrackSourceRoom` | Handles reverting moves when generation is stuck |

#### `RoomBounds` (`BP_RoomBounds`)
An editor-only actor used to define a room's bounding box and door positions. Use the **DoorEditor** category to create, edit, and delete doors. Door data is serialized into a `UReberuRoomData` data asset.

#### `ReberuData`
A primary data asset that configures an entire generation run:
- List of available rooms and the designated starting room
- Target and minimum room counts
- Room selection method (`Breadth`, `Depth`, `Random`, or custom)
- Backtrack method (`FromTail`, `UntilCurrent`, `None`, or custom)
- Max backtrack tries before generation fails
- Door actor map (keyed by Gameplay Tag) for open and blocked doorways

#### `ReberuRoomData`
A primary data asset representing a single room. Stores the level reference, bounding box transform/extent, doors (`FReberuDoor` array), room tags, and whether the room can connect to itself.

#### `FReberuDoor`
Represents a door on a room. Each door has:
- A unique auto-generated ID
- A world-space transform
- A box extent (for overlap checks)
- An optional Gameplay Tag
- A flag `bOnlyConnectSameDoor` to restrict connections to matching tags

#### `UReberuRule`
An abstract Blueprint-able UObject. Override `ShouldPlaceRoom(OwningRoom, ConnectingRoom)` to implement custom placement logic (e.g., prevent two boss rooms from being adjacent).

</div>

---

<div id="configuration">

### Configuration

`UReberuData` exposes the following settings:

| Property | Default | Description |
|---|---|---|
| `StartingRoom` | — | The room placed first, at the world origin |
| `ReberuRooms` | — | Pool of rooms available for generation |
| `TargetRoomAmount` | 10 | Desired total room count |
| `MinRoomAmount` | 5 | Minimum rooms required to consider generation successful |
| `MaxBacktrackTries` | 5 | Max consecutive backtracks before giving up |
| `RoomSelectionMethod` | Breadth | How the next source room is chosen |
| `BacktrackMethod` | FromTail | How the generator backtracks on failure |
| `DoorMap` | — | Maps Gameplay Tags to door/blocked-door actor classes |

</div>

---

<div id="extending-the-plugin">

### Extending the Plugin

Subclass `ALevelGeneratorActor` in C++ or Blueprint to customize generation behavior. Override any of the virtual methods listed in [Core Concepts](#core-concepts). Custom `ERoomSelection` and `ERoomBacktrack` enum values (`Custom1`–`Custom4`) are provided specifically for this purpose.

To add custom placement rules without subclassing the generator, create a `UReberuRule` Blueprint subclass and implement `ShouldPlaceRoom`.

</div>

---

<div id="credits" align="center">

  ### Credits

  Took inspiration from: <a href="https://github.com/BenPyton/ProceduralDungeon">ProceduralDungeon</a>, but the code and implementation is all original :).

  Created by [Peter Gilbert](https://petergilbert.pizza)

</div>
