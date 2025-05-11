Okay, this is a significant expansion! Let's break it down into manageable steps for a one-month timeline. This plan prioritizes getting core systems working first and assumes you'll iterate and refine.

**Core Philosophy:**

1.  **Build Incrementally:** Get one system working before moving to the next complex one.
2.  **Leverage Existing Systems:** Integrate new features into your current structure (BT, Components, Pools, AI Manager).
3.  **Keep Initial Scope Small:** Implement the simplest version first, then add complexity if time allows.
4.  **Placeholder First:** Use simple logic or values initially (e.g., fixed work times, fixed prices) and make them data-driven or dynamic later.

---

**Month-Long Plan (Approximate Weekly Goals):**

**Week 1: Foundational Systems & Interaction Polish**

*   **Goal:** Establish time, basic economy, and make interactions feel less instant.

<span style="color: green;">

*   **Step 1: Day/Night Cycle & Time System**
    *   **What:** Need a global clock and day/night states.
    *   **How:**
        *   Create a new `UGameInstanceSubsystem` (e.g., `UMS_TimeSubsystem`) or add to `UMS_GameManager`.
        *   Add variables: `CurrentHour` (float 0-24), `CurrentDay` (int), `TimeScale` (float).
        *   In the subsystem's `Tick`, increment `CurrentHour` based on `DeltaTime * TimeScale`. Handle rollover to the next day.
        *   Add functions `IsDayTime()`, `IsNightTime()`.
        *   Define delegates `OnDayStart`, `OnNightStart` and broadcast them when the time transitions.
        *   **(Optional UI):** Create a simple UI widget to display time/day.



*   **Step 2: Basic Economy - Money & Quest Rewards**
    *   **What:** Pawns need money, quests need to give money.
    *   **How:**
        *   Add `int32 Money;` to `AMS_AICharacter`. Initialize to a starting value.
        *   Modify `FQuest` struct in `MS_ResourceSystem.h` to include `int32 Reward;`.
        *   Modify `AMS_AIManager::Tick` (or wherever quests are generated) to assign a `Reward` value to new quests based on `Amount` and `Type`.
        *   Modify the AI logic where a quest is considered "complete". This is likely after storing items in `AMS_AICharacter::OnOverlapBegin` (for resource quests). Add `AICharacter->Money += Quest_.Reward;` there. Log this for debugging.
        *   **(Refinement):** You'll need a more robust way to signal quest completion later, perhaps a dedicated function or event.


*   **Step 3: Interaction Feedback - Work Duration & Animation Hooks**
    *   **What:** Replace instant resource gathering with a timed action. Add animation triggers.
    *   **How:**
        *   Modify `AMS_AICharacter::OnOverlapBegin` (for Workplaces): **Remove** the *instant* call to `WorkPlace->TakeResources()` and `Inventory_->AddToResources()`.
        *   Create a new `UBehaviorTreeComponent` key (bool) like `bIsAtWorkLocation`. Set this to `true` in the overlap logic when the AI reaches its target workplace.
        *   Create a new BT Task: `UMS_PerformWorkAction` (latent task - `bNotifyTick = true`).
            *   **Input:** Blackboard key for target workplace, float `WorkDuration`.
            *   **ExecuteTask:** Get the `WorkDuration` (start with a fixed value like 3.0 seconds). Store StartTime. Maybe trigger a "Work_Start" animation montage (`PlayAnimMontage`). Return `InProgress`.
            *   **TickTask:** Check if `GetWorld()->TimeSeconds - StartTime >= WorkDuration`.
            *   **If Duration Complete:**
                *   Get the `AMS_BaseWorkPlace*` from the Blackboard.
                *   Call `WorkPlace->TakeResources()`.
                *   Add resources to `AICharacter->Inventory_`.
                *   Call `GameManager->GetWorkPlacePool()->RemoveWorkplaceAndFreeNode(WorkPlace)` (if keeping this single-use logic).
                *   Maybe trigger "Work_End" animation/stop montage.
                *   Clear the `bIsAtWorkLocation` BB key.
                *   `FinishLatentTask` with `Succeeded`.
            *   **If AI Moves/Aborts:** The BT needs logic (e.g., using Abort triggers or decorators) to cancel this task if the AI leaves the location or conditions change. Handle cleanup (stop montage, `FinishLatentTask` with `Aborted`).
        *   Modify BT: After reaching the workplace (MoveTo succeeds), use a sequence: [Set `bIsAtWorkLocation`=true (maybe via overlap?), Run `UMS_PerformWorkAction` Task].


**Week 2: Core Gameplay Loop Change - Job Assignment**

*   **Goal:** Implement the bidding system for task acquisition.

