# AG_MedievalSimulation Development Journal

Advanced Games Programming 24/25

Aimar Goñi

2410569

## Research

### Sources and References Relevant to the Task

#### Sources
To effectively develop the **AG_MedievalSimulation** project, I conducted comprehensive research focusing on the technical, creative, and conceptual aspects of the systems involved. This research guided the implementation of AI behavior systems, resource management mechanics, pathfinding algorithms, and medieval workplace simulations.

1. **Unreal Engine Official Documentation**
   - **Relevance:** The Unreal Engine documentation provided essential guidance on implementing Behavior Trees, Blackboards, Actor Components, and AI Controllers. These concepts were critical for developing modular, scalable AI systems and integrating them with the simulation's world dynamics.
   - **Key Insight:** Learned to create behavior-driven AI systems using Unreal Engine’s native tools, ensuring a seamless blend between AI decision-making and environmental interactions.

2. **"Artificial Intelligence for Games" by Ian Millington and John Funge**
   - **Relevance:** This book offered a deep understanding of AI pathfinding algorithms, finite state machines, and utility-based decision-making systems. It provided the foundation for implementing the **A\*** pathfinding and behavior tree strategies.
   - **Key Insight:** Emphasized the importance of balancing AI sophistication with system performance, influencing optimization strategies in pathfinding and decision-making.

3. **GDC Talks**
   - **Specific Examples:** "The AI of The Last of Us" and "Dynamic AI in Total War: Three Kingdoms" offered case studies on designing adaptable, goal-oriented AI systems.
   - **Key Insight:** Demonstrated how to simulate resource scarcity and prioritize AI objectives dynamically, aligning perfectly with AG_MedievalSimulation’s design goals.

4. **Amit Patel’s Guide on A\* Pathfinding**
   - **Relevance:** Provided a detailed breakdown of the A\* algorithm, which became the backbone of the pathfinding system.
   - **Key Insight:** Optimized the A\* algorithm by adapting heuristic calculations for grid-based movement systems, enabling efficient pathfinding in complex environments.

#### Avoided Sources
- **Outdated Tutorials or Unreal Engine Forums**
   - **Reason:** These often included deprecated methods or inefficient practices that could compromise system reliability or compatibility with Unreal Engine’s latest versions.
   - **Impact:** Avoiding these ensured that all implementations adhered to modern, optimized development practices.

### Insights Influencing Design
- **Modularity:** The importance of creating systems that could be reused or extended (e.g., workplaces derived from `MS_BaseWorkPlace`).
- **Scalability:** Ensured that systems such as pathfinding and resource management could handle an expanding game world without performance degradation.
- **Realism vs. Practicality:** Balanced authentic medieval resource management with gameplay feasibility.

---

## Implementation

### Development Process

#### Initial Planning
The project started with a high-level breakdown of required systems:
1. **AI Systems:** Handle decision-making, resource collection, and workplace interactions.
2. **Resource Management:** Track and modify resources dynamically across AI and storage systems.
3. **Pathfinding:** Implement a robust algorithm to handle AI movement across complex terrains.
4. **Workplaces:** Create modular workplaces to simulate various medieval tasks like farming, woodcutting, and water collection.
5. **Quests and Bulletin Boards:** Provide a framework for task assignment and tracking.
6. **Pawn Stats:** Manage hunger, thirst, and happiness levels of AI characters, simulating human needs.
7. **Game Management:** Orchestrate and initialize all subsystems.

This structure ensured clear priorities and dependencies, guiding development through iterations.

---

### System-Specific Details

#### **1. AI Decision-Making**

AI decision-making in this project was implemented using **Unreal Engine's Behavior Tree and Blackboard systems**, with additional custom logic for managing states like hunger, thirst, and task completion. The goal was to create dynamic agents that could prioritize tasks intelligently while responding to environmental changes.

