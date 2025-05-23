## Plugin Documentation: AlesAndFables

### Table of Contents

1.  **Introduction**
    *   1.1. Purpose
    *   1.2. Features
    *   1.3. Dependencies
2.  **Core Systems & Components**
    *   2.1. AI Character Subsystem
        *   2.1.1. `AMS_AICharacter` (Pawn)
        *   2.1.2. `AMS_AICharacterController` (Controller)
    *   2.2. AI Management System
        *   2.2.1. `AMS_AIManager` (Actor)
    *   2.3. Resource & Inventory System
        *   2.3.1. `UMS_InventoryComponent` (Actor Component)
        *   2.3.2. `ResourceType` (Enum)
        *   2.3.3. `FResource` (Struct)
    *   2.4. Pawn Statistics (Needs) System
        *   2.4.1. `UMS_PawnStatComponent` (Actor Component)
    *   2.5. Time and Environment System
        *   2.5.1. `UMS_TimeSubsystem` (GameInstance Subsystem)
        *   2.5.2. `AMS_SkyController` (Actor)
    *   2.6. Quest System
        *   2.6.1. `FQuest` (Struct)
        *   2.6.2. `FBidInfo` (Struct)
    *   2.7. Placeable Actors & Pools
        *   2.7.1. Workplaces (`MS_BaseWorkPlace`, `MS_WorkpPlacePool`, etc.)
        *   2.7.2. Storage (`MS_StorageBuilding`, `MS_StorageBuildingPool`)
        *   2.7.3. Housing (`MS_House`)
        *   2.7.4. Construction (`MS_ConstructionSite`)
        *   2.7.5. Farming (`MS_WheatField`)
        *   2.7.6. Recreation (`MS_Tavern`)
3.  **Setup and Configuration**
    *   3.1. Plugin Installation
    *   3.2. Essential Actor Placement
    *   3.3. `AMS_AIManager` Configuration
    *   3.4. `AMS_AICharacter` Blueprint Setup
    *   3.5. `UMS_TimeSubsystem` Configuration
    *   3.6. `AMS_SkyController` Configuration
    *   3.7. Placeable Actor Blueprint Setup
    *   3.8. Behavior Tree and Blackboard Setup
4.  **Behavior Tree Integration**
    *   4.1. Overview of Provided Custom Nodes
    *   4.2. Essential Blackboard Keys
    *   4.3. Custom BT Tasks (Detailed)
    *   4.4. Custom BT Decorators (Detailed)
    *   4.5. Custom BT Services (Detailed)
5.  **Key AI Workflows & Mechanics**
    *   5.1. Quest Lifecycle (Generation, Bidding, Assignment, Execution, Completion)
    *   5.2. Needs Management & Fulfillment
    *   5.3. Resource Gathering & Storage
    *   5.4. Construction Projects
    *   5.5. Farming Cycle (Planting, Watering, Harvesting)
    *   5.6. Housing and Sleep
    *   5.7. Recreation (Tavern Visits)
6.  **C++ API Reference (Key Classes & Functions)**
7.  **Extensibility and Customization**
8.  **Debugging and Troubleshooting**

---

### 1. Introduction

#### 1.1. Purpose

The `AlesAndFables` plugin provides a comprehensive framework for creating AI-driven simulations within Unreal Engine. It focuses on character autonomy, resource-based economies, dynamic quests, and environmental interactions, aiming to simulate a small, evolving virtual world.

#### 1.2. Features

*   **Autonomous AI Characters**: Characters with needs (hunger, thirst, energy, happiness) and inventories.
*   **Behavior Tree Driven Logic**: Flexible AI decision-making through Behavior Trees with a suite of custom nodes.
*   **Central AI Management**: An `AMS_AIManager` orchestrates high-level simulation aspects.
*   **Dynamic Quest System**: Quests are generated based on simulation needs, with AI bidding for tasks.
*   **Resource Management**: Simple inventory system for tracking resources and money.
*   **Construction System**: AI can be tasked to deliver resources to construct new buildings.
*   **Farming System**: Specialized `AMS_WheatField` actors with a multi-stage growth and interaction cycle.
*   **Day/Night Cycle**: Managed by `UMS_TimeSubsystem`, affecting AI behavior (e.g., sleep) and environment (via `AMS_SkyController`).
*   **Housing**: AI characters can be assigned houses for sleeping.
*   **Pathfinding Integration**: Relies on the `CustomMovementPlugin` for all character navigation.

#### 1.3. Dependencies

*   **`CustomMovementPlugin`**: This plugin is a hard dependency. `AlesAndFables` will not function correctly without it, as all AI character movement relies on its pathfinding capabilities. Ensure `CustomMovementPlugin` is enabled and properly configured.

---

### 2. Core Systems & Components

#### 2.1. AI Character Subsystem

##### 2.1.1. `AMS_AICharacter` (Pawn)

*   **Role**: The primary autonomous agent. Each `AMS_AICharacter` instance represents an individual in the simulation.
*   **Core Components Attached**:
    *   `Inventory_` (UInventoryComponent\*): Manages the character's personal resources (e.g., gathered items before storing) and money.
    *   `PawnStats_` (UMS_PawnStatComponent\*): Tracks and updates the character's needs (Hunger, Thirst, Energy, Happiness).
    *   `WidgetComponent_`: Used to display a UI element representing the character's stats (e.g., needs bars).
    *   `ShopCollision` (UBoxComponent\*): A trigger volume used for proximity-based interactions, such as delivering items to a construction site or interacting with a storage building.
*   **Key Properties**:
    *   `behaviorTree_` (TObjectPtr<UBehaviorTree>): The Behavior Tree asset that defines the character's decision-making logic.
    *   `AIManager` (TWeakObjectPtr<AMS_AIManager>): A weak pointer to the global `AMS_AIManager` instance, used for quest interactions and other global queries.
    *   `PathfindingSubsystem` (TObjectPtr<UMS_PathfindingSubsystem>): A cached pointer to the `CustomMovementPlugin`'s pathfinding subsystem.
    *   `Path_` (TArray<FIntPoint>): Stores the current navigation path as a sequence of grid coordinates from the `PathfindingSubsystem`.
    *   `CurrentNodeIndex` (int32): Tracks the current segment of the `Path_` being traversed.
    *   `CurrentTargetLocation` (FVector): The world-space destination of the current path segment.
    *   `AssignedQuest` (FQuest): Holds the data for the quest currently assigned to this character.
    *   `MyHouse` (TWeakObjectPtr<AMS_House>): A weak pointer to the `AMS_House` actor this character is assigned to for sleeping.
    *   `Money` (int32): The character's personal currency, earned from completing quests.
    *   `WorkPlacesPool_`, `StorageBuldingsPool_`, `BulletingBoardPool_` (TWeakObjectPtr<AActor>): Pointers to various pool managers, typically found and assigned in `BeginPlay`.
*   **Key Functionality**:
    *   **Initialization (`BeginPlay`)**: Caches references to essential systems like `AIManager`, `PathfindingSubsystem`, and various actor pools. Subscribes to `AIManager::OnQuestAvailable` and `PathfindingSubsystem::OnPathUpdated`.
    *   **Pathfinding (`CreateMovementPath`, `OnPathUpdated`)**: Interfaces with `PathfindingSubsystem` to generate and manage movement paths. Reacts to dynamic changes in the navigation graph.
    *   **Quest Interaction**:
        *   `OnNewQuestReceived`: Evaluates new quest announcements.
        *   `EvaluateQuestAndBid`: Calculates bid value based on internal state and quest parameters.
        *   `AssignQuest`: Called by `AIManager` to formally assign a quest and update the character's Blackboard.
        *   `CompleteCurrentQuest`: Notifies `AIManager` upon quest completion.
    *   **Needs Management (`CheckIfHungry`)**: Updates Blackboard keys based on `PawnStats_`, allowing the Behavior Tree to react to needs.
    *   **Direct Consumption (`ConsumeResourceDirectly`)**: Allows the AI to consume a resource directly from a workplace (e.g., drinking water) to satisfy a need.
    *   **Interaction Overlaps (`OnOverlapBegin`, `OnOverlapEnd`)**: Handles logic when the `ShopCollision` trigger interacts with other key actors like storage buildings (for item deposit/retrieval, satisfying needs), construction sites (resource delivery), or workplaces (marking arrival).