*   **Step 4: AIManager - Quest Broadcasting & Bid Management**
    *   **What:** AI Manager needs to hold available quests and manage bids.
    *   **How:**
        *   In `AMS_AIManager`:
            *   Change `ActiveQuests_` to `AvailableQuests_`. This list holds quests *not yet assigned*.
            *   Create `TMap<FQuest, AMS_AICharacter*> AssignedQuests;` (or similar to track assignments).
            *   Create a delegate: `DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestAvailable, const FQuest&, NewQuest);` and `UPROPERTY(BlueprintAssignable) FOnQuestAvailable OnQuestAvailable;`
            *   Modify `Tick`: When a new quest is generated, add it to `AvailableQuests_` and broadcast `OnQuestAvailable.Broadcast(NewQuest);`. *Do not* assign it to a billboard directly anymore.
            *   Create a struct: `FBidInfo { TWeakObjectPtr<AMS_AICharacter> Bidder; float BidValue; }`
            *   Create `TMap<FQuest, TArray<FBidInfo>> CurrentBids;`
            *   Create a function `UFUNCTION() void ReceiveBid(AMS_AICharacter* Bidder, FQuest Quest, float BidValue);`. This adds the bid to `CurrentBids`. Set a timer here (`StartBidTimer(Quest)`) for maybe 1-2 seconds to collect bids.
            *   Create a function `UFUNCTION() void SelectQuestWinner(FQuest Quest);`. Called by the timer. Finds the best `BidValue` in `CurrentBids[Quest]`, assigns the quest to the winner (move from `AvailableQuests_` to `AssignedQuests_`, notify the winner AI), clears bids for that quest from `CurrentBids`.

*   **Step 5: AI Character - Listening & Bidding**
    *   **What:** AI needs to react to new quests and submit bids.
    *   **How:**
        *   In `AMS_AICharacter::BeginPlay`: Find the `AMS_AIManager` instance and bind a new function `OnNewQuestReceived` to `AIManager->OnQuestAvailable`.
        *   Create `UFUNCTION() void OnNewQuestReceived(const FQuest& NewQuest);`:
            *   Check if the AI is currently idle/available (needs a state in BB or character).
            *   If available, call a new function `EvaluateQuest(NewQuest);`.
        *   Create `float EvaluateQuest(const FQuest& Quest);`:
            *   **Simple Version:** Calculate distance to the nearest resource node of `Quest.Type`. Return `1.0f / Distance` (higher is better).
            *   **Later:** Factor in needs (lower bid if hungry/thirsty), money (maybe?), distance to storage.
        *   If `EvaluateQuest` returns a decent value, call `AIManager->ReceiveBid(this, Quest, BidValue);`.
        *   Add `UPROPERTY() FQuest AssignedQuest;`
        *   Create `UFUNCTION() void AssignQuest(FQuest Quest);`. Called by the AIManager when this AI wins. Set `AssignedQuest = Quest;` and update the Blackboard (`bHasQuest=true`, set quest details).

*   **Step 6: Behavior Tree Update**
    *   **What:** Change the BT logic from "Find Billboard" to waiting for assignment.
    *   **How:**
        *   Remove the sequence involving `MS_FindNearestBulletingBoard` and `MS_GetTask`.
        *   Add a state/branch for "Idle / Waiting for Quest". This could be a service that runs periodically, checking the `bHasQuest` BB key.
        *   When `bHasQuest` becomes true (set by `AssignQuest`), the BT transitions to the "Do Task" sequence (Find Resource -> Go To Resource -> Perform Work -> etc.).



**Week 3: Building Construction & New Placeables (House, Field)**

*   **Goal:** Implement the system for constructing buildings and add initial House/Field functionality.

*   **Step 7: Construction Site Actor**
    *   **What:** An actor representing an unfinished building.
    *   **How:**
        *   Create `AMS_ConstructionSite : public AActor`.
        *   Add properties: `UPROPERTY(EditAnywhere) TSubclassOf<AActor> BuildingClassToSpawn;`, `UPROPERTY(EditAnywhere) ResourceType RequiredResource = ResourceType::WOOD;`, `UPROPERTY(EditAnywhere) int32 AmountRequired;`, `UPROPERTY(VisibleAnywhere) int32 CurrentAmount;`, `UPROPERTY(VisibleAnywhere) FIntPoint GridPosition_;`.
        *   Add a static mesh component (scaffolding?).
        *   Add a function `UFUNCTION() void AddResource(int32 Amount);`. Increments `CurrentAmount`. Checks if `CurrentAmount >= AmountRequired`. If so, calls `CompleteConstruction()`.
        *   Add function `void CompleteConstruction();`: Spawns `BuildingClassToSpawn` at its location, potentially copies relevant data, then `Destroy()` self. Needs error handling.