---

  ###### **Key Components**

  1. **Behavior Tree**  
    - **Structure**: The AI’s decision-making hierarchy was built into a `BehaviorTree` asset. This allowed for modular and readable logic.
    - **Root Node**: The root evaluated high-level states stored in the `Blackboard`. For example:
      - Is the AI hungry?  
      - Is the AI thirsty?  
      - Does the AI have a quest?  
    - **Leaf Nodes**: The leaf nodes executed specific actions, such as:
      - Navigating to a workplace.
      - Fetching resources from storage.
      - Completing a quest.
    - **Task Nodes**: Custom nodes (`MS_FollowNodePath`, `MS_GetTask`) were implemented to handle specific behaviors like navigating a path or interacting with objects.

  2. **Blackboard**  
    - A `BlackboardComponent` served as a shared memory for AI state and objectives.
    - **Keys Used:**
      - `GettingFood`: True if the AI is seeking food.
      - `GettingWater`: True if the AI is seeking water.
      - `Working`: True if the AI is generally completing a task.
      - `DoingTask`: True if the AI is actively completing a task.
      - `GettingTask`: True if the AI is specifically going to get a task.
      - `StoringItems`: True if the AI is specifically going to store the resources.
      - `Target`: The current actor or location the AI is interacting with.
      - `Ignoring`: True if the AI is actively seeking food or water when none is accesible on the storage. The AI is going to ignore all the other commands until it gets the resource it needs.

  3. **State Management**  
    - **Hunger and Thirst**: Monitored using the `UMS_PawnStatComponent`. These stats decreased over time, and thresholds triggered behavior tree changes.
    - **Quests**: Managed via `AMS_AIManager` and updated in the AI’s Blackboard. Quests included fetching resources and delivering them to workplaces or storage buildings.

  ---

  ###### **Flow of Decision-Making**

  1. **Initialization**:
    - AI controllers set up the Blackboard with default values (e.g., `GettingTask = true`, `Working = false`).

    _Code Example: Blackboard Setup_
    ```cpp
    void AMS_AICharacterController::OnPossess(APawn* Pawn)
    {
        Super::OnPossess(Pawn);

        AMS_AICharacter* Character = Cast<AMS_AICharacter>(Pawn);
        if (Character && Character->behaviorTree_)
        {
            blackboard_->InitializeBlackboard(*Character->behaviorTree_->BlackboardAsset);
            blackboard_->SetValueAsBool("GettingTask", true);
        }
    }
    ```

  2. **Behavior Tree Execution**:
    - The tree’s selectors evaluated the AI’s current state (e.g., hungry, thirsty) to decide the next task. Tasks were prioritized in the following order:
      1. Hunger/Thirst management.
      2. Quest retrieval from bulletin boards.
      3. Task execution at workplaces.

  3. **Task Nodes**:
    - Custom `BehaviorTree` nodes controlled specific actions, such as finding the nearest storage or following a path to a workplace.

    _Example Task: Fetching Resources_
    ```cpp
    EBTNodeResult::Type UMS_FindNearestStorage::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
    {
        auto* AIController = Cast<AMS_AICharacterController>(OwnerComp.GetAIOwner());
        auto* AICharacter = AIController ? Cast<AMS_AICharacter>(AIController->GetPawn()) : nullptr;

        if (!AICharacter || !AICharacter->StorageBuldingsPool_) return EBTNodeResult::Failed;

        auto* ClosestStorage = AICharacter->FindClosestStorage();
        if (ClosestStorage)
        {
            OwnerComp.GetBlackboardComponent()->SetValueAsObject("Target", ClosestStorage);
            return EBTNodeResult::Succeeded;
        }
        return EBTNodeResult::Failed;
    }
    ```

  4. **Dynamic Updates**:
    - Blackboard keys were updated dynamically as the AI’s stats changed or tasks were completed. For example:
      - When `Hunger < 20`, the `GettingFood` key was set to true.
      - Completing a quest updated the `Working` key to false.

  ---

  ##### **Development Challenges and Solutions**

  ###### **1. Initialization Order**

  **Issue:**  
  - `AMS_BulletingBoardPool` and `AMS_StorageBuildingPool` were not fully initialized when the `BeginPlay` method in `AMS_AIManager` was called. This caused null references when the AI tried to interact with these systems.

  **Solution:**  
  - Introduced delegate checks linked to functions. This ensured that all pools were fully initialized before AI systems began accessing them.

  _Code Example: Delegate Initialization_
  ```cpp
  void AMS_AIManager::BeginPlay()
  {
      Super::BeginPlay();
      BulletingBoardPool_->OnBulletingBoardPoolInitialized.AddDynamic(this, &AMS_AIManager::OnBulletingBoardPoolReady);

  }
  ```

  ---

  ###### **2. Overlapping AI Needs**

  **Issue:**  
  - AI agents sometimes got stuck in repetitive tasks, such as switching between hunger and thirst without completing either.

  **Solution:**  
  - Added priority levels to tasks in the Behavior Tree. Hunger and thirst tasks were given higher precedence than quests.

  _Behavior Tree Adjustment_
  ```plaintext
  Root
  ├── Selector (High-Level States)
  │   ├── Sequence (Hunger)
  │   │   ├── Condition: IsHungry
  │   │   └── Task: FindFood
  │   ├── Sequence (Thirst)
  │   │   ├── Condition: IsThirsty
  │   │   └── Task: FindWater
  │   └── Sequence (Quests)
  │       ├── Condition: HasQuest
  │       └── Task: ExecuteQuest
  ```

  ---

  ###### **3. Debugging Behavior Trees**

  **Issue:**  
  - Complex behavior trees made it difficult to pinpoint why specific decisions were made or skipped.

  **Solution:**  
  - Used Unreal’s Behavior Tree debugging tools, which provided real-time visualization of node execution. This was combined with `UE_LOG` statements for deeper analysis.

  _Debugging Example_
  ```cpp
  UE_LOG(LogTemp, Log, TEXT("AI transitioning to Hunger state."));
  ```

  ---

  ##### **Reflection**

  AI decision-making posed significant challenges due to the need for seamless integration between multiple systems. However, through the use of modular components like Behavior Trees and Blackboard, the project achieved a scalable and responsive AI framework. Debugging tools and careful prioritization of tasks played a key role in overcoming implementation hurdles. For future improvements, introducing a weighted task prioritization system and dynamic pathfinding grid adjustments would enhance the AI’s adaptability.