##### 2.1.2. `AMS_AICharacterController` (Controller)

*   **Role**: Standard Unreal AI Controller that possesses an `AMS_AICharacter` pawn and is responsible for running its associated Behavior Tree.
*   **Core Components**:
    *   `behaviorTree_` (UBehaviorTreeComponent\*): The component that executes the Behavior Tree logic.
    *   `blackboard_` (UBlackboardComponent\*): The data store for the Behavior Tree.
*   **Key Functionality (`OnPossess`)**:
    *   Initializes the `blackboard_` using the `BlackboardAsset` defined in the `AMS_AICharacter`'s `behaviorTree_`.
    *   Sets up initial Blackboard values:
        *   `SelfActor`: Reference to the possessed `AMS_AICharacter`.
        *   Default states for various boolean flags like `GettingTask`, `Working`, `DoingTask`, etc. These flags are critical for driving the Behavior Tree's logic flow.
    *   Starts the execution of the Behavior Tree.

#### 2.2. AI Management System

##### 2.2.1. `AMS_AIManager` (Actor)

*   **Role**: Acts as a central orchestrator for the AI simulation. It manages quests, initiates construction projects, and oversees AI housing and high-level resource demands. Typically, only one instance of `AMS_AIManager` should exist in the level.
*   **Key Properties**:
    *   `CentralStorageBuilding` (TWeakObjectPtr<AMS_StorageBuilding>): A reference to the primary storage facility from which global resource levels are assessed.
    *   Quest Management:
        *   `AvailableQuests_` (TArray<FQuest>): A list of quests that are currently open for AI characters to bid on.
        *   `AssignedQuests_` (TMap<FGuid, TWeakObjectPtr<AMS_AICharacter>>): Tracks which AI is assigned to which quest, using the `FQuest::QuestID`.
        *   `CurrentBids` (TMap<FGuid, TArray<FBidInfo>>): Stores all bids received for a specific quest during its bidding window.
        *   `BidTimers` (TMap<FGuid, FTimerHandle>): Manages the duration for which a quest is open for bidding.
        *   `BidDuration` (float): Default duration for the quest bidding window.
        *   `LowResourceThreshold` (int32): If a `ManagedResourceType` in `CentralStorageBuilding` falls below this, new gathering quests are generated.
        *   `MaxResourcePerQuest` (int32): The maximum amount of a resource a single gathering quest will ask for.
        *   `DeliveryCarryCapacity` (int32): Max amount for a single delivery trip quest (e.g., for construction).
    *   Construction Management:
        *   `ConstructionSiteClass` (TSubclassOf<AMS_ConstructionSite>): The Blueprint class to spawn for new construction projects.
        *   `HouseBuildingClass`, `WheatFieldClass`, `TavernBuildingClass` (TSubclassOf<AActor>): Blueprint classes for the final buildings.
        *   `HouseWoodCost`, `WheatFieldWoodCost`, `TavernWoodCost` (int32): Resource costs for buildings.
        *   `MaxConcurrentConstruction` (int32): Limits how many construction projects can be active simultaneously.
        *   `ActiveConstructionDeliveryQuests` (TMap<TWeakObjectPtr<AMS_ConstructionSite>, TArray<FGuid>>): Tracks delivery quests associated with specific construction sites.
    *   Population & Housing:
        *   `AICharacterClass` (TSubclassOf<AMS_AICharacter>): The AI character Blueprint to consider for population counts.
        *   `HousingCheckInterval` (float): How often to check population vs. housing capacity.
        *   `CurrentPopulation`, `TotalHousingCapacity` (int32): Tracked internally.
    *   Other:
        *   `ManagedResourceTypes` (TArray<ResourceType>): List of resources the AIManager actively tries to maintain stock of.
        *   `PathfindingSubsystemCache`: Cached pointer to the pathfinding system.
*   **Key Functionality**:
    *   **Initialization (`BeginPlay`)**: Locates/spawns essential pools (`StorageBuldingsPool_`), finds the `CentralStorageBuilding`, caches `PathfindingSubsystem`, starts timers for `UpdateHousingState` and `CheckAndInitiateConstruction`, and calls `InitializeFieldListeners` to subscribe to `AMS_WheatField` events.
    *   **Quest Generation (`GenerateQuestsForResourceType`, specific field event handlers)**:
        *   For general resources: If `CentralStorageInventory` is low, creates `FQuest` objects for gathering, calculates rewards, adds them to `AvailableQuests_`, starts a bid timer, and broadcasts `OnQuestAvailable`.
        *   For `AMS_WheatField` needs (planting, watering, harvesting): Responds to delegates from fields to generate appropriate quests.
        *   For construction: When `StartBuildingProject` is called, generates a series of delivery quests for the required materials.
    *   **Bidding Process**:
        *   `StartBidTimer(const FQuest& Quest)`: Initiates the bidding window for a quest.
        *   `ReceiveBid(AMS_AICharacter* Bidder, FQuest Quest, float BidValue)`: Collects bids from AI characters.
        *   `SelectQuestWinner_Internal(FGuid QuestID)` (Timer Callback): After `BidDuration`, evaluates bids, selects a winner (highest bid, tie-broken by timestamp), assigns the quest via `winner->AssignQuest()`, and moves the quest from `AvailableQuests_` to `AssignedQuests_`. If no valid winner, may re-post the quest.
    *   **Quest Completion (`RequestQuestCompletion`)**: Called by `AMS_AICharacter`. Verifies assignment, awards `Character->Money`, and cleans up the quest from `AssignedQuests_`.
    *   **Construction Management**:
        *   `CheckAndInitiateConstruction()`: Periodically evaluates if new buildings (Houses, Wheat Fields, Taverns) are needed based on simulation state (population, resources, happiness).
        *   `FindSuitableBuildLocation()`: Uses `PathfindingSubsystem` to find an empty 2x2 area for construction.
        *   `StartBuildingProject()`: Spawns an `AMS_ConstructionSite`, blocks its nodes in the pathfinding graph, and generates delivery quests for it.
        *   `NotifyConstructionProgress()`: Receives updates from `AMS_ConstructionSite` (currently not heavily used beyond logging).
    *   **Housing Management (`UpdateHousingState`)**: Periodically counts `CurrentPopulation` (instances of `AICharacterClass`) and `TotalHousingCapacity` (from all `AMS_House` instances). Assigns homeless AI to available houses. Triggers `CheckAndInitiateConstruction` if more housing is needed.
    *   **Field Listener Initialization (`InitializeFieldListeners`)**: Finds all existing `AMS_WheatField` actors and subscribes to their `OnFieldNeedsPlanting`, `OnFieldNeedsWatering`, and `OnFieldReadyToHarvest` delegates.

#### 2.3. Resource & Inventory System

##### 2.3.1. `UMS_InventoryComponent` (Actor Component)

*   **Role**: A generic component that can be attached to any actor to give it an inventory for resources and money. Used by `AMS_AICharacter` and `AMS_StorageBuilding`.
*   **Properties**:
    *   `Resources_` (TMap<ResourceType, int32>): Stores the quantity of each `ResourceType`. Initialized with all enum types and 0 quantity.
    *   `Money` (int32): Stores currency (primarily used on `AMS_AICharacter`).
*   **Key Functions**:
    *   `AddToResources(ResourceType Type, int32 NewAmount)`: Adds to the quantity of a resource. Broadcasts `OnResourceChanged`.
    *   `ExtractFromResources(ResourceType Type, int32 ExtractAmount)`: Attempts to remove resources. Returns amount extracted or -1 on failure. Broadcasts `OnResourceChanged`.
    *   `SetResource(ResourceType Type, int32 NewAmount)`: Directly sets the quantity of a resource. Broadcasts `OnResourceChanged`.
    *   `GetResourceAmount(ResourceType Type) const`: Returns the current quantity of a resource.
*   **Delegates**:
    *   `OnResourceChanged` (FOnResourceChanged): Broadcasts when a resource quantity changes, passing the `ResourceType` and the `NewAmount`.

##### 2.3.2. `ResourceType` (Enum)

*   **Definition**: `UENUM(BlueprintType)` defining the types of resources in the game: `ERROR`, `BERRIES`, `WOOD`, `WATER`, `WHEAT`.

##### 2.3.3. `FResource` (Struct)