*   **Step 8: AIManager - Initiating Construction**
    *   **What:** AIManager decides when and where to build.
    *   **How:**
        *   **Resolve AIManager Inventory:** First, decide how `AIManager::Inventory_` works. Is it a central town hall storage? If so, AI needs a way to deposit resources *there* sometimes. If it's just a view, it needs to properly aggregate data. **Let's assume it's a central storage for now.**
        *   In `AMS_AIManager::Tick`: Add logic to check `Inventory_->GetResourceAmount(ResourceType::WOOD)`. If > threshold (e.g., 100), decide to build something.
        *   Function `void StartBuildingProject(TSubclassOf<AActor> BuildingClass, int32 WoodCost);`:
            *   Find a suitable location (e.g., query `PathfindingSubsystem` for a free area or use predefined spots).
            *   Spawn `AMS_ConstructionSite` at the location. Configure its `BuildingClassToSpawn`, `AmountRequired`.
            *   **Crucially:** How does the wood get *from* the AIManager storage *to* the site? This needs a new type of task. Generate a quest: `FQuest BuildQuest(BuildingType, SiteLocation, WoodCost)`. This isn't a resource gathering quest, but a delivery quest. AIs bidding would need to know they need wood *from the AIManager storage*.
            *   **(Alternative):** Instead of AIManager holding wood, construction requires AIs to gather wood *and deliver it directly* to the site. The AIManager just spawns the site and generates "Deliver Wood to [Site]" quests. This avoids the central storage complexity for now. Let's proceed with this alternative.
        *   Modify `AIManager::Tick`: Generate `FQuest(ResourceType::WOOD, AmountNeededForSite)` quests, but somehow tag them or add target info pointing to the specific `AMS_ConstructionSite`. Maybe `FQuest` needs a `TargetActor` field?



*   **Step 9: AI Task - Delivering Resources to Site**
    *   **What:** AI needs to gather wood and take it to the construction site.
    *   **How:**
        *   Modify `FQuest` to optionally hold `AActor* TargetDestination;` (or `FVector TargetLocation`).
        *   Modify `AIManager` to set this target when creating construction delivery quests.
        *   Modify AI BT: The sequence after gathering resources needs to change. If `Quest.TargetDestination` is set (and is a Construction Site), the AI should:
            *   Find Path to `Quest.TargetDestination`.
            *   Move To `Quest.TargetDestination`.
            *   New BT Task/Overlap Logic: `UMS_DeliverToConstructionSite`. Interacts with `AMS_ConstructionSite::AddResource()`, removing wood from AI inventory.




*   **Step 10: House Actor & Basic Sleep Behavior**
    *   **What:** A place for AIs to sleep at night.
    *   **How:**
        *   Create `AMS_House : public AActor`. Give it capacity (`MaxOccupants`), current occupants count. Maybe `TArray<TWeakObjectPtr<AMS_AICharacter>> Occupants;`. Add `GridPosition_`.
        *   Modify `AMS_AICharacter` to have `TWeakObjectPtr<AMS_House> MyHouse;`. AIManager needs logic to assign houses as they are built.
        *   Modify AI BT: Add a high-priority Decorator/Service active during `IsNightTime()`.
        *   If Night: Abort current low-priority task (like gathering wood). Run sequence:
            *   Check if `MyHouse` is valid. If not, maybe wander or find temporary shelter (or skip sleep).
            *   `FindPathTo(MyHouse->GetActorLocation())`.
            *   `MoveTo` House.
            *   New BT Task: `UMS_SleepTask` (Latent). Runs while `IsNightTime()`. Inside `TickTask`, slowly increase `PawnStats_->ModifyEnergy()`. When `IsDayTime()`, task succeeds. Needs interaction with House capacity.

*   **Step 11: Wheat Field Actor & Basic Interaction**
    *   **What:** A placeable that generates Wheat. Simplified cycle.
    *   **How:**
        *   Create `AMS_WheatField : public AActor`. Add `GridPosition_`.
        *   Add enum `EFieldState { Idle, Growing, ReadyToHarvest }`. Add `State`. Add `GrowthTimer`.
        *   `BeginPlay` or when built: Set `State = Growing`, start `GrowthTimer` (e.g., 60 seconds).
        *   When timer finishes: Set `State = ReadyToHarvest`.
        *   `AIManager`: When a field is `ReadyToHarvest`, generate `FQuest(ResourceType::WHEAT, HarvestAmount)` targeting this field.
        *   AI BT: Handle Wheat quest. Go to field. `PerformWorkAction` task (maybe shorter duration). On success, AI gains `ResourceType::WHEAT`. The field needs to reset (`State = Idle` or `Growing`).
        *   Add `ResourceType::WHEAT` to `UInventoryComponent`. Decide how Wheat is used (Needs conversion to Food? A bakery building later?). For now, maybe it directly satisfies hunger slightly less effectively than berries?