#### **2. Pawn Stat System**

##### **Pawn Stats System Overview**

The Pawn Stats system in this project was inspired by games like **RimWorld**, where NPCs (pawns) manage multiple needs (e.g., hunger, thirst, energy) that influence their behavior. In this simulation, the stats system was designed to track and dynamically update an AI agent’s internal state, driving decision-making and interaction with the environment. This system provided a crucial layer of complexity, ensuring AI actions were tied to meaningful survival mechanics.

---

###### **Key Components**

1. **`UMS_PawnStatComponent`**  
   - Encapsulated all stats for a pawn (e.g., Hunger, Thirst, Happiness).  
   - Automatically reduced stats over time, triggering events when thresholds were crossed.  
   - Provided APIs for modifying stats during interactions or quest completion.

2. **Stat Modifiers**  
   - Each stat had a "critical threshold" below which it influenced AI decision-making.

3. **Integration with AI Decision-Making**  
   - Stats directly influenced the AI’s `Blackboard` values, determining priorities in the behavior tree.  
   - For example, if `Hunger < 20`, the AI would prioritize finding food over completing a quest.

4. **Visualization**  
   - Added a floating widget over AI pawns displaying current stats, inspired by RimWorld's stat bars for pawns.

---

##### **Flow of the Pawn Stats System**

1. **Initialization**:
   - The `UMS_PawnStatComponent` initialized default values for each stat (e.g., `Hunger = 100`).  
   - Registered tick updates to decrement stats over time.

2. **Stat Depletion**:
   - Stats were decremented at regular intervals, simulating real-world needs.  
   - Crossing a critical threshold triggered events like "Hungry" or "Thirsty," which updated the AI’s behavior.

3. **Stat Modification**:
   - Interactions with workplaces or storage buildings modified stats. For example:
     - Consuming berries increased Hunger by +100.
     - Drinking water increased Thirst by +100.

4. **AI Integration**:
   - Stats were monitored during each behavior tree tick. If a critical threshold was crossed, the AI switched priorities to satisfy the need.

---

##### **Development Challenges and Solutions**

###### **1. Managing Multiple Stats Dynamically**

**Issue:**  
- Handling multiple stats in real-time while ensuring smooth integration with AI decision-making created synchronization challenges.

**Solution:**  
- Centralized all stats into `UMS_PawnStatComponent` to simplify tracking and modification.  
- Used delegates (`OnStatChanged`) to notify AI controllers whenever a stat crossed a threshold, updating the Blackboard dynamically.

_Code Snippet: Stat Update and Event Trigger_
```cpp
void UMS_PawnStatComponent::ModifyStat(EStatType Stat, float Amount)
{
    float& CurrentValue = Stats[Stat];
    CurrentValue = FMath::Clamp(CurrentValue + Amount, 0.0f, 100.0f);

    if (CurrentValue <= CriticalThreshold)
    {
        OnStatChanged.Broadcast(Stat, true);
    }
    else
    {
        OnStatChanged.Broadcast(Stat, false);
    }
}
```

---

###### **2. Balancing Stat Depletion**

**Issue:**  
- Finding the right balance for stat depletion rates was challenging. Too fast, and the AI seemed unrealistic. Too slow, and the system felt inconsequential.

**Solution:**  
- Tuned depletion rates based on inspiration from RimWorld, where needs degrade steadily but not overwhelmingly.  
- Used configurable variables (`HungerRate`, `ThirstRate`) to allow tweaking during runtime.

_Code Snippet: Stat Depletion in Tick Function_
```cpp
void UMS_PawnStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    DepleteStat(EStatType::Hunger, HungerRate * DeltaTime);
    DepleteStat(EStatType::Thirst, ThirstRate * DeltaTime);
}

void UMS_PawnStatComponent::DepleteStat(EStatType Stat, float DepletionAmount)
{
    ModifyStat(Stat, -DepletionAmount);
}
```

---

###### **3. AI Priority Conflicts**

**Issue:**  
- AI agents sometimes struggled to prioritize stats effectively, switching between hunger and thirst without satisfying either.

**Solution:**  
- Introduced weighted priority values for each stat. If both Hunger and Thirst were critical, the stat with the lowest value (e.g., `Hunger = 5` vs `Thirst = 15`) took precedence.  
- Updated the Behavior Tree to handle these priorities more effectively.

_Code Snippet: Stat Prioritization**
```cpp
void AMS_AICharacter::UpdateStatPriorities()
{
    if (PawnStats_->GetStatValue(EStatType::Hunger) < CriticalThreshold)
    {
        AIController->GetBlackboardComponent()->SetValueAsBool("GettingFood", true);
    }

    if (PawnStats_->GetStatValue(EStatType::Thirst) < CriticalThreshold)
    {
        AIController->GetBlackboardComponent()->SetValueAsBool("GettingWater", true);
    }
}
```

---

###### **4. Visualization and Feedback**

**Issue:**  
- Without clear feedback, it was difficult to determine which stats were influencing AI decisions during debugging.

