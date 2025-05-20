
## Plugin Documentation: CustomMovementPlugin


### Table of Contents

1.  **Introduction**
    *   1.1. Purpose
    *   1.2. Features
2.  **Core Concepts & Components**
    *   2.1. `AMS_MovementNodeMeshStarter` (Actor)
    *   2.2. `FMoveNode` (Struct)
    *   2.3. `UMS_PathfindingSubsystem` (GameInstance Subsystem)
    *   2.4. `UMS_PathfindingSubsystemLib` (Blueprint Function Library)
    *   2.5. `AMS_MovementNode` (Actor - Debug Visualizer)
3.  **Setup and Configuration**
    *   3.1. Plugin Installation
    *   3.2. Level Setup
        *   3.2.1. Placing `AMS_MovementNodeMeshStarter`
        *   3.2.2. Tagging Floor Geometry
        *   3.2.3. Collision Configuration
    *   3.3. Configuration Parameters
4.  **Pathfinding System Workflow**
    *   4.1. Node Generation
    *   4.2. Path Request and A\* Algorithm
    *   4.3. Path Following (Conceptual)
5.  **Dynamic Environment Handling**
    *   5.1. Blocking and Unblocking Nodes
    *   5.2. Responding to Path Updates (`OnPathUpdated` Delegate)
    *   5.3. Adding Nodes at Runtime
6.  **Blueprint Integration**
    *   6.1. Using `UMS_PathfindingSubsystemLib`
    *   6.2. Subscribing to `OnPathUpdated`
7.  **C++ API Reference (Key Functions)**
    *   7.1. `UMS_PathfindingSubsystem`
    *   7.2. `AMS_MovementNodeMeshStarter`
8.  **Debugging Tools**
9.  **Limitations and Future Considerations**

---

### 1. Introduction

#### 1.1. Purpose

The `CustomMovementPlugin` for Unreal Engine provides a robust and dynamic grid-based pathfinding system. It is designed to allow characters and other actors to navigate complex 3D environments by generating a navigation graph at runtime and efficiently finding paths across it.

#### 1.2. Features

*   **Runtime Node Generation**: Automatically creates a navigation grid based on level geometry tagged as "Floor".
*   **A\* Pathfinding**: Utilizes the A\* algorithm for optimal path calculation.
*   **Dynamic Obstacle Support**: Allows for nodes to be blocked or unblocked at runtime, reflecting changes in the environment.
*   **Path Update Notifications**: Broadcasts events when the navigation graph changes, enabling agents to adapt their paths.
*   **Blueprint Accessibility**: Exposes key functionalities to Blueprints via a function library.
*   **Debug Visualizations**: Offers options to display debug information for node generation and pathfinding processes.

---

### 2. Core Concepts & Components

The plugin revolves around a few key classes and structs that work together to create the pathfinding system.

#### 2.1. `AMS_MovementNodeMeshStarter` (Actor)

*   **Responsibility**: This actor is the entry point for the navigation system's initialization. It is responsible for scanning the environment and generating the initial set of navigation nodes (`FMoveNode`).
*   **Initialization Process (`BeginPlay`)**:
    1.  Performs an initial downward raycast to detect a surface tagged "Floor". This point becomes the seed for node generation.
    2.  Calls `GenerateNodes()` to build the `NodeMap`.
    3.  Passes the completed `NodeMap` to the `UMS_PathfindingSubsystem`.
    4.  Broadcasts its `OnNodeMapReady` delegate.
    5.  Starts a periodic timer to call `UpdateBlockedPaths()`.
*   **Node Generation (`GenerateNodes`)**:
    *   Uses a queue-based flood-fill approach starting from the initial floor point.
    *   For each processed node, it checks potential neighbor locations (up, down, left, right) based on `NodeSeparationX_` and `NodeSeparationY_`.
    *   `PerformRaycastAtPosition()`: Validates if a potential neighbor location is suitable (e.g., has ground). Uses the "TestFloor" collision profile.
    *   `PerformRaycastToPosition()`: Checks for direct line-of-sight traversability between the current node and a valid potential neighbor. Uses the "CheckFreeWay" collision profile.
    *   Valid and traversable neighbors are added to the `NodeMap` and the `Neighbors` list of the respective nodes.