</span>
DOING: new construction block floor

TODO: fix house spawning so its closer
TODO: Fix the fps drops with multiple pawns
Fix the not having resource source

**Week 4: Pub, Refinements & Polish**

*   **Goal:** Add the Pub for mood, refine bidding, handle edge cases.

*   **Step 12: Pub Actor & Mood Interaction**
    *   **What:** Building for happiness boost, costs money.
    *   **How:**
        *   Create `AMS_Pub : public AActor`. Add `GridPosition_`.
        *   Modify AI BT: Add a low-priority sequence triggered when `PawnStats_->IsSad()` is true AND `AICharacter->Money >= DrinkCost`.
        *   Sequence: Find nearest Pub -> Go To Pub -> New BT Task `UMS_BuyDrinkTask` (Latent).
        *   `UMS_BuyDrinkTask`: Takes `DrinkCost`, `HappinessGain`. `ExecuteTask`: Check money. If enough, deduct money, start timer/animation. Return `InProgress`. `TickTask`: Wait duration. `FinishLatentTask`: Call `PawnStats_->ModifyHappiness(HappinessGain)`.

*   **Step 13: Refine Job Bidding Evaluation**
    *   **What:** Make AI bidding smarter.
    *   **How:**
        *   Update `AMS_AICharacter::EvaluateQuest`:
            *   Calculate path distance to resource (`Dist_Resource`).
            *   Calculate path distance from resource to storage/destination (`Dist_Delivery`). Total path length = `Dist_Resource + Dist_Delivery`.
            *   Get current needs (Hunger, Thirst). Apply penalty if needs are high (e.g., `BidValue *= (1.0 - (HungerPenalty * CurrentHunger/100.0))` ).
            *   Base bid could be related to `Quest.Reward`.
            *   Final Bid = `(BaseReward / TotalPathLength) * NeedsModifier`.

*   **Step 14: System Integration & Testing**
    *   **What:** Test interactions between all systems.
    *   **How:** Playtest extensively. Do AIs correctly prioritize needs vs. work vs. sleep vs. pub? Does construction complete? Does bidding work? Fix bugs. Add logging where needed.

*   **Step 15: (If Time) Add Animation Polish**
    *   **What:** Connect actual animations.
    *   **How:**
        *   Create AnimMontages for actions (Chopping, Harvesting, Building, Drinking, Sleeping).
        *   In the relevant BT Tasks (`PerformWorkAction`, `SleepTask`, `BuyDrinkTask`, etc.), use `PlayAnimMontage` and `StopAnimMontage`.

---

**Things You Might Forget (Cross-Cutting Concerns):**

1.  **Saving/Loading:** How will game state (AI positions, inventories, building states, time) be saved and loaded? This is complex; maybe defer beyond the month.
2.  **UI Feedback:** Display AI status (working, sleeping), money, maybe current quest. Display building progress. Indicate low resources for the AIManager.
3.  **Balancing:** How much reward per quest? How much do drinks cost? How fast do needs decay? How long does work take? How much wood per building? Initial values will need tuning.
4.  **Error Handling:** What if paEthfinding fails? What if AIManager can't find a free spot to build? What if an AI tries to interact with a destroyed object? Add checks for null pointers (`IsValid()`).
5.  **Performance:** Spawning many actors or complex BTs can slow things down. Keep pooling active. Profile if needed. Bidding system might need optimization if many AIs bid simultaneously.
6.  **AIManager Inventory:** Seriously define how this works (central storage vs. aggregated view). This impacts construction and quest generation.
7.  **Resource Consumption:** How is Wheat used? Does it need processing (Mill, Bakery)? How do AIs decide *what* to eat if they have Berries and Wheat products?
8.  **Building Placement:** Ensure construction sites/buildings don't block critical paths or spawn overlapping. Pathfinding subsystem needs to be updated correctly when buildings are placed/removed.
9.  **AI State Management:** Ensure clear transitions between states (Idle, GoingToWork, Working, GoingHome, Sleeping, GoingToPub, Bidding etc.) in the Behavior Tree and character state variables. Use Blackboard keys effectively.
10. **Tool Requirements:** Do AIs need tools (axe for wood)? Add logic for acquiring/carrying tools if necessary.

Epifanio16