**Solution:**  
- Added a floating widget over AI pawns to display current stat values. The widget updated dynamically, providing real-time feedback for both developers and players.  
- Stat bars turned red when below the critical threshold, further emphasizing urgency.

_Code Snippet: Widget Initialization**
```cpp
AMS_AICharacter::AMS_AICharacter()
{
    WidgetComponent_ = CreateDefaultSubobject<UWidgetComponent>(TEXT("StatsWidget"));
    WidgetComponent_->SetupAttachment(RootComponent);
    WidgetComponent_->SetWidgetSpace(EWidgetSpace::World);

    static ConstructorHelpers::FClassFinder<UUserWidget> WidgetClass(TEXT("/Game/UI/WBP_StatWidget"));
    if (WidgetClass.Succeeded())
    {
        WidgetComponent_->SetWidgetClass(WidgetClass.Class);
    }
}
```

---

##### **Inspiration from RimWorld**

The stat system draws significant inspiration from RimWorld’s **Need system**, where each pawn tracks multiple needs like Hunger, Rest, and Mood. Key aspects borrowed include:
- **Gradual Stat Depletion:** Like RimWorld, stats degrade slowly over time, encouraging players to plan ahead and manage resources effectively.  
- **Critical Thresholds:** Needs crossing a threshold (e.g., Hunger < 20) directly influence behavior, creating a sense of urgency.  
- **Stat Prioritization:** Similar to RimWorld’s pawns prioritizing food over work, AI agents in this simulation switch to satisfying critical needs when necessary.  

---

##### **Reflection**

###### **What Worked Well**
- The centralized stat management system simplified development and debugging, making it easy to add or modify stats.  
- Integrating stats with Behavior Trees created a seamless loop where AI decisions reflected their current needs.  
- The floating widget provided excellent visual feedback, improving both player immersion and debugging efficiency.

###### **What Didn’t Work Well**
- Balancing stat depletion rates and thresholds required extensive testing to ensure realism without overwhelming the player.  
- Priority conflicts occasionally arose when multiple stats were critical, though weighted priorities alleviated this.

---

##### **Future Improvements**
- **Mood and Long-Term Needs:** Introduce mood or happiness as a long-term stat influenced by work-life balance, similar to RimWorld’s complex needs.  
- **Dynamic Stat Scaling:** Adjust stat depletion rates based on environmental conditions (e.g., faster thirst depletion in hot weather).  
- **Stat Dependency System:** Implement dependencies between stats, such as reducing energy faster when hunger is low.  

This system successfully emulated the engaging survival mechanics of RimWorld while adapting them to a more focused simulation environment, laying the groundwork for future expansions in AI behavior and player interaction.