*   **Path Re-evaluation (`UpdateBlockedPaths`)**:
    *   This timer-driven function iterates through all nodes in the `NodeMap` and re-checks the traversability of connections to their neighbors using `PerformRaycastToPosition()`.
    *   It directly updates the `bool` accessibility flag within each `FMoveNode::Neighbors` map.
    *   **Note**: This function *does not* inherently broadcast `UMS_PathfindingSubsystem::OnPathUpdated`. It's a mechanism for detecting passive changes in traversability.

#### 2.2. `FMoveNode` (Struct)

*   **Responsibility**: Represents a single traversable point in the navigation graph. This is a lightweight struct for efficient storage and processing.
*   **Properties**:
    *   `Position` (FVector): The world-space coordinate of the node.
    *   `GridPosition` (FIntPoint): The 2D integer-based coordinate of the node within the grid system.
    *   `Neighbors` (TMap<TSharedPtr<FMoveNode>, bool>): A map where keys are shared pointers to neighboring `FMoveNode`s and values are booleans indicating if the direct path to that neighbor is currently clear (`true`) or obstructed (`false`).

#### 2.3. `UMS_PathfindingSubsystem` (GameInstance Subsystem)

*   **Responsibility**: The central manager and query interface for the pathfinding system. It holds the node graph and performs path calculations. Being a GameInstance Subsystem ensures it's globally accessible.
*   **Key Data Structures**:
    *   `NodeMap` (TMap<FIntPoint, TSharedPtr<FMoveNode>>): The primary data structure storing all `FMoveNode`s, keyed by their `GridPosition`.
    *   `BlockedNodes` (TSet<FIntPoint>): A set of `GridPosition`s for nodes that are considered permanently or semi-permanently untraversable (e.g., a building is constructed on them). A* will avoid these nodes.
*   **Core Functionality**:
    *   **Pathfinding**:
        *   `FindPathNodes(StartNode, GoalNode)`: Implements the A* algorithm.
            *   Uses open set, closed set, G-score (cost from start), and F-score (G-score + heuristic).
            *   The heuristic is the Manhattan distance: `abs(dx) + abs(dy)` on the grid.
            *   Considers nodes in `BlockedNodes` and the accessibility flag in `FMoveNode::Neighbors`.
        *   `FindPathPoints(StartNode, GoalNode)`: A convenience wrapper around `FindPathNodes` that returns an array of `FIntPoint` (grid coordinates).
    *   **Node Access**:
        *   `FindClosestNodeToActor(AActor*)`, `FindClosestNodeToPosition(FVector)`: Utility functions to map world locations to the nearest `FMoveNode`.
        *   `FindNodeByGridPosition(const FIntPoint&)`: Direct lookup of a node by its grid coordinates.
        *   `GetRandomFreeNode(FVector& OutLocation, FIntPoint& OutGrid)`: Selects a random node from `NodeMap` that is not present in the `BlockedNodes` set.
    *   **Dynamic Graph Modification**:
        *   `AddNodeAtPosition(const FVector& Position)`: Dynamically adds a new node to `NodeMap`, calculates its grid position, and attempts to link it to existing neighbors based on proximity and raycast checks.
        *   `BlockNode(FVector Position)`, `BlockNodeGrid(FIntPoint GridPosition)`: Adds the specified node to `BlockedNodes`. Iterates through its neighbors and marks the connections *to and from* this node as blocked (`false` in their respective `Neighbors` maps). Broadcasts `OnPathUpdated`.
        *   `UnblockNode(FVector Position)`, `UnblockNodeGrid(FIntPoint GridPosition)`: Removes the node from `BlockedNodes`. Re-evaluates connections to its neighbors via raycasting. If a path is clear, marks the connection as traversable (`true`). Broadcasts `OnPathUpdated`.
        *   `DeactivateClosestNodes(const FVector& CenterPoint, TArray<FIntPoint>& OutDeactivatedNodePositions, int32 NumNodesToDeactivate)`: Finds a specified number of the closest *unblocked* nodes to a given point and blocks them using `BlockNodeGrid`. Returns the list of affected node positions.
    *   `IsNodeBlocked(const FIntPoint& GridPosition) const`: Queries if a node is in the `BlockedNodes` set.
*   **Events**:
    *   `OnPathUpdated` (FOnPathUpdated Delegate, takes `FIntPoint ChangedNodePos`): Broadcast whenever a node's accessibility is significantly altered by `BlockNode`, `UnblockNode`, or `SetNodeBlockedStatus`. This allows pathfinding agents to react to changes.