*   **Definition**: A simple struct to bundle a `ResourceType` and an `int32 Amount`. Used, for example, when `AMS_BaseWorkPlace::TakeResources()` returns gathered items.

#### 2.4. Pawn Statistics (Needs) System

##### 2.4.1. `UMS_PawnStatComponent` (Actor Component)

*   **Role**: Manages the physiological and psychological needs of a pawn, typically an `AMS_AICharacter`.
*   **Properties (Stats)**:
    *   `Hunger`, `Thirst`, `Energy`, `Happiness` (float, 0-100): Current values of the needs.
    *   `HungerDecreaseRate`, `ThirstDecreaseRate`, etc. (float): Rate at which stats decrease per second.
    *   `HungryThreshold`, `ThirstThreshold`, etc. (float): Value below which the corresponding boolean flag (e.g., `Hungry`) becomes true.
    *   `Hungry`, `Thirsty`, `Tired`, `Sad` (bool): Flags indicating critical need states.
*   **Key Functionality**:
    *   `BeginPlay()`: Starts a timer that calls `DecreaseStats()` every second.
    *   `DecreaseStats()` (Timer Callback): Decreases each stat by its respective rate. Updates the boolean threshold flags (e.g., `Hungry`). Broadcasts `OnStateChanged`.
    *   `ModifyHunger(float Amount)`, `ModifyThirst(float Amount)`, etc.: Public functions to change stat values. Clamps values between 0 and 100. Broadcasts the specific stat's `OnStatChanged` delegate (e.g., `OnHungerChanged`).
    *   `GetHunger()`, `IsHungry()`, etc.: Accessors for stat values and threshold flags.
*   **Delegates**:
    *   `OnHungerChanged`, `OnThirstChanged`, etc. (FOnStatChanged): Broadcast when a specific stat's value changes.
    *   `OnStateChanged` (FOnStateChanged): Broadcast every second after `DecreaseStats` has updated all stats and threshold flags. `AMS_AICharacter` uses this to trigger `CheckIfHungry` and update its Blackboard.

#### 2.5. Time and Environment System

##### 2.5.1. `UMS_TimeSubsystem` (GameInstance Subsystem)

*   **Role**: Manages the global in-game clock, including current hour, day, and the progression of time. It also handles transitions between day and night.
*   **Properties**:
    *   `TimeScale` (float): Multiplier for how fast game time progresses relative to real-time (e.g., 60.0 means 1 real second = 1 game minute).
    *   `CurrentHour` (float, 0.0-23.99): The current hour of the game day.
    *   `CurrentDay` (int32): The current day number.
    *   `DayStartHour` (float): The hour at which daytime officially begins.
    *   `NightStartHour` (float): The hour at which nighttime officially begins.
    *   `bEnableOnScreenDebug` (bool): Toggles on-screen display of time information.
*   **Key Functionality**:
    *   **Time Progression (`Tick`)**:
        *   Advances `CurrentHour` based on `DeltaTime` and `TimeScale`.
        *   Handles `CurrentHour` wrapping around 24.0, incrementing `CurrentDay`.
        *   Detects integer hour changes and broadcasts `OnHourChanged`.
        *   Calls `UpdateCycleState()` to check for day/night transitions.
    *   **Day/Night Logic (`IsDayTime`, `IsNightTime`, `UpdateCycleState`)**:
        *   `IsDayTime()`: Returns true if `CurrentHour` falls within the defined daytime period (handles wrap-around if `DayStartHour` > `NightStartHour`).
        *   `UpdateCycleState()`: Compares current day/night status with the previous frame's. If a change is detected (or `bForceImmediateBroadcast` is true), it updates `bIsCurrentlyNight` and broadcasts `OnDayStart` or `OnNightStart` accordingly.
    *   **Console Commands**: Provides `exec` functions for debugging and controlling time:
        *   `TimeSystem_ToggleDebug`, `TimeSystem_SetScale`, `TimeSystem_SetHour`, `TimeSystem_AddHours`, `TimeSystem_SetDay`.
*   **Delegates**:
    *   `OnDayStart`, `OnNightStart`: Broadcast when the simulation transitions between day and night.
    *   `OnHourChanged`: Broadcasts when the integer part of `CurrentHour` changes.
    *   `OnDayChanged`: Broadcasts when `CurrentDay` increments.

##### 2.5.2. `AMS_SkyController` (Actor)

*   **Role**: Controls a `ADirectionalLight` actor in the scene to simulate the sun's movement, intensity, and color based on the time provided by `UMS_TimeSubsystem`.
*   **Properties**:
    *   `TargetDirectionalLight` (TObjectPtr<ADirectionalLight>): A reference to the Directional Light actor to be controlled. **Must be assigned in the editor.**
    *   Intensity/Color Parameters: `MinNightIntensity`, `MaxDayIntensity`, `DayColor`, `NightColor`.
    *   Rotation Parameters: `SunrisePitch`, `MiddayPitch`, `SunsetPitch`, `FixedSunYaw`, `FixedSunRoll`.
    *   Curve Assets: `IntensityCurve`, `ColorCurve`, `PitchCurve` (TObjectPtr<UCurveFloat/LinearColor>): Optional curves to define properties over a 24-hour cycle, overriding simple interpolation.
*   **Key Functionality**:
    *   `BeginPlay()`: Initializes `TimeSubsystem` reference and caches `DirectionalLightComponent` from `TargetDirectionalLight`.
    *   `Tick()`: If dependencies are valid, calls `UpdateSunlight()` with `TimeSubsystem->GetCurrentHour()`.
    *   `UpdateSunlight(float CurrentHour)`:
        *   Sets the `TargetDirectionalLight` actor's rotation using `CalculateSunRotation()`.
        *   Sets the `DirectionalLightComponent`'s intensity using `CalculateSunIntensity()`.
        *   Sets the `DirectionalLightComponent`'s color using `CalculateSunColor()`.
    *   `CalculateSunRotation/Intensity/Color()`: These functions determine the light's properties. If a corresponding Curve asset is provided, it uses the curve; otherwise, it performs linear interpolation between the defined day/night or sunrise/midday/sunset parameters.

#### 2.6. Quest System

##### 2.6.1. `FQuest` (Struct)

*   **Role**: Defines the data structure for a single quest or task.
*   **Properties**:
    *   `QuestID` (FGuid): A unique identifier for this quest instance, automatically generated.
    *   `Type` (ResourceType): The type of resource involved (e.g., `WOOD` for gathering, `WHEAT` for farming actions).
    *   `Amount` (int32): The quantity of the resource required. A special value of `-1` is used by `AMS_WheatField` quests to indicate a "planting" action where a specific amount isn't relevant, only the act of planting.
    *   `Reward` (int32): The monetary reward for completing the quest.
    *   `TargetDestination` (TWeakObjectPtr<AActor>): An optional target actor for the quest. Used for:
        *   Delivery quests (e.g., `AMS_ConstructionSite`).
        *   Field operation quests (e.g., `AMS_WheatField` to plant/water/harvest at).
        *   If `nullptr`, it's typically a general gathering quest where items are to be brought to central storage.

##### 2.6.2. `FBidInfo` (Struct)

*   **Role**: Stores information about a single bid made by an AI character for a quest.
*   **Properties**:
    *   `Bidder` (TWeakObjectPtr<AMS_AICharacter>): The AI character who made the bid.
    *   `BidValue` (float): The calculated value of the bid. Higher is better.
    *   `BidTimestamp` (float): The game time (seconds) when the bid was received, used for tie-breaking.

#### 2.7. Placeable Actors & Pools

These actors represent interactive elements or locations in the game world. Many are managed by "Pool" actors, though some pools are more about discovery/management of existing instances than dynamic spawning from a pre-allocated set.

##### 2.7.1. Workplaces (`MS_BaseWorkPlace`, `MS_WorkpPlacePool`, specific types)

*   **`AMS_BaseWorkPlace` (Actor)**:
    *   **Role**: Abstract base class for any location where AI can gather resources or perform work.
    *   **Properties**: `ResourceType_`, `ResourceAmount_`, `InfiniteResource_`, `RespawnTime_`, `ResourceAvaliable_`, `GridPosition_`.
    *   **Functionality**: `TakeResources()` (marks as unavailable if not infinite, starts respawn timer), `ResetWorkPlace()` (respawn callback).
    *   **Derived Classes**:
        *   `AMS_BushWorkPlace` (Berries)
        *   `AMS_TreeWorkPlace` (Wood)
        *   `AMS_WellWorkPlace` (Water)
        *   `AMS_WheatField` (inherits, but has much more specialized logic)