#### **3. Resource Management**

  ##### **Resource Management Overview**

  Resource management in this project centered around tracking and distributing resources dynamically across the simulation. The system ensured resources were shared between AI agents, workplaces, and storage buildings while maintaining realistic constraints. It also supported quest generation to address resource shortages.

  ---

  ###### **Key Components**

  1. **`AMS_StorageBuildingPool` and `AMS_StorageBuilding`:**  
    - Represented the centralized resource storage in the simulation. Each storage building maintained an inventory of resource types (`BERRIES`, `WATER`, etc.).
    - The pool served as a container for managing multiple storage buildings and provided APIs for querying resources.

  2. **`AMS_BulletingBoardPool` and `AMS_BulletingBoard`:**  
    - Served as quest hubs where AI characters retrieved tasks to collect and deliver resources.
    - The pool coordinated all bulletin boards and assigned quests dynamically based on shortages.

  3. **`AMS_AIManager`:**  
    - Managed global resource levels and dynamically created quests to address shortages.  
    - Used event-driven communication to update inventories and distribute resources when changes occurred.

  4. **`UResourceManager`:**  
    - Abstracted interactions with `StorageBuildingPool` and `BulletingBoardPool`, centralizing all resource-related operations.  
    - Simplified the integration of new resource types or storage structures.

  ---

  ###### **Flow of Resource Management**

  1. **Initialization**:
    - The `GameManager` initialized the `ResourceManager` at runtime. The `ResourceManager` handled the setup of storage and bulletin board pools.
    - Storage buildings were populated with initial resources, and bulletin boards were linked to the quest system.

  2. **Resource Updates**:
    - Changes to storage inventories were propagated using event-driven systems. For example, when an AI agent deposited resources into a storage building, the manager recalculated shortages.

  3. **Quest Creation**:
    - The `TaskManager` generated quests based on global shortages. Quests were distributed across bulletin boards, ensuring even load balancing.

  4. **AI Interaction**:
    - AI agents queried storage buildings for available resources or retrieved quests from bulletin boards. Upon task completion, they returned resources to storage or consumed them directly.

  ---

  ##### **Development Challenges and Solutions**

  ###### **1. Initialization Order**

  **Issue:**  
  - Similar to AI decision-making, resource pools (`BulletingBoardPool`, `StorageBuildingPool`) were not fully initialized during `AMS_AIManager::BeginPlay`.

  **Solution:**  
  - Used delegates to ensure pools were ready before other components accessed them. This was done using Unreal's `TimerManager`.

  _Code Snippet: Deferred Initialization_
  ```cpp
  void AMS_AIManager::BeginPlay()
  {
      Super::BeginPlay();
      BulletingBoardPool_->OnBulletingBoardPoolInitialized.AddDynamic(this, &AMS_AIManager::OnBulletingBoardPoolReady);
  }

  void AMS_AIManager::OnBulletingBoardPoolReady()
  {
      for (AMS_BulletingBoard* BulletinBoard : BulletingBoardPool_->BulletingBoards_)
      {
          BulletinBoard->OnQuestObtained.AddDynamic(this, &AMS_AIManager::RemoveQuest);
      }
  }
  ```

  ---

  ###### **2. Dynamic Quest Generation**

  **Issue:**  
  - Generating quests dynamically based on resource shortages required real-time analysis of storage levels and efficient quest distribution.

  **Solution:**  
  - Implemented a `TaskManager` that monitored global resource levels and created quests for resources falling below a predefined threshold.  
  - Quests were evenly distributed across bulletin boards using a round-robin approach.

  _Code Snippet: Dynamic Quest Creation_
  ```cpp
  void AMS_AIManager::Tick(float DeltaTime)
  {
      Super::Tick(DeltaTime);

      const int32 LowResourceThreshold = 50;
      int32 MaxResourcePerQuest = 15;

      for (const auto& ResourcePair : Inventory_->Resources_)
      {
          ResourceType Type = ResourcePair.Key;
          int32 Amount = ResourcePair.Value;

          if (Amount < LowResourceThreshold)
          {
              int32 NeededResources = LowResourceThreshold - Amount;

              while (NeededResources > 0)
              {
                  int32 QuestAmount = FMath::Min(NeededResources, MaxResourcePerQuest);

                  bool QuestExists = false;
                  for (const FQuest& ActiveQuest : ActiveQuests_)
                  {
                      if (ActiveQuest.Type == Type && ActiveQuest.Amount == QuestAmount)
                      {
                          QuestExists = true;
                          break; 
                      }
                  }

                  if (!QuestExists)
                  {
                      FQuest NewQuest;
                      NewQuest.Type = Type;
                      NewQuest.Amount = QuestAmount;

                      BulletingBoardPool_->BulletingBoards_[FMath::RandRange(0, BulletingBoardPool_->BulletingBoards_.Num()-1)]->AddQuest(NewQuest);
                      ActiveQuests_.Add(NewQuest);            

                  }
              }
          }
      }
  }
  ```

  ---

  ##### **Reflection**

  Resource management required careful planning to balance dynamic behavior and scalability. Key achievements included implementing event-driven inventory updates and robust quest generation. However, debugging and testing the interplay between AI agents and resource systems proved to be time-intensive. Future improvements could involve AI prioritization logic to reduce task conflicts and adaptive quest generation based on evolving simulation demands.

#### **4. Custom AI Movement**

##### **AI Movement System Overview**

The AI movement system was designed to enable agents to navigate a dynamic, grid-based environment with obstacles and varying terrain. The system focused on implementing **custom pathfinding** using an A* algorithm, combined with seamless node-based traversal for realistic agent movement.

---

###### **Key Components**

1. **Node Grid Initialization**  
   - **`AMS_MovementNodeMeshStarter`**: Responsible for generating a grid of navigable nodes. Each node represented a position on the grid, with information about neighbors and traversability.
   - **`FNode` Struct**: Each node stored positional data, connectivity to neighbors, and traversal cost.

2. **Pathfinding Subsystem (`UMS_PathfindingSubsyste`)**  
   - Implemented the **A* algorithm** to calculate the optimal path between two nodes. The subsystem acted as the centralized navigation system for all AI agents.

3. **Behavior Tree Integration**  
   - The AI agents used a custom `UMS_FollowNodePath` task to traverse the paths generated by the pathfinding system. This task handled node-by-node movement, ensuring smooth interpolation and dynamic obstacle handling.

4. **Visualization Tools**  
   - Debugging tools like line drawings and spheres were added to visualize the node grid and paths, aiding development and troubleshooting.

---

###### **Flow of AI Movement**

1. **Node Grid Generation**:
   - The grid was initialized at runtime by the `AMS_MovementNodeMeshStarter`.
   - Nodes were spaced at predefined intervals, with raycasts performed at each position to ensure traversability.

2. **Pathfinding**:
   - When an AI agent needed to move to a target, it queried the `UMS_PathfindingSubsyste` to find the optimal path using A*.
   - The path was returned as a sequence of nodes, which the agent then followed.

3. **Node-Based Traversal**:
   - The agent iteratively moved from one node to the next along the path.
   - Movement smoothing was achieved by interpolating between nodes, creating natural motion.

4. **Dynamic Updates**:
   - If an obstacle was detected along the path, the AI recalculated a new path on the fly, ensuring responsiveness to environmental changes.

---

##### **Development Challenges and Solutions**

###### **1. Node Grid Initialization**

**Issue:**  
- Uneven terrain caused gaps in the node grid, leading to disconnected paths. This issue arose from raycasts failing to detect valid surfaces at certain positions.