#### 2.4. `UMS_PathfindingSubsystemLib` (Blueprint Function Library)

*   **Responsibility**: Provides a simplified Blueprint interface to the `UMS_PathfindingSubsystem`.
*   **Functions**:
    *   `AddNodeVector(FVector Position)`: Exposes `UMS_PathfindingSubsystem::AddNodeAtPosition`.
    *   `FindPathVector(const UObject* WorldContextObject, FVector Start, FVector Goal)`: Exposes pathfinding. It internally finds the closest nodes to the start/goal vectors and returns the path as an array of `FVector` world positions.

#### 2.5. `AMS_MovementNode` (Actor - Debug Visualizer)

*   **Responsibility**: This actor was likely an earlier concept for representing nodes physically in the world. In the current implementation, it's primarily used by `AMS_MovementNodeMeshStarter::SpawnAgentAtPosition` for debugging purposes if enabled.
*   **Features**:
    *   Contains a `UStaticMeshComponent` (defaulted to a cube) for visual representation.
    *   Its tick and collision are disabled when spawned by `SpawnAgentAtPosition` to minimize performance overhead.
*   **Note**: The system's core logic relies on the in-memory `FMoveNode` structs for performance, not on these actors for pathfinding calculations.

---

### 3. Setup and Configuration

#### 3.1. Plugin Installation

1.  Ensure the `CustomMovementPlugin` is located in your project's `Plugins` directory or the engine's `Plugins` directory.
2.  Open your Unreal Engine project.
3.  Navigate to **Edit > Plugins**.
4.  Search for "CustomMovementPlugin" and ensure its "Enabled" checkbox is ticked.
5.  Restart the editor if prompted.

#### 3.2. Level Setup

##### 3.2.1. Placing `AMS_MovementNodeMeshStarter`