*   **`AMS_WorkpPlacePool` (Actor)**:
    *   **Role**: Manages the availability and spawning of `AMS_BaseWorkPlace` instances.
    *   **Functionality**:
        *   `FindWorkplacesOnScene()`: Locates pre-placed workplaces and adds them to `ActiveWorkplaces_`.
        *   Spawns an initial set of inactive workplaces from `WorkplaceClasses`.
        *   `SpawnWorkplaceAtRandomNode()` (Timer Callback): If `bSpawning` is true, periodically tries to reactivate an inactive workplace of a random type at a random free node obtained from `PathfindingSubsystem`. Blocks the original pathfinding node and adds the workplace as a new, distinct node.
        *   `RemoveWorkplaceAndFreeNode()`: Called when a workplace is depleted. Deactivates the workplace, moves it to `InactiveWorkplaces_`, and unblocks its original pathfinding node.

##### 2.7.2. Storage (`MS_StorageBuilding`, `MS_StorageBuildingPool`)

*   **`AMS_StorageBuilding` (Actor)**:
    *   **Role**: A building where resources can be stored and retrieved.
    *   **Components**: `Inventory_` (UInventoryComponent).
    *   **Properties**: `GridPosition_`, `placeActive_`.
*   **`AMS_StorageBuildingPool` (Actor)**:
    *   **Role**: Primarily discovers and manages references to `AMS_StorageBuilding` instances already placed in the level.
    *   **Functionality**: `FindStorageBuildingsOnScene()` (called after `CustomMovementPlugin`'s node map is ready) iterates all `AMS_StorageBuilding` actors, adds them to `StorageBuldings_`, sets them active, blocks nearby pathfinding nodes, and adds the storage as a new pathfinding node.
    *   Includes `DeactivateStorageBuilding` and `ReactivateStorageBuilding` for potential pooling, though current use is mainly discovery.

##### 2.7.3. Housing (`MS_House`)

*   **Role**: A building that provides shelter and a place for AI characters to sleep.
*   **Properties**: `MaxOccupants`, `CurrentOccupantCount`, `GridPosition_`, `OccupyingCharacters` (TArray of TWeakObjectPtr<AMS_AICharacter>).
*   **Functionality**: `TryEnterHouse()`, `LeaveHouse()`, `HasSpace()`. AI characters assigned a house will attempt to use it for sleeping.

##### 2.7.4. Construction (`MS_ConstructionSite`)

*   **Role**: A temporary actor representing a building project. AI deliver resources to it.
*   **Properties**: `BuildingClassToSpawn` (TSubclassOf<AActor> for the final building), `RequiredResource`, `AmountRequired`, `CurrentAmount`, `OccupiedNodes` (pathfinding nodes it covers).
*   **Functionality**:
    *   `AddResource(int32 AmountToAdd)`: Called by AI when delivering. Increments `CurrentAmount`. If `CurrentAmount >= AmountRequired`, calls `CompleteConstruction()`.
    *   `CompleteConstruction()`: Spawns `BuildingClassToSpawn` at its location and then destroys itself.
    *   `BeginDestroy()`: Intended to unblock `OccupiedNodes` (currently commented out in source, `AMS_AIManager` handles initial unblocking on spawn failure).

##### 2.7.5. Farming (`MS_WheatField`)

*   **Role**: A specialized workplace for growing and harvesting wheat. Inherits from `AMS_BaseWorkPlace` but has a complex state machine.
*   **States (`EFieldState`)**: `Constructed`, `Planted`, `Watered`, `Growing`, `ReadyToHarvest`, `Harvested`.
*   **Properties**: `CurrentState`, `WateredToGrowingDuration`, `GrowingToReadyDuration`, `HarvestAmount`, various `UStaticMesh` properties for visual state changes.
*   **Functionality**:
    *   `ChangeState(EFieldState NewState)`: Manages state transitions, updates mesh, starts/clears growth timers, and broadcasts delegates.
    *   `PerformPlanting()`, `PerformWatering()`, `PerformHarvesting()`: Called by AI (via `MS_PerformWorkAction` BTTask) to interact with the field. These trigger state changes.
    *   Timer Callbacks (`OnWateringGrowthTimerComplete`, `OnGrowingTimerComplete`): Advance the growth cycle.
*   **Delegates**: `OnFieldNeedsPlanting`, `OnFieldNeedsWatering`, `OnFieldReadyToHarvest`. `AMS_AIManager` subscribes to these to generate appropriate farming quests.

##### 2.7.6. Recreation (`MS_Tavern`)

*   **Role**: A building where AI can go to increase their Happiness stat.
*   **Properties**: `DrinkCost`, `HappinessGain`, `DrinkDuration` (for AI task), `InteractionPoint` (USceneComponent for AI to move to).
*   **Functionality**: AI characters (via `MS_BuyDrink` BTTask) can go to a tavern, pay `DrinkCost`, "drink" for `DrinkDuration`, and gain `HappinessGain`.

---

### 3. Setup and Configuration

#### 3.1. Plugin Installation

1.  Ensure `AlesAndFables` and its dependency `CustomMovementPlugin` are in your project's/engine's `Plugins` directory.
2.  Enable both plugins in **Edit > Plugins** and restart the editor if needed.

#### 3.2. Essential Actor Placement

Place one instance of each of the following actors into your persistent level:

*   **`AMS_AIManager`**: This is critical for the simulation to function.
*   **`AMS_MovementNodeMeshStarter`**: (From `CustomMovementPlugin`) For pathfinding.
*   **`AMS_StorageBuilding`**: At least one, to act as the Central Storage.
*   **`AMS_WorkpPlacePool`**: To manage dynamic workplaces.
*   **`AMS_StorageBuildingPool`**: To manage storage buildings.
*   **`AMS_SkyController`**: To control the day/night visual cycle.

#### 3.3. `AMS_AIManager` Configuration

Select the `AMS_AIManager` instance in your level and configure its properties in the **Details panel**. Key properties to set:

*   **AI Manager | Quests**:
    *   `Low Resource Threshold` (e.g., 50)
    *   `Max Resource Per Quest` (e.g., 15)
    *   `Managed Resource Types`: Add `WOOD`, `BERRIES`, `WATER`, `WHEAT`.
    *   `Bid Duration` (e.g., 3.0 seconds)
    *   `Delivery Carry Capacity` (e.g., 15)
*   **AI Manager | Construction**:
    *   `HouseBuildingClass`: Assign your `AMS_House` Blueprint.
    *   `ConstructionSiteClass`: Assign your `AMS_ConstructionSite` Blueprint.
    *   `WheatFieldClass`: Assign your `AMS_WheatField` Blueprint.
    *   `TavernBuildingClass`: Assign your `AMS_Tavern` Blueprint.
    *   `HouseWoodCost`, `WheatFieldWoodCost`, `TavernWoodCost` (e.g., 50, 25, 75).
    *   `MaxConcurrentConstruction` (e.g., 3).
    *   `HousingCheckInterval` (e.g., 5.0s).
    *   `TavernCheckInterval` (e.g., 5.0s).
    *   `MinAverageHappinessForTavernConsideration` (e.g., 40.0).
    *   `MaxTaverns` (e.g., 1).
*   **AI Manager | Population**:
    *   `AICharacterClass`: Assign your `AMS_AICharacter` Blueprint.

#### 3.4. `AMS_AICharacter` Blueprint Setup

1.  Create a new Blueprint Class derived from `AMS_AICharacter`.
2.  **Behavior Tree**: In the Class Defaults, assign a `BehaviorTree` asset to the `Behavior Tree_` property. This BT will control the AI's actions.
3.  **Mesh & Animation**: Set up the Skeletal Mesh, Animation Blueprint, etc., as per standard character setup.
4.  **Components (Defaults)**:
    *   `Inventory Component`: Default settings are usually fine.
    *   `Pawn Stat Component`: Adjust `DecreaseRate` and `Threshold` values if desired.
    *   `Widget Component`:
        *   `Widget Class`: Assign a `UserWidget` Blueprint designed to display pawn stats (e.g., `/Game/Dynamic/UI/UI_PawnStats` if you have one).
        *   `Widget Space`: Set to `World`.
    *   `ShopCollision`: Adjust its extent/shape as needed for interaction detection.

#### 3.5. `UMS_TimeSubsystem` Configuration

*   Access its default properties by finding `MS_TimeSubsystem.h` and modifying the constructor values, or by creating a C++ or Blueprint class derived from it if you need to expose settings to the editor easily via a Data Asset or Game Instance subclass.
*   Key Defaults:
    *   `TimeScale` (e.g., 600.0 for 1 real second = 10 game minutes)
    *   `DayStartHour` (e.g., 6.0 for 6 AM)
    *   `NightStartHour` (e.g., 20.0 for 8 PM)
    *   `bEnableOnScreenDebug` (true by default)

#### 3.6. `AMS_SkyController` Configuration

1.  Place an `AMS_SkyController` actor in the level.
2.  Select it, and in the **Details panel**:
    *   **Sky Controller | Setup**:
        *   `Target Directional Light`: Assign the `DirectionalLight` actor from your scene that represents the sun.
    *   **Sky Controller | Intensity / Color / Rotation**:
        *   Adjust `MinNightIntensity`, `MaxDayIntensity`, `DayColor`, `NightColor`, `SunrisePitch`, `MiddayPitch`, `SunsetPitch`.
    *   **Sky Controller | Advanced Curves**:
        *   Optionally, create `CurveFloat` assets for Intensity and Pitch, and a `CurveLinearColor` asset for Color. Assign these to `IntensityCurve`, `PitchCurve`, and `ColorCurve` respectively for more precise control over the sky's appearance throughout the 24-hour cycle. The X-axis of these curves represents the hour (0-24).

#### 3.7. Placeable Actor Blueprint Setup

For each of the following base C++ classes, create Blueprint children to allow for easy mesh assignment and property tweaking:

*   **`AMS_StorageBuilding`**: No specific properties beyond default inventory.
*   **`AMS_House`**: Set `MaxOccupants`. Assign a mesh.
*   **`AMS_ConstructionSite`**: The `AIManager` spawns this. Default mesh can be scaffolding. `BuildingClassToSpawn` etc. are set by AIManager.
*   **`AMS_WheatField`**:
    *   Assign static meshes for `MeshState_Constructed`, `MeshState_Planted`, `MeshState_Watered`, `MeshState_Growing`, `MeshState_ReadyToHarvest`, `MeshState_Harvested`.
    *   Set `WateredToGrowingDuration`, `GrowingToReadyDuration`, `HarvestAmount`.
    *   `ResourceType_` should be `WHEAT`.
*   **`AMS_Tavern`**: Set `DrinkCost`, `HappinessGain`, `DrinkDuration`. Assign a mesh. Position its `InteractionPoint` component where AI should stand.
*   **Workplaces (`AMS_BushWorkPlace`, `AMS_TreeWorkPlace`, `AMS_WellWorkPlace`)**:
    *   For each, set `ResourceType_` (e.g., `BERRIES`, `WOOD`, `WATER`).
    *   Set `ResourceAmount_` (amount gained per interaction).
    *   Set `InfiniteResource_` (e.g., true for a Well, false for a Bush/Tree that depletes).
    *   Set `RespawnTime_` if not infinite.
    *   Assign these Blueprint classes to `AMS_WorkpPlacePool`'s `WorkplaceClasses` array.

#### 3.8. Behavior Tree and Blackboard Setup

1.  **Blackboard Asset**: Create a new Blackboard asset. This will store the AI's working memory.
2.  **Behavior Tree Asset**: Create a new Behavior Tree asset and assign the Blackboard asset to it.
3.  **Key Blackboard Entries (ensure these types match the C++ expectations):**
    *   `SelfActor` (Object, Base Class: Actor) - Set by controller
    *   `Target` (Object, Base Class: Actor) - General movement/interaction target
    *   `bHasQuest` (Bool) - True if AI has an active quest
    *   `QuestType` (Enum, Enum Type: `ResourceType`) - Type of resource for current quest
    *   `QuestAmount` (Int) - Amount needed for current quest/trip
    *   `QuestReward` (Int) - Reward for current quest
    *   `QuestID` (String) - GUID of current quest
    *   `QuestTargetDestination` (Object, Base Class: Actor) - e.g., ConstructionSite for delivery
    *   `QuestResourceSource` (Object, Base Class: Actor) - e.g., Central Storage for fetching construction materials
    *   `bIsHungry` (Bool)
    *   `bIsThirsty` (Bool)
    *   `bIsSad` (Bool)
    *   `bIsSleeping` (Bool) - Set by `MS_SleepTask`
    *   `bGettingFood` (Bool) - AI's intent to get food (from storage or world)
    *   `bGettingWater` (Bool) - AI's intent to get water
    *   `AtWorkLocation` (Bool) - True if AI is at the `Target` workplace
    *   `bAtStorage` (Bool) - True if AI is at the `Target` storage
    *   `bAtConstructionSite` (Bool) - True if AI is at the `Target` construction site
    *   `bAtWheatField` (Bool) - True if AI is at the `Target` wheat field
    *   `bIsFetchingConstructionMaterials` (Bool) - State for multi-trip construction delivery
    *   `bIsDeliveringConstructionMaterials` (Bool) - State for multi-trip construction delivery
    *   `bIsStoringGatheredItems` (Bool) - State for generic gathering quest storage
    *   `TargetTavern` (Object, Base Class: Actor) - Specific target for tavern visit
    *   `TargetWorkplace` (Object, Base Class: Actor) - Specific target for needs fulfillment from world
    *   `TargetLocation` (Vector) - For random wander
    *   `bEmergencyFood` (Bool) - Flag if storage had no food
    *   `bEmergencyWater` (Bool) - Flag if storage had no water
    *   `bHasFoundHouse` (Bool) - If `MS_FindHouse` succeeded
    *   Many of these are initialized in `AMS_AICharacterController::OnPossess`.

---

### 4. Behavior Tree Integration

#### 4.1. Overview of Provided Custom Nodes

The plugin includes a variety of custom Behavior Tree nodes (Tasks, Decorators, Services) to build sophisticated AI. Refer to their individual header files for `UPROPERTY` details.

#### 4.2. Essential Blackboard Keys

See section 3.8.3 for a comprehensive list. The correct setup of these keys and their types is crucial for the BT nodes to function.

#### 4.3. Custom BT Tasks (Detailed)

*   **`FlipBoolTask`**:
    *   Purpose: Toggles the value of a boolean Blackboard key.
    *   Key Property: `BoolKey` (FBlackboardKeySelector).
*   **`MS_BuyDrink`**:
    *   Purpose: AI interacts with a Tavern to buy a drink and gain happiness. Latent task.
    *   Inputs (BB): `BlackboardKey_TargetTavern` (Actor - the `AMS_Tavern`).
    *   Actions: Checks AI `Money` against `Tavern->DrinkCost`. Deducts cost. Plays `DrinkingMontage`. After `Tavern->DrinkDuration`, modifies AI `Happiness` via `PawnStats_`.
    *   Properties: `DrinkingMontage`.
*   **`MS_ConsumeFromTarget`**:
    *   Purpose: AI directly consumes a resource from a workplace to satisfy a need (e.g., drinks water from a well).
    *   Inputs (BB): `BlackboardKey_TargetWorkplace` (Actor - an `AMS_BaseWorkPlace`).
    *   Actions: Calls `Workplace->TakeResources()`, then `AICharacter->ConsumeResourceDirectly()` (which updates `PawnStats_`). Removes the (non-infinite) workplace from the `WorkplacePool`.
*   **`MS_CopyBlackboardValue`**:
    *   Purpose: Copies an `UObject*` value from a source BB key to a destination BB key. Also calls `AICharacter->CreateMovementPath()` to the copied Actor.
    *   Key Properties: `SourceBlackboardKey`, `DestinationBlackboardKey`.
*   **`MS_FetchFromStorage`**:
    *   Purpose: For delivery quests (e.g., to a construction site). AI goes to storage, picks up items, then sets its target to the final destination.
    *   Inputs (BB): `BlackboardKey_TargetStorage` (Actor), `BlackboardKey_QuestType` (Enum), `BlackboardKey_QuestAmount` (Int - amount for *this trip*), `BlackboardKey_QuestTargetDestination` (Actor - e.g., construction site).
    *   Outputs (BB): Sets `BlackboardKey_IsFetchingMaterials` to false, `BlackboardKey_IsDeliveringMaterials` to true. Sets `BlackboardKey_NewMovementTarget` to `QuestTargetDestination`.
    *   Actions: Extracts resources from storage inventory, adds to AI inventory. Generates path to `QuestTargetDestination`.
*   **`MS_FindHouse`**:
    *   Purpose: Sets the AI's assigned `AMS_House` as the `Target` if the house is valid and has space.
    *   Outputs (BB): `BlackboardKey_Target` (Actor), `bHasFoundHouse` (Bool).
    *   Actions: Calls `AICharacter->GetAssignedHouse()`. Generates path if house found.
*   **`MS_FindNearestStorage`**:
    *   Purpose: Finds the closest *active* `AMS_StorageBuilding` from `AICharacter->StorageBuldingsPool_`.
    *   Outputs (BB): `BlackboardKey_TargetStorage` (Actor), and also sets the generic `Target` (Actor) key.
    *   Actions: Generates path to the found storage.
*   **`MS_FindNearestTavern`**:
    *   Purpose: Finds the closest `AMS_Tavern` in the world.
    *   Outputs (BB): `BlackboardKey_TargetPub` (Actor), and also sets the generic `Target` (Actor) key.
    *   Actions: Generates path to the found tavern.
*   **`MS_FindNearestWorkSite`**:
    *   Purpose: Finds the closest *available* `MS_BaseWorkPlace` that provides the `ResourceType` specified in the `QuestType` Blackboard key.
    *   Inputs (BB): `QuestType` (Enum).
    *   Outputs (BB): Sets the generic `Target` (Actor) key.
    *   Actions: Iterates `AICharacter->WorkPlacesPool_`. Generates path to the found worksite.
*   **`MS_FindRandomWanderLocation`**:
    *   Purpose: Finds a random, unblocked navigation node using `PathfindingSubsystem->GetRandomFreeNode()`.
    *   Outputs (BB): `BlackboardKey_TargetLocation` (Vector), clears `BlackboardKey_TargetActor`.
    *   Actions: Generates path to the random location.
*   **`MS_FindResourceForNeed`**:
    *   Purpose: Finds a `MS_BaseWorkPlace` that provides a *specific* resource type, configured directly in the task, to satisfy an immediate need (e.g., finding a well for thirst).
    *   Key Property: `NeededResourceType` (ResourceType Enum).
    *   Outputs (BB): `BlackboardKey_Target` (Actor).
    *   Actions: Generates path to the found resource.
*   **`MS_FollowNodePath`**:
    *   Purpose: Makes the AI character traverse the `Path_` (array of `FIntPoint`) stored on itself. Latent task.
    *   Actions: Iteratively calls `AAIController::MoveToLocation()` for each node in `AICharacter->Path_`. Succeeds when the last node is reached.
*   **`MS_GeneratePathToTarget`**:
    *   Purpose: A general task to make `AICharacter` generate a path to an actor specified in a Blackboard key.
    *   Inputs (BB): `BlackboardKey_TargetActor` (Actor).
    *   Actions: Calls `AICharacter->CreateMovementPath()`. Succeeds if path generated.
*   **`MS_GetBuildingLocation`**: (Seems specific/potentially redundant with `MS_GeneratePathToTarget`)
    *   Purpose: Assumes `Target` BB key holds a building (e.g., `AMS_ConstructionSite`) and generates a path to it.
    *   Inputs (BB): `Target` (Actor).
    *   Actions: Calls `AICharacter->CreateMovementPath()`.
*   **`MS_PerformWorkAction`**:
    *   Purpose: Core latent task for performing work at a target workplace (generic or `AMS_WheatField`).
    *   Inputs (BB): `BlackboardKey_WorkplaceTarget` (Actor), `BlackboardKey_IsAtTargetWorkplace` (Bool - a precondition, often set by overlap), `BlackboardKey_QuestType` (Enum), `BlackboardKey_QuestAmount` (Int).
    *   Actions:
        *   Plays an animation based on `WorkMontages` TMap (keyed by `QuestTypeBB`) or specific field montages (`PlantingMontage`, `WateringMontage`, `HarvestingMontage`). Falls back to `DefaultWorkMontage`.
        *   After `WorkDuration`:
            *   If target is `AMS_WheatField`: Calls `PerformPlanting()`, `PerformWatering()` (checks AI inventory for water), or `PerformHarvesting()` based on field state and quest details.
            *   If target is generic `AMS_BaseWorkPlace`: Calls `TakeResources()`.
            *   If successful, adds received resources to AI inventory.
            *   If it was a planting or watering field quest, calls `AICharacter->CompleteCurrentQuest()`.
            *   If it was a generic workplace, removes it from `WorkplacePool`.
        *   Clears `BlackboardKey_IsAtTargetWorkplace`.
*   **`MS_SleepTask`**:
    *   Purpose: AI enters a sleep state until daytime. Latent task.
    *   Outputs (BB): Sets `BlackboardKey_IsSleeping` to true during execution, clears on finish/abort.
    *   Actions: Caches `UMS_TimeSubsystem`. In `TickTask`, checks `TimeSubsystem->IsDayTime()`. If day, finishes. Restores AI energy (`PawnStats_->ModifyEnergy()`) while sleeping.
*   **`MS_StoreGatheredItems`**:
    *   Purpose: AI deposits all items of a specific type (from a completed gathering quest) into a target storage.
    *   Inputs (BB): `BlackboardKey_TargetStorage` (Actor), `BlackboardKey_QuestType` (Enum).
    *   Outputs (BB): Clears `BlackboardKey_IsStoringItems`.
    *   Actions: Transfers all of `QuestType` from AI inventory to storage inventory. Calls `AICharacter->CompleteCurrentQuest()`.

#### 4.4. Custom BT Decorators (Detailed)

*   **`MS_BTDecorator_IsNightTime`**:
    *   Purpose: Checks if it's currently night according to `UMS_TimeSubsystem`.
    *   Key Property: `bIsInversed` (bool) - Inverts the condition if true.
    *   Flow Control: `bAllowAbortChildNodes = true`, `FlowAbortMode = EBTFlowAbortMode::Self`. This means if time changes from night to day (or vice-versa) while this branch is active, it can abort.
*   **`MS_BTDecorator_NeedToGatherItems`**:
    *   Purpose: Checks if the AI has an active quest and its inventory amount for the `QuestType` is less than the `QuestAmount`.
    *   Inputs (BB): `BlackboardKey_HasQuest` (Bool), `BlackboardKey_QuestType` (Enum), `BlackboardKey_QuestAmount` (Int).
*   **`MS_CanAfford`**:
    *   Purpose: Checks if `AICharacter->Money` is greater than or equal to a cost.
    *   Key Properties: `LiteralCost` (Int), `BlackboardKey_Cost` (FBlackboardKeySelector for Int). If BB key is set, it overrides `LiteralCost`.
*   **`MS_HasEnoughResource`**:
    *   Purpose: Checks if `AICharacter->Inventory_` has at least `AmountNeeded` of `ResourceTypeToCheck`.
    *   Key Properties: `ResourceTypeToCheck` (Enum), `BlackboardKey_ResourceTypeToCheck` (FBlackboardKeySelector for Enum), `AmountNeeded` (Int), `BlackboardKey_AmountNeeded` (FBlackboardKeySelector for Int). BB keys override literal properties if set.
*   **`MS_NeedToStoreOrDeliver`**:
    *   Purpose: Checks conditions for either storing general gathered items or delivering items for a specific quest.
    *   Key Property: `bCheckForDelivery` (bool).
        *   If `true` (Delivery): Checks `HasQuest` AND `Inventory >= QuestAmount` AND `QuestTargetDestination` (BB Object) is SET.
        *   If `false` (Storage): Checks `HasQuest` AND `Inventory >= QuestAmount` AND `QuestTargetDestination` (BB Object) is NOT SET.
    *   Inputs (BB): `BlackboardKey_HasQuest`, `BlackboardKey_QuestType`, `BlackboardKey_QuestAmount`, `BlackboardKey_QuestTargetDestination`.

#### 4.5. Custom BT Services (Detailed)

*   **`MS_BTService_UpdateIdleStatus`**:
    *   Purpose: Periodically updates a designated "IsIdle" boolean Blackboard key.
    *   Inputs (BB): `BlackboardKey_HasQuest` (Bool), `BlackboardKey_IsSleeping` (Bool), `BlackboardKey_IsHungry` (Bool), `BlackboardKey_IsThirsty` (Bool).
    *   Outputs (BB): Writes to the key specified in `BlackboardKey` (inherited from `UBTService_BlackboardBase`).
    *   Logic: Sets the output key to `true` if `!bHasQuest && !bIsSleeping && !bIsHungry && !bIsThirsty`. Otherwise, `false`.

---

### 5. Key AI Workflows & Mechanics

#### 5.1. Quest Lifecycle

1.  **Generation**:
    *   `AMS_AIManager` monitors `CentralStorageBuilding` inventory. If a `ManagedResourceType` drops below `LowResourceThreshold`, `GenerateQuestsForResourceType` creates a gathering `FQuest`.
    *   `AMS_AIManager` responds to `AMS_WheatField` delegates (`OnFieldNeedsPlanting`, `OnFieldNeedsWatering`, `OnFieldReadyToHarvest`) by creating specific field operation quests.
    *   When `AMS_AIManager` starts a construction project via `StartBuildingProject`, it creates delivery `FQuest`s targeting the new `AMS_ConstructionSite`.
2.  **Bidding**:
    *   `AIManager` adds new quests to `AvailableQuests_`, starts a bid timer (`StartBidTimer`), and broadcasts `OnQuestAvailable`.
    *   `AMS_AICharacter` instances receive this via `OnNewQuestReceived`.
    *   If idle (checked by `IsIdle()`, which looks at its own Blackboard state influenced by `MS_BTService_UpdateIdleStatus`), the AI calls `EvaluateQuestAndBid`.
    *   `CalculateBidValue` considers distance (estimated Manhattan to resource and then to storage/destination), current needs penalties (more needy = lower bid), and quest reward.
    *   The AI submits the bid to `AIManager::ReceiveBid`.
3.  **Assignment**:
    *   `AIManager::SelectQuestWinner_Internal` (timer callback) picks the highest bidder (earliest bid is tie-breaker).
    *   The winner's `AssignQuest(FQuest)` is called.
    *   `AssignQuest` on `AMS_AICharacter` updates its internal `AssignedQuest` and sets relevant Blackboard keys: `bHasQuest=true`, `QuestType`, `QuestAmount`, `QuestReward`, `QuestID`, `QuestTargetDestination`, `QuestResourceSource` (if applicable, e.g., for construction delivery, source is central storage).
4.  **Execution (driven by Behavior Tree using Blackboard data)**:
    *   **Gathering Quest**:
        *   BT finds a worksite (`MS_FindNearestWorkSite` using `QuestType`).
        *   Moves to it (`MS_FollowNodePath`).
        *   Performs work (`MS_PerformWorkAction`).
        *   If inventory full or enough gathered for quest, finds storage (`MS_FindNearestStorage`).
        *   Moves to storage.
        *   Stores items (`MS_StoreGatheredItems`).
    *   **Delivery Quest (e.g., Construction)**:
        *   BT sets target to `QuestResourceSource` (Central Storage).
        *   Moves to storage.
        *   Fetches items (`MS_FetchFromStorage`): transfers from storage to AI inv, sets new target to `QuestTargetDestination` (Construction Site), updates BB state flags.
        *   Moves to `QuestTargetDestination`.
        *   Delivers items (handled in `AMS_AICharacter::OnOverlapBegin` with `AMS_ConstructionSite`).
    *   **Field Operation Quest (Plant, Water, Harvest)**:
        *   BT sets target to `QuestTargetDestination` (the `AMS_WheatField`).
        *   Moves to field.
        *   If watering, may first need to go to storage/well to get water.
        *   Performs work (`MS_PerformWorkAction` which calls specific field methods).
        *   If harvesting, then proceeds like a gathering quest to store the harvested wheat.
5.  **Completion**:
    *   After the final action of a quest (storing gathered items, delivering to site, successfully planting/watering), `AMS_AICharacter::CompleteCurrentQuest()` is called.
    *   This notifies `AIManager::RequestQuestCompletion()`.
    *   `AIManager` verifies, awards `Character->Money`, and removes the quest from `AssignedQuests_`.
    *   `AMS_AICharacter` clears its `AssignedQuest` and related Blackboard keys (`bHasQuest=false`).

#### 5.2. Needs Management & Fulfillment

1.  `UMS_PawnStatComponent::DecreaseStats` (timer) reduces Hunger, Thirst, Energy, Happiness. Updates threshold flags (`bIsHungry`, etc.). Broadcasts `OnStateChanged`.
2.  `AMS_AICharacter::CheckIfHungry` (handler for `OnStateChanged`) updates Blackboard keys: `bIsHungry`, `bIsThirsty`, `bIsSad`, and intent flags `bGettingFood`, `bGettingWater`.
3.  **Behavior Tree Response (Example for Hunger)**:
    *   A high-priority branch in BT checks `bIsHungry`.
    *   If true, AI decides to get food.
    *   **Option 1: From Storage**:
        *   `MS_FindNearestStorage` sets target.
        *   Move to storage.
        *   In `AMS_AICharacter::OnOverlapBegin` with `AMS_StorageBuilding`, if `bGettingFood` is true:
            *   Attempt to `ExtractFromResources(ResourceType::BERRIES, ...)` from storage inventory.
            *   If successful, `PawnStats_->ModifyHunger(100)`.
            *   Clear `bGettingFood`.
            *   If storage is out of food, set `bEmergencyFood` to true.
    *   **Option 2: From World (if `bEmergencyFood` or configured preference)**:
        *   `MS_FindResourceForNeed` (configured for `ResourceType::BERRIES`) sets target workplace.
        *   Move to workplace.
        *   `MS_ConsumeFromTarget` directly consumes from the workplace and updates stats.
    *   Similar logic applies for `bIsThirsty` and `bGettingWater` (using `ResourceType::WATER`).
4.  **Energy (Sleep)**: See 5.6.
5.  **Happiness (Recreation)**: See 5.7.

#### 5.3. Resource Gathering & Storage

*   Driven by gathering quests (see 5.1).
*   AI uses `MS_FindNearestWorkSite` to locate a `MS_BaseWorkPlace` of the `QuestType`.
*   `MS_PerformWorkAction` is used to gather resources, which are added to `AICharacter->Inventory_`.
*   Once enough items are gathered (or inventory is full), the AI uses `MS_FindNearestStorage`.
*   `MS_StoreGatheredItems` transfers items from AI inventory to the storage's inventory and completes the quest.

#### 5.4. Construction Projects

1.  **Initiation**: `AMS_AIManager::CheckAndInitiateConstruction()` decides to build (e.g., a House if `CurrentPopulation > TotalHousingCapacity`).
2.  `FindSuitableBuildLocation()` finds a 2x2 free area using `PathfindingSubsystem`.
3.  `StartBuildingProject()`:
    *   Blocks the 2x2 pathfinding nodes under the site.
    *   Spawns an `AMS_ConstructionSite` actor, configured with `BuildingClassToSpawn`, `RequiredResource`, and `ResourceCost`.
    *   Generates one or more delivery quests (max `DeliveryCarryCapacity` per trip) targeting this `AMS_ConstructionSite`.
4.  **Execution**: AI characters bid for and execute these delivery quests (see 5.1).
    *   `MS_FetchFromStorage` is used to get materials from `CentralStorageBuilding`.
    *   AI moves to the `AMS_ConstructionSite`.
    *   In `AMS_AICharacter::OnOverlapBegin` with the site:
        *   `ConstructionSite->AddResource()` is called.
        *   AI calls `CompleteCurrentQuest()` (for this delivery trip).
5.  **Completion**: When `ConstructionSite->CurrentAmount >= AmountRequired`:
    *   `ConstructionSite->CompleteConstruction()` is called.
    *   The final building (e.g., `AMS_House`) is spawned.
    *   The `AMS_ConstructionSite` actor is destroyed.
    *   **Important**: The pathfinding nodes occupied by the construction site (and now the final building) *remain blocked*. The final building itself might then register as a new, specific pathfinding target via `PathfindingSubsystem::AddNodeAtPosition()`.

#### 5.5. Farming Cycle (`AMS_WheatField`)

1.  **Construction**: `AMS_AIManager` can initiate construction of a `WheatFieldClass`. `AMS_ConstructionSite` spawns `AMS_WheatField`. The field calls `AIManager->InitializeFieldListeners()` so manager knows about it.
2.  **Planting**:
    *   `AMS_WheatField` starts in `EFieldState::Constructed`, broadcasts `OnFieldNeedsPlanting`.
    *   `AIManager` creates a planting quest (Type: `WHEAT`, Amount: -1, Target: the field).
    *   AI executes quest: `MS_PerformWorkAction` calls `WheatField->PerformPlanting()`. Field state -> `Planted`.
3.  **Watering**:
    *   Field broadcasts `OnFieldNeedsWatering`.
    *   `AIManager` creates watering quest (Type: `WATER`, Amount: 1, Target: field).
    *   AI executes quest:
        *   May need to fetch water first from storage/well (another sub-quest or BT branch).
        *   `MS_PerformWorkAction` calls `WheatField->PerformWatering()`. AI's water is consumed. Field state -> `Watered`.
4.  **Growth**:
    *   `AMS_WheatField` starts `WateredToGrowingDuration` timer. On completion -> `Growing` state.
    *   Starts `GrowingToReadyDuration` timer. On completion -> `ReadyToHarvest` state. Meshes update visually.
5.  **Harvesting**:
    *   Field broadcasts `OnFieldReadyToHarvest`.
    *   `AIManager` creates harvesting quest (Type: `WHEAT`, Amount: `Field->HarvestAmount`, Target: field).
    *   AI executes quest: `MS_PerformWorkAction` calls `WheatField->PerformHarvesting()`. Wheat added to AI inventory. Field state -> `Harvested`.
6.  **Cycle Restart**: `Harvested` state transitions to `Constructed`, broadcasting `OnFieldNeedsPlanting` again.
7.  Quest completion for plant/water happens immediately after the action. Harvest quest completes after storing the wheat.

#### 5.6. Housing and Sleep

1.  `AMS_AIManager::UpdateHousingState` (timer) assigns `AMS_AICharacter` instances to available `AMS_House` instances if they are homeless.
2.  **Behavior Tree (Nighttime)**:
    *   `MS_BTDecorator_IsNightTime` becomes true.
    *   AI may decide to sleep (especially if `PawnStats_->Energy` is low).
    *   `MS_FindHouse` task: Sets `AICharacter->MyHouse` as `Target` (Blackboard).
    *   AI moves to its house.
    *   `MS_SleepTask`:
        *   `House->TryEnterHouse(AICharacter)` (implicitly, AI should be at house location).
        *   Sets `bIsSleeping` BB key to true.
        *   Task remains latent, ticking and restoring energy via `PawnStats_->ModifyEnergy()`.
        *   When `TimeSubsystem->IsDayTime()` becomes true, task succeeds.
        *   `House->LeaveHouse(AICharacter)` (implicitly when sleep ends/aborts).
        *   Clears `bIsSleeping`.

#### 5.7. Recreation (Tavern Visits)

1.  `AMS_AIManager::CheckAndInitiateConstruction` may decide to build a Tavern if average AI happiness (`PawnStats_->GetHappiness()`) is below `MinAverageHappinessForTavernConsideration` and `MaxTaverns` limit not reached.
2.  **Behavior Tree (Low Happiness)**:
    *   If AI `PawnStats_->IsSad()` is true (or happiness below a threshold).
    *   `MS_FindNearestTavern` task: Sets `Target` and `TargetTavern` (Blackboard) to the closest `AMS_Tavern`.
    *   AI moves to the tavern's `InteractionPoint`.
    *   `MS_CanAfford` decorator checks if AI has `Tavern->DrinkCost`.
    *   `MS_BuyDrink` task:
        *   AI `Money -= Tavern->DrinkCost`.
        *   Task is latent for `Tavern->DrinkDuration`.
        *   `PawnStats_->ModifyHappiness(Tavern->HappinessGain)`.
        *   Task succeeds.

---

### 6. C++ API Reference (Key Classes & Functions)

Refer to the individual header files for full API details. This section highlights some of the most central classes and their roles.

*   **`AMS_AICharacter`**: Core AI pawn. See properties and functions related to quest handling, pathing, and interactions.
*   **`AMS_AIManager`**: Central simulation coordinator. See functions for quest generation, bidding, assignment, construction initiation.
*   **`UMS_InventoryComponent`**: `AddToResources`, `ExtractFromResources`, `GetResourceAmount`.
*   **`UMS_PawnStatComponent`**: `ModifyHunger/Thirst/Energy/Happiness`, `IsHungry/Thirsty/Tired/Sad`.
*   **`UMS_TimeSubsystem`**: `IsDayTime`, `GetCurrentHour`, delegates like `OnDayStart`, `OnNightStart`.
*   **`AMS_WheatField`**: `PerformPlanting/Watering/Harvesting`, `ChangeState`, delegates.
*   **`AMS_ConstructionSite`**: `AddResource`, `CompleteConstruction`.
*   **Behavior Tree Nodes**: Each custom task, decorator, and service has its `ExecuteTask`/`CalculateRawConditionValue`/`TickNode` implementation and `UPROPERTY`s for configuration.

---

### 7. Extensibility and Customization

*   **New AI Behaviors**: Create new BT Tasks, Decorators, and Services. Add new Blackboard keys as needed.
*   **New Resources/Needs**:
    *   Add to `ResourceType` enum.
    *   Update `UMS_InventoryComponent` defaults if necessary.
    *   Add new stats/rates/thresholds to `UMS_PawnStatComponent`.
    *   Create new workplaces for new resources.
    *   Update `AMS_AIManager` to manage new resources if desired.
*   **New Buildings**: Derive from `AActor` (or relevant base like `AMS_BaseWorkPlace`).
    *   Implement interaction logic.
    *   Add `TSubclassOf` to `AMS_AIManager` and logic for its construction conditions in `CheckAndInitiateConstruction`.
    *   Update AI Behavior Trees to interact with the new building type.
*   **Quest Variations**: Modify `FQuest` or create derived structs. Adjust `AMS_AIManager` generation and `AMS_AICharacter` bid calculation/execution logic.
*   **UI**: Create/modify `UserWidget` Blueprints for `AMS_AICharacter`'s `WidgetComponent` or for global simulation status.

---

### 8. Debugging and Troubleshooting

*   **`CustomMovementPlugin` Debug**: Enable `bShowDebugLinesPathfinding` and `bShowDebugLinesStarter` for pathing issues.
*   **`AlesAndFables` Debug**:
    *   `UMS_TimeSubsystem::bEnableOnScreenDebug`: Shows current game time.
    *   `UE_LOG` statements are prevalent throughout the plugin. Check the Output Log extensively. Filter by categories like `LogTemp`, or specific log messages.
    *   **Unreal Gameplay Debugger** (Apostrophe key): Inspect Behavior Tree execution, Blackboard values for selected AI.
    *   **Visual Logger**: Can provide a timeline of AI actions and state changes.
*   **Common Issues**:
    *   **AI Not Moving**: Check `CustomMovementPlugin` setup (Floor tag, collision, `AMS_MovementNodeMeshStarter` presence). Ensure BT pathing tasks are executing and `AICharacter->Path_` is being populated.
    *   **AI Not Taking Quests**:
        *   Is `AMS_AIManager` generating quests? Check logs.
        *   Is AI idle? Check `MS_BTService_UpdateIdleStatus` and its inputs.
        *   Are bid values too low or AI failing `CalculateBidValue`? Check logs.
    *   **Construction Not Starting**:
        *   Is `AMS_AIManager` configured with building classes?
        *   Are resource costs met? (Though AIManager doesn't explicitly check storage *before* starting a construction site, it generates quests after).
        *   Is `PathfindingSubsystem` returning valid free locations?
    *   **Needs Not Being Met**:
        *   Is `UMS_PawnStatComponent` ticking and decreasing stats?
        *   Is `CheckIfHungry` updating Blackboard correctly?
        *   Does the BT have logic to handle needs? Are there available resources/workplaces?
*   **Breakpoints**: Use C++ breakpoints in core logic functions to step through execution.