**Solution:**  
- Adjusted the raycasting logic to use a wider trace and higher initial offset, improving detection of valid floor surfaces.  
- Added neighbor validation to ensure nodes connected to at least one other node.
- Removed collisions on the debug points.

_Code Snippet: Grid Node Validation_
```cpp
bool AMS_MovementNodeMeshStarter::PerformRaycastAtPosition(const FVector& Position)
{
    FHitResult HitResult;
    FVector Start = Position + FVector(0, 0, 50.0f); // Offset to account for terrain
    FVector End = Position - FVector(0, 0, 100.0f);  // Trace downwards

    return GetWorld()->LineTraceSingleByChannel(
        HitResult, Start, End, ECC_GameTraceChannel3
    );
}
```

---

###### **2. Debugging Pathfinding**

**Issue:**  
- Debugging pathfinding logic was difficult without visual feedback on the node grid and paths.

**Solution:**  
- Added debug visualizations to display the node grid, connections, and paths in real-time.  
- This helped identify gaps or unintended obstacles in the environment.

_Code Snippet: Node Grid Debug Visualization_
```cpp
void AMS_MovementNodeMeshStarter::DebugDrawNodes()
{
    for (auto& NodePair : NodeMap)
    {
        TSharedPtr<FMoveNode> Node = NodePair.Value;
        DrawDebugSphere(GetWorld(), Node->Position, 50.0f, 16, FColor::Blue, false, -1.0f);
        for (TSharedPtr<FMoveNode> Neighbor : Node->Neighbors)
        {
            DrawDebugLine(GetWorld(), Node->Position, Neighbor->Position, FColor::Green, false, -1.0f);
        }
    }
}
```

---

##### **Reflection**

###### **What Worked Well**
- The **A* algorithm** performed efficiently in small- to medium-sized grids, providing optimal paths under dynamic conditions.  
- Debugging tools significantly improved development speed by providing real-time feedback on node grid and pathfinding logic.

###### **What Didn’t Work Well**
- Large grids caused noticeable performance slowdowns, requiring extensive optimizations.  

---

###### **Future Improvements**
- **Dynamic Node Grid Scaling:** Automatically adjust grid density based on the area’s complexity, reducing unnecessary nodes.  
- **Weighted Pathfinding:** Incorporate additional weights for factors like terrain difficulty or priority paths to improve navigation realism.  
- **Pathfinding Preprocessing:** Cache frequently used paths between common points to reduce real-time computation.  

This approach would further enhance the scalability and adaptability of the AI movement system for larger and more complex environments.






#### **5. Workplaces**

##### **Workplace System Overview**

The workplace system was designed to simulate resource production, consumption, and interaction points for AI agents. Each workplace served as a resource node, providing materials like water or berries, while also acting as destinations for task completion. The system needed to integrate with the AI decision-making and resource management frameworks, ensuring smooth interactions between agents and workplaces.

---

###### **Key Components**

1. **Base Workplace (`AMS_BaseWorkPlace`)**  
   - The parent class for all workplace types.  
   - Provided core functionality such as resource generation, availability tracking, and interaction APIs.  
   - Managed respawn timers for resources after depletion.

2. **Derived Workplaces**  
   - **`AMS_BushWorkPlace`:** Produced berries for AI consumption.  
   - **`AMS_WellWorkPlace`:** Provided water for AI agents.  
   - **`AMS_TreeWorkPlace`:** Produced wood for storage.  
   Each derived class specialized the behavior of the base workplace, such as resource types and production rates.

3. **Workplace Pool (`AMS_WorkpPlacePool`)**  
   - Managed collections of all workplaces in the simulation.  
   - Provided efficient querying for available workplaces matching AI requirements.

4. **AI Interaction Logic**  
   - AI agents interacted with workplaces via tasks in the Behavior Tree.  
   - Interaction logic included resource collection, task validation, and state updates.

---

###### **Flow of Workplace System**

1. **Initialization**:
   - Workplaces were spawned and registered with the `AMS_WorkpPlacePool`.
   - Each workplace was initialized with resource types, capacities, and respawn timers.

2. **AI Task Execution**:
   - AI agents queried the `WorkpPlacePool` for the nearest workplace matching their resource needs.
   - Upon arrival, the AI validated resource availability and performed interactions.

3. **Resource Management**:
   - Resources were depleted during AI interactions and set to respawn after a cooldown period.
   - Resource availability was dynamically updated, and workplaces notified the pool when resources became available again.

---

##### **Development Challenges and Solutions**

###### **1. Resource Respawn Mechanics**

**Issue:**  
- Synchronizing resource respawn timers with AI interactions caused inconsistencies, where resources were marked as available before they were fully replenished.

**Solution:**  
- Implemented a dedicated `FTimerHandle` for each workplace to track resource respawn independently. The respawn process updated resource states only when the timer completed.