*   From the Content Browser (if C++ classes are visible or you've created a Blueprint child), drag an instance of `AMS_MovementNodeMeshStarter` into your persistent level.
*   Position this actor high above the central area where you intend navigation to occur. It will perform a downward raycast from its location to find the initial "Floor" surface.

##### 3.2.2. Tagging Floor Geometry

*   The `AMS_MovementNodeMeshStarter` relies on Actors or Components having the specific tag **"Floor"** to identify walkable surfaces for node generation.
*   Select your primary ground meshes (Static Meshes, Landscapes, etc.).
*   In the **Details panel**, under **Actor > Tags** or **Component > Tags**, add a new tag and name it `Floor`.

##### 3.2.3. Collision Configuration

The plugin uses specific collision profiles/channels for its raycasts. These need to be set up correctly in your **Project Settings > Engine > Collision**:

1.  **Object Channels**:
    *   Consider creating a new Object Channel named something like "NavFloor" and another named "NavObstacle".
2.  **Trace Channels**:
    *   **"TestFloor"**: This is the `FName` used in `AMS_MovementNodeMeshStarter::PerformRaycastAtPosition`. Create a new Trace Channel (e.g., TraceChannel1, rename it "TestFloorTrace").
        *   Set its **Default Response** to `Ignore`.
    *   **"CheckFreeWay"**: This is the `FName` used in `AMS_MovementNodeMeshStarter::PerformRaycastToPosition` and `UMS_PathfindingSubsystem` when unblocking nodes. Create another new Trace Channel (e.g., TraceChannel2, rename it "CheckFreeWayTrace").
        *   Set its **Default Response** to `Ignore`.
3.  **Collision Presets / Profiles**:
    *   Modify your existing presets or create new ones:
        *   **For your "Floor" geometry**:
            *   Set its **Object Type** to "NavFloor" (or your chosen channel).
            *   Ensure it **Blocks** the "TestFloorTrace" channel.
            *   It can `Ignore` or `Overlap` "CheckFreeWayTrace" as appropriate (usually ignore unless floors can also block line of sight between nodes).
        *   **For your Obstacles (walls, large rocks, etc.)**:
            *   Set their **Object Type** to "NavObstacle" (or `WorldStatic`, `WorldDynamic` as appropriate).
            *   Ensure they **Block** the "CheckFreeWayTrace" channel.
            *   They can `Ignore` "TestFloorTrace".

    *Alternatively, if not using custom object channels, ensure `TestFloor` profile in C++ (e.g., `ECC_GameTraceChannel1`) is set up to collide with your floor, and `CheckFreeWay` profile (e.g., `ECC_GameTraceChannel2`) is set to collide with obstacles.* The current C++ code uses `TEXT("TestFloor")` and `TEXT("CheckFreeWay")` as profile names, which implies they should be defined as **Preset** names that use the appropriate trace channel settings. If `LineTraceSingleByProfile` is used, define these presets. If `LineTraceSingleByChannel` is used, ensure the ECC enum values are correct. The provided code `LineTraceSingleByProfile(HitResult, Start, End, TEXT("TestFloor"), Params)` indicates you should define a **Collision Profile** named "TestFloor".

#### 3.3. Configuration Parameters

*   **`AMS_MovementNodeMeshStarter`**:
    *   `NodeSeparationX_`, `NodeSeparationY_` (int32, C++ defaults: 250): These values in `AMS_MovementNodeMeshStarter.cpp` control the spacing of the generated grid nodes. Smaller values create a denser grid (potentially more accurate paths but higher generation time and memory) and vice-versa. To make these configurable in the editor, expose them as `UPROPERTY(EditAnywhere)`.
*   **Debug Booleans (Static C++)**:
    *   `bShowDebugLinesStarter` (in `MS_MovementNodeMeshStarter.h`): Set to `true` in C++ to enable visualization of node generation raycasts.
    *   `bShowDebugLinesPathfinding` (in `MS_PathfindingSubsystem.h`): Set to `true` in C++ to enable visualization of A* pathfinding attempts and node blocking/unblocking.

---

### 4. Pathfinding System Workflow

#### 4.1. Node Generation

1.  **Trigger**: Occurs when `AMS_MovementNodeMeshStarter::BeginPlay()` is called.
2.  **Initial Raycast**: A downward raycast from `AMS_MovementNodeMeshStarter`'s position searches for an Actor/Component tagged "Floor" using the "TestFloor" collision profile. The impact point is the starting location.
3.  **Iterative Expansion (`GenerateNodes`)**:
    *   A queue (`NodeQueue`) is initialized with the starting node.
    *   While the queue is not empty:
        *   Dequeue a `CurrentNode`.
        *   For each cardinal direction (right, left, up, down):
            *   Calculate `NeighborGridPos` and `NeighborPosition`.
            *   `PerformRaycastAtPosition(NeighborPosition)`: Checks if this new position is valid (e.g., on a "Floor").
            *   If valid, `PerformRaycastToPosition(CurrentNode->Position, NeighborPosition)`: Checks if the path from `CurrentNode` to this neighbor is clear of obstacles using "CheckFreeWay" profile.
            *   If both checks pass, a new `FMoveNode` is created (if not already existing at `NeighborGridPos`), added to `NodeMap`, and linked as a neighbor to `CurrentNode` (and vice-versa). The connection's traversability is marked.
            *   If the new neighbor hasn't been visited, it's added to `NodeQueue`.
4.  **Subsystem Update**: Once `GenerateNodes` completes, `NodeMap` is passed to `UMS_PathfindingSubsystem::SetNodeMap()`.
5.  **Ready Event**: `AMS_MovementNodeMeshStarter::OnNodeMapReady` delegate is broadcast.

#### 4.2. Path Request and A\* Algorithm

1.  **Agent Requirement**: An agent needs to move from its current location (Start) to a target location (Goal).
2.  **Subsystem Access**: The agent retrieves `UMS_PathfindingSubsystem` (e.g., via `GetGameInstance()->GetSubsystem<UMS_PathfindingSubsystem>()`).
3.  **Node Mapping**:
    *   `StartNode = PathSubsystem->FindClosestNodeToPosition(AgentCurrentLocation)`
    *   `GoalNode = PathSubsystem->FindClosestNodeToPosition(TargetWorldLocation)`
4.  **Path Calculation (`FindPathNodes`)**:
    *   **Initialization**:
        *   `OpenSet`: Nodes to be evaluated (initially contains `StartNode`). Implemented as `PriorityQueue` sorted by F-score.
        *   `ClosedSet`: Nodes already evaluated.
        *   `GScore`: Map of `FMoveNode` to cost from `StartNode` to that node (`GScore[StartNode] = 0`).
        *   `FScore`: Map of `FMoveNode` to estimated total cost from `StartNode` to `GoalNode` via that node (`FScore[StartNode] = Heuristic(StartNode, GoalNode)`).
        *   `CameFrom`: Map to reconstruct the path, storing the predecessor of each node in the optimal path.
    *   **Loop**: While `OpenSet` (PriorityQueue) is not empty:
        *   Select `CurrentNode` from `OpenSet` with the lowest `FScore`.
        *   If `CurrentNode == GoalNode`, reconstruct and return path using `CameFrom`.
        *   Move `CurrentNode` from `OpenSet` to `ClosedSet`.
        *   For each `Neighbor` of `CurrentNode`:
            *   If `Neighbor` is in `ClosedSet` or the path `CurrentNode -> Neighbor` is blocked (checked via `NeighborPair.Value` from `CurrentNode->Neighbors` and `PathSubsystem->IsNodeBlocked(Neighbor->GridPosition)`), skip.
            *   `TentativeGScore = GScore[CurrentNode] + Distance(CurrentNode, Neighbor)`.
            *   If `TentativeGScore < GScore[Neighbor]` (or `Neighbor` not in `GScore`):
                *   `CameFrom[Neighbor] = CurrentNode`
                *   `GScore[Neighbor] = TentativeGScore`
                *   `FScore[Neighbor] = GScore[Neighbor] + Heuristic(Neighbor, GoalNode)`
                *   If `Neighbor` not in `OpenSet`, add it.
    *   **Result**: Returns `TArray<TSharedPtr<FMoveNode>>` or an empty array if no path is found.
5.  **Path Conversion (`FindPathPoints`)**: The `TArray<TSharedPtr<FMoveNode>>` is often converted to `TArray<FIntPoint>` for easier use by agents.

#### 4.3. Path Following (Conceptual)

*   The agent receives the `TArray<FIntPoint>` representing the path.
*   It sets its `CurrentNodeIndex = 0`.
*   It retrieves the world position of `Path[CurrentNodeIndex]` using `PathSubsystem->FindNodeByGridPosition(Path[CurrentNodeIndex])->Position`.
*   It uses the engine's AI movement capabilities (e.g., `AAIController::MoveToLocation`) to move towards this position.
*   Upon reaching (or nearing) the current target node's position, it increments `CurrentNodeIndex` and repeats the process for the next node in the path.
*   The path is complete when `CurrentNodeIndex` reaches the end of the `Path` array.
*   (The `AlesAndFables` plugin's `MS_FollowNodePath` BTTask implements this logic).

---

### 5. Dynamic Environment Handling

The plugin supports changes to the navigation graph after initial generation.

#### 5.1. Blocking and Unblocking Nodes

*   **Purpose**: To make specific nodes temporarily or permanently untraversable (e.g., a building is constructed, a rockslide occurs).
*   **Mechanism (`UMS_PathfindingSubsystem`)**:
    *   `BlockNodeGrid(GridPosition)` / `BlockNode(WorldPosition)`:
        1.  Adds `GridPosition` to the `BlockedNodes` set.
        2.  Retrieves the `FMoveNode` at `GridPosition`.
        3.  Iterates through its `Neighbors`: sets the boolean flag in `Node->Neighbors[Neighbor]` to `false`.
        4.  Crucially, it also iterates through `Neighbor->Neighbors[Node]` and sets that to `false` (bidirectional blocking).
        5.  Broadcasts `OnPathUpdated(GridPosition)`.
    *   `UnblockNodeGrid(GridPosition)` / `UnblockNode(WorldPosition)`:
        1.  Removes `GridPosition` from the `BlockedNodes` set.
        2.  Retrieves the `FMoveNode` at `GridPosition`.
        3.  Iterates through its `Neighbors`:
            *   Performs a raycast (`PerformRaycastToPosition`) between `Node->Position` and `Neighbor->Position`.
            *   If the raycast is clear, sets `Node->Neighbors[Neighbor]` to `true` and `Neighbor->Neighbors[Node]` to `true`.
        4.  Broadcasts `OnPathUpdated(GridPosition)`.
*   `DeactivateClosestNodes(...)`: A utility that calls `BlockNodeGrid` for a number of nodes closest to a given point. Useful for making space for new constructions.

#### 5.2. Responding to Path Updates (`OnPathUpdated` Delegate)

*   **Purpose**: Allows pathfinding agents to react when the traversability of the graph changes.
*   **Subscription**: Agents (e.g., characters following a path) should subscribe to `UMS_PathfindingSubsystem::OnPathUpdated`.
    ```cpp
    // In the agent's BeginPlay or when starting to follow a path:
    UMS_PathfindingSubsystem* PathSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UMS_PathfindingSubsystem>();
    if (PathSubsystem)
    {
        PathSubsystem->OnPathUpdated.AddDynamic(this, &AMyAgent::HandlePathNodeUpdate);
    }
    ```
*   **Handler Function (`HandlePathNodeUpdate(FIntPoint ChangedNodePos)`)**:
    1.  Check if the agent has an active path (`Path_`).
    2.  Iterate through the *remaining* nodes in `Path_` (from `CurrentNodeIndex` onwards).
    3.  If `ChangedNodePos` is one of these nodes, or if `ChangedNodePos` is a neighbor of one of these nodes and the connection is now blocked, the current path is potentially invalid.
    4.  The agent should then:
        *   Clear its current path (`Path_.Empty()`, `CurrentNodeIndex = -1`).
        *   Request a new path to its original final destination.
        *   The BT/AI logic needs to handle pathfinding failure gracefully.
    *   (See `AMS_AICharacter::OnPathUpdated` for a concrete example).

#### 5.3. Adding Nodes at Runtime

*   **Purpose**: To expand the navigation graph if new walkable areas become available.
*   **Mechanism (`UMS_PathfindingSubsystem::AddNodeAtPosition`)**:
    1.  Takes a `FVector WorldPosition`.
    2.  Calculates the `FIntPoint GridPosition` (rounding world coordinates).
    3.  Creates a new `TSharedPtr<FMoveNode>`.
    4.  Adds it to `NodeMap` if a node doesn't already exist at `GridPosition`.
    5.  Iterates through all existing nodes in `NodeMap`:
        *   If an existing node is within a certain distance (e.g., `NodeSeparation_ * 2`), it's a potential neighbor.
        *   Performs a raycast (`PerformRaycastToPosition`) between the new node and the potential neighbor.
        *   If clear, establishes a bidirectional connection in their `Neighbors` maps.
    *   Returns the `FIntPoint` of the newly added or found node.
*   **Note**: This does not automatically broadcast `OnPathUpdated` for the new node itself, but it might if existing nodes become connected to it.

---

### 6. Blueprint Integration

#### 6.1. Using `UMS_PathfindingSubsystemLib`

This static Blueprint Function Library provides access to common subsystem functions:

*   **`Find Path Vector`**:
    *   Inputs: `WorldContextObject` (get from self or any world actor), `Start` (FVector), `Goal` (FVector).
    *   Output: `TArray<FVector>` (Path Points).
    *   Use this to get a series of world locations your Blueprint character can move to.
*   **`Add Node Vector`**:
    *   Input: `Position` (FVector).
    *   Allows adding a new navigation node from Blueprints.

#### 6.2. Subscribing to `OnPathUpdated`

Blueprints can respond to navigation graph changes:

1.  **Get Subsystem**: In your Blueprint (e.g., an AI Controller or Character `Event BeginPlay`), get the `UMS_PathfindingSubsystem` instance:
    *   `Get Game Instance` -> `Get Subsystem` (select `MS_PathfindingSubsystem`).
2.  **Bind Event**:
    *   Drag off the subsystem reference and find `Assign to OnPathUpdated` or `Bind Event to OnPathUpdated`.
    *   Create a new Custom Event in your Blueprint that matches the delegate's signature (takes an `IntPoint` named `ChangedNodePos`).
    *   Connect this Custom Event to the `Bind Event` node.
3.  **Implement Handler**: In your Custom Event, implement the logic to check if the `ChangedNodePos` affects the Blueprint's current path and re-path if necessary.

---

### 7. C++ API Reference (Key Functions)

Refer to the header files for full declarations. This highlights most critical functions.

#### 7.1. `UMS_PathfindingSubsystem`

*   `TArray<FIntPoint> FindPathPoints(TSharedPtr<FMoveNode> StartNode, TSharedPtr<FMoveNode> GoalNode)`
*   `TArray<TSharedPtr<FMoveNode>> FindPathNodes(TSharedPtr<FMoveNode> StartNode, TSharedPtr<FMoveNode> GoalNode)`
*   `TSharedPtr<FMoveNode> FindNodeByGridPosition(const FIntPoint& GridPosition)`
*   `bool GetRandomFreeNode(FVector& OutLocation, FIntPoint& OutGrid)`
*   `TSharedPtr<FMoveNode> FindClosestNodeToActor(AActor* TargetActor)`
*   `TSharedPtr<FMoveNode> FindClosestNodeToPosition(FVector position)`
*   `void SetNodeMap(TMap<FIntPoint, TSharedPtr<FMoveNode>> newNodeMap)`
*   `FIntPoint AddNodeAtPosition(const FVector& Position)`
*   `void BlockNode(FVector Position)`
*   `void BlockNodeGrid(FIntPoint GridPosition)`
*   `void UnblockNode(FVector Position)`
*   `void UnblockNodeGrid(FIntPoint GridPosition)`
*   `bool IsNodeBlocked(const FIntPoint& GridPosition) const`
*   `bool DeactivateClosestNodes(const FVector& CenterPoint, TArray<FIntPoint>& OutDeactivatedNodePositions, int32 NumNodesToDeactivate = 4)`
*   `FOnPathUpdated OnPathUpdated` (Delegate)

#### 7.2. `AMS_MovementNodeMeshStarter`

*   `void GenerateNodes(FVector FirstPos)` (Protected)
*   `bool PerformRaycastAtPosition(const FVector& Position)` (Protected)
*   `bool PerformRaycastToPosition(const FVector& Start, const FVector& End)` (Protected)
*   `void UpdateBlockedPaths()` (Protected, Timer Callback)
*   `FOnNodeMapReady OnNodeMapReady` (Delegate)

---

### 8. Debugging Tools

*   **`bShowDebugLinesStarter`** (static bool in `MS_MovementNodeMeshStarter.h`):
    *   Default: `false`. Set to `true` in C++.
    *   Visualizes:
        *   Initial downward raycast.
        *   Raycasts for `PerformRaycastAtPosition` (node validity).
        *   Raycasts for `PerformRaycastToPosition` (neighbor connectivity during generation).
        *   `SpawnAgentAtPosition` will spawn visual `AMS_MovementNode` actors if uncommented/enabled.
        *   Raycasts during `UpdateBlockedPaths`.
*   **`bShowDebugLinesPathfinding`** (static bool in `MS_PathfindingSubsystem.h`):
    *   Default: `false`. Set to `true` in C++.
    *   Visualizes:
        *   Start (Green Sphere) and Goal (Red Sphere) nodes for `FindPathNodes`.
        *   Node exploration during A* (Yellow/Cyan spheres, though sometimes commented out for performance).
        *   `BlockNode`: Node turns Red, connections to it turn Red.
        *   `UnblockNode`: Node turns Green, valid connections turn Blue.
        *   `AddNodeAtPosition`: New node Purple, connections Cyan.
*   **Unreal Engine Tools**:
    *   **Output Log**: Check for `UE_LOG` messages from the plugin.
    *   **Gameplay Debugger** (Apostrophe key): Can be extended to show custom pathfinding data.
    *   **Visual Logger**: For recording and inspecting pathfinding events over time.

---

### 9. Limitations and Future Considerations

*   **Performance**: For very large or dense grids, initial node generation can be time-consuming. `UpdateBlockedPaths` also iterates all nodes; consider optimizing if it becomes a bottleneck.
*   **Z-Axis**: The current grid generation is primarily 2.5D (finds floor, then expands on X/Y). True 3D navigation (e.g., flying, multi-level buildings without distinct "floors" for the starter) would require significant modification to node generation and neighbor-finding logic.
*   **Node Size/Shape**: Assumes point nodes and fixed separation. Different agent sizes might require a more sophisticated representation (e.g., navigation mesh generation).
*   `UpdateBlockedPaths` Eventing: Currently, changes detected by `AMS_MovementNodeMeshStarter::UpdateBlockedPaths` do not broadcast `OnPathUpdated`. This could be added if passive environment changes need to trigger AI re-pathing more proactively.
*   **Memory**: `TSharedPtr` for `FMoveNode` helps manage memory, but very large maps will still consume RAM.
*   **Path Smoothing**: The generated paths are a series of straight lines between node centers. Path smoothing algorithms could be implemented for more natural movement.