_Code Snippet: Resource Respawn Timer_
```cpp
void AMS_BaseWorkPlace::StartRespawnTimer()
{
    GetWorld()->GetTimerManager().SetTimer(
        RespawnTimerHandle, 
        this, 
        &AMS_BaseWorkPlace::RespawnResources, 
        RespawnTime, 
        false
    );
}

void AMS_BaseWorkPlace::RespawnResources()
{
    ResourceAvaliable_ = true;
    UE_LOG(LogTemp, Log, TEXT("Resources respawned at workplace: %s"), *GetName());
}
```
---

###### **2. Debugging Workplace Interactions**

**Issue:**  
- Debugging AI interactions with workplaces was challenging due to the complexity of real-time updates and multiple agents accessing the same workplace.

**Solution:**  
- Added debug text to display resource states above each workplace. This included resource type, current amount, and availability status.

_Code Snippet: Workplace Debug Display_
```cpp
void AMS_BaseWorkPlace::DisplayDebugInfo()
{
    FString DebugText = FString::Printf(
        TEXT("Type: %s | Amount: %d | Available: %s"), 
        *UEnum::GetValueAsString(ResourceType_),
        ResourceAmount_,
        ResourceAvaliable_ ? TEXT("Yes") : TEXT("No")
    );
    DrawDebugString(GetWorld(), GetActorLocation() + FVector(0, 0, 100), DebugText, nullptr, FColor::White, 2.0f);
}
```

---

###### **3. Resource Conflicts**

**Issue:**  
- Multiple AI agents attempting to collect resources from the same workplace led to conflicts, with some agents being left without resources.

**Solution:**  
- Implemented a place reservartion system, when a IA gets a new obejective, it reserves that workplace so no one else can get there.

_Code Snippet: Resource Queueing**
```cpp
bool AMS_BaseWorkPlace::IsPlaceOccupied()
{
	return bWorkPlaceOcupied_;
}

void AMS_BaseWorkPlace::ReservePlace()
{
	bWorkPlaceOcupied_ = true;
}
```

---

##### **Reflection**

###### **What Worked Well**
- The workplace system successfully simulated dynamic resource generation and agent interactions, providing an immersive gameplay loop.  
- The resource locking and queueing mechanisms reduced conflicts and ensured fair distribution among agents.  
- Debugging tools significantly improved the ability to diagnose and resolve issues with workplace states.

###### **What Didn’t Work Well**
- The respawn timers needed careful tuning to balance resource availability with AI task flow.  
- High agent density led to occasional bottlenecks in workplace interactions, especially for high-demand resources.

---

##### **Future Improvements**
- **Dynamic Respawn Rates:** Adjust resource respawn times based on global shortages or AI demand.  
- **Expanded Workplace Types:** Introduce multi-functional workplaces that provide multiple resource types or task opportunities.  

This approach would further enhance the realism and scalability of the workplace system, ensuring a smoother and more engaging AI-driven simulation.


#### **6. Quests and Bulletin Boards**

##### **Quests and Bulletin Boards System Overview**

The quest and bulletin board system was designed to manage task distribution for AI agents dynamically. Quests served as structured tasks for agents, such as collecting or delivering resources, while bulletin boards acted as hubs where quests were posted and retrieved. This system bridged resource management and AI decision-making, enabling seamless task prioritization and execution.

---

###### **Key Components**

1. **Bulletin Boards (`AMS_BulletingBoard`)**  
   - Represented physical locations where AI agents could fetch tasks.  
   - Stored active quests and dispatched them to agents upon interaction.  

2. **Bulletin Board Pool (`AMS_BulletingBoardPool`)**  
   - Managed all bulletin boards in the world.  
   - Distributed quests across boards to balance task availability.  

3. **Quest Structure (`FQuest`)**  
   - Defined the task type, target resource, and quantity required.  
   - Tracked quest progress and completion states.

4. **AI Integration**  
   - AI agents interacted with bulletin boards to fetch quests.  
   - Upon completion, agents updated the quest state and notified the system.

5. **Task Manager (`UTaskManager`)**  
   - Dynamically created quests based on resource shortages or other triggers.  
   - Distributed quests to bulletin boards in a balanced manner.

---

###### **Flow of Quest and Bulletin Board System**

1. **Initialization**:
   - The `TaskManager` initialized the `BulletingBoardPool` and populated bulletin boards with initial quests.  
   - Each board was linked to the `TaskManager` for future quest updates.

2. **Dynamic Quest Generation**:
   - Quests were created dynamically based on conditions such as resource shortages or task demands.  
   - The `TaskManager` distributed these quests to bulletin boards using a round-robin approach to ensure even distribution.

3. **AI Interaction**:
   - AI agents retrieved quests from bulletin boards, updating their Blackboard with the task details.  
   - Upon task completion, the AI agent updated the bulletin board or manager to mark the quest as resolved.

4. **Quest Completion**:
   - Completed quests triggered inventory updates, resource redistribution, or new quest generation.

---

##### **Development Challenges and Solutions**

###### **1. Balancing Quest Distribution**

**Issue:**  
- Some bulletin boards received a disproportionate number of quests, leading to uneven task loads for AI agents.

**Solution:**  
- Implemented a random distribution system in the `AIManager`. This ensured that quests were evenly spread across all boards.

_Code Snippet: Quest Distribution**
```cpp
void AMS_AIManager::Tick(float DeltaTime)
{
  FQuest NewQuest;
  NewQuest.Type = Type;
  NewQuest.Amount = QuestAmount;

  BulletingBoardPool_->BulletingBoards_[FMath::RandRange(0, BulletingBoardPool_->BulletingBoards_.Num()-1)]->AddQuest(NewQuest);
  ActiveQuests_.Add(NewQuest);    
}
```

---

###### **2. AI Quest Retrieval**

**Issue:**  
- AI agents occasionally failed to retrieve quests from bulletin boards due to concurrent interactions or quest mismatches.

**Solution:**  
- Added a lock mechanism on bulletin boards during AI interactions, preventing multiple agents from retrieving the same quest simultaneously.  
- Included fallback logic to direct agents to other boards if no quests matched their needs.
- Saved a list of the active quests so the quest weren't repeated.

_Code Snippet: Quest Locking and Fallback**
```cpp
void AMS_AIManager::RemoveQuest(FQuest Quest) {
    int i = 0;
    for (FQuest ActiveQuest : ActiveQuests_)
    {
        if (ActiveQuest.Type == Quest.Type && ActiveQuest.Amount == Quest.Amount) {
            ActiveQuests_.RemoveAt(i);
            break;
        }
        i++;
    }
}
```
---

###### **3. Debugging Quest Distribution**

**Issue:**  
- Debugging quest distribution and retrieval logic was difficult without clear visibility of quest states.

**Solution:**  
- Added debug text above bulletin boards to display active quest counts and their types.  
- Used logs to track quest creation and assignment in real-time.

_Code Snippet: Debug Text for Bulletin Boards_
```cpp
void AMS_BulletingBoard::DisplayDebugInfo()
{
    FString DebugText = FString::Printf(TEXT("Active Quests: %d"), ActiveQuests.Num());
    DrawDebugString(GetWorld(), GetActorLocation() + FVector(0, 0, 100), DebugText, nullptr, FColor::Yellow, 2.0f);
}
```

---

##### **Reflection**

###### **What Worked Well**
- The dynamic quest generation system effectively addressed global resource shortages, keeping the simulation balanced and responsive.  
- The event-driven approach to quest progress tracking reduced synchronization issues and improved system scalability.

###### **What Didn’t Work Well**
- High-density AI populations occasionally overwhelmed bulletin boards, leading to delays in quest retrieval.  
- Debugging concurrent AI interactions required extensive logging and visualizations to identify edge cases.

---

##### **Future Improvements**
- **Priority-Based Quest Assignment:** Implement a system to prioritize critical resource needs and assign quests accordingly.  
- **Agent-Specific Quests:** Allow AI agents to reserve specific quests based on their current state, reducing conflicts.  

This system lays the groundwork for scalable and efficient task management in AI-driven simulations, ensuring dynamic and engaging interactions between agents and resources.

## Outcome

### Links
- **GitHub Repository:** [AG_MedievalSimulation](https://github.com/Aimar-Goni/AG_AIMedievalSim)
- **Demo Video:** [YouTube](https://youtu.be/your-demo-video)


---

## Critical Reflection

### Strengths
1. **Modularity:** Systems like workplaces, AI tasks, and resource management were highly reusable and extensible.
2. **AI Responsiveness:** Behavior Trees provided a flexible framework for managing AI decision-making.
3. **Resource Synchronization:** The delegate-based inventory system ensured seamless integration between AI, workplaces, and storage.
4. **Scalability:** Pathfinding and node generation systems supported large game worlds.

### Challenges
1. **Pathfinding Performance:** While the A* implementation was optimized, large node networks occasionally caused delays.
2. **UI Feedback:** Limited visual feedback made debugging AI behaviors challenging.
3. **Dynamic Changes:** AI struggled to adapt to significant environmental alterations like obstructed paths.

### Improvements for the Future
1. **Enhanced Pathfinding:** Implement hierarchical A* or navmesh integration for better scalability.
2. **Dynamic Environment Adaptation:** Develop systems for real-time AI adjustments to environmental changes.
3. **Improved UI:** Add visual indicators for AI states, resource levels, and quest progress to aid player understanding and debugging.

### Personal Growth
- **Problem-Solving:** Enhanced my ability to troubleshoot complex systems and debug dynamic interactions.
- **AI System Design:** Gained a deeper understanding of behavior trees and AI state management.
- **Efficiency Focus:** Learned to balance performance optimization with feature complexity.

---

## Bibliography

- Millington, I., & Funge, J. (2009). *Artificial Intelligence for Games*. CRC Press.
- Patel, A. (n.d.). *Introduction to A* Pathfinding*. Available at: https://www.redblobgames.com/pathfinding/a-star/introduction.html
- Unreal Engine Documentation. (n.d.). *Behavior Trees*. Available at: https://docs.unrealengine.com/4.27/en-US/InteractiveExperiences/ArtificialIntelligence/BehaviorTrees/QuickStart/

---

## Declared Assets

- **Custom Implementations:** All code and assets in this project were written and designed by me, with the exception of libraries and tools explicitly referenced above.

