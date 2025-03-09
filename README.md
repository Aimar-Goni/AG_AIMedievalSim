# AG_AIMedievalSim

## Project Outline
AG_AIMedievalSim is a medieval simulation project developed in Unreal Engine, focusing on AI-driven resource management and task execution. The primary goal of this project is to create intelligent AI agents capable of making autonomous decisions, gathering resources, and interacting with various workplaces within a historically inspired medieval setting. By designing a structured yet flexible system, the project aims to balance realism and gameplay practicality, ensuring that AI characters can simulate medieval life effectively.

The initial objectives of AG_AIMedievalSim were to develop AI characters that could dynamically collect resources such as wood, berries, and water while also managing their own needs, such as hunger and thirst. To achieve this, the project integrated a robust resource management system, modular workplaces, and an interactive bulletin board for quest assignment. AI behavior trees and blackboards played a crucial role in driving decision-making processes, allowing the agents to evaluate their current state and prioritize actions accordingly. Another core objective was to optimize pathfinding using an adapted A* algorithm, ensuring efficient movement across complex terrains without unnecessary computation overhead.

During development, several im anticipating several challenges. One major issue will be ensuring synchronization between AI decision-making and resource availability, as conflicts can arise when multiple AI agents sought the same resources. Optimizing pathfinding for a dynamic environment will pose another significant challenge, requiring careful adjustments to heuristics and node processing. Additionally, debugging AI behaviors within the behavior tree framework will be expected to be time-consuming due to the complexity of state transitions. Finally, striking a balance between AI prioritization of survival needs (such as hunger and thirst) and fulfilling assigned tasks will be another potential obstacle that needs a strategic solution.

## Research

### Methodology  
To create AG_AIMedievalSim, extensive research was conducted across multiple disciplines, including game AI development, medieval simulation design, and technical implementation strategies. The research process involved studying AI decision-making models such as behavior trees and finite state machines, analyzing historical labor systems for a realistic portrayal of medieval work-life balance, and reviewing best practices in Unreal Engine AI development. By leveraging these insights, the project was designed to integrate both historical accuracy and practical game mechanics.

### Game Sources  
Several games served as primary references for AG_AIMedievalSim. Kingdom Come: Deliverance provided valuable insights into medieval life simulation, including resource scarcity, NPC routines, and the importance of immersive AI behavior. RimWorld heavily influenced the pawn stats system, which allowed AI agents to manage their own needs and dynamically prioritize tasks. The Settlers series helped shape the workplace interaction and resource transportation mechanics, ensuring that AI agents could efficiently distribute resources across different workplaces.

### Academic Sources  
Academic literature also played a fundamental role in the project’s development. "Artificial Intelligence for Games" by Ian Millington and John Funge provided a deep understanding of pathfinding, finite state machines, and utility-based decision-making, which were crucial for implementing adaptive AI behaviors. Amit Patel’s Guide on A Pathfinding* was instrumental in refining the pathfinding system, optimizing it for real-time navigation within a medieval village setting. GDC talks on AI decision-making, particularly those focusing on hierarchical AI structures, provided strategies to enhance AI adaptability and efficiency in real-time simulations.

### Documentation Sources  
Technical documentation was another crucial aspect of the research process. Unreal Engine’s AI documentation guided the implementation of behavior trees, blackboards, and AI controllers, ensuring that AI characters could efficiently evaluate and execute tasks. The Unreal Engine Pathfinding Documentation offered essential insights into grid-based navigation and custom A* algorithm integration, helping refine AI movement across complex terrains. Additionally, developer forums and community tutorials provided solutions to specific issues, such as optimizing AI state transitions and improving event-driven system interactions.

## Implementation

### Process
- Provide a step-by-step breakdown of your development process, including key milestones and decisions made throughout the project.  
- Highlight any tools, frameworks, or techniques used, and explain how they contributed to the implementation.  
- Include screenshots, diagrams, or code snippets where relevant to showcase your progress.

### New Approaches  
One of the most innovative aspects of AG_AIMedievalSim was the dynamic task prioritization system, which allowed AI agents to adjust their priorities based on real-time needs. Instead of following a rigid task structure, AI could dynamically shift focus between gathering resources, fulfilling workplace duties, and meeting survival needs. Additionally, an event-driven resource update system was implemented, ensuring that AI agents responded to resource shortages and adjusted their actions accordingly. Pathfinding was another area of innovation, where heuristic adjustments improved efficiency, reducing unnecessary calculations while maintaining precise movement logic.

### Testing
Testing played a crucial role in refining AG_AIMedievalSim. Automated tests were conducted to debug AI decision loops and validate blackboard key updates, ensuring smooth transitions between states. Guided user testing was employed to observe AI interactions with workplaces and fine-tune decision-making behaviors. Performance profiling was used to identify computational bottlenecks, leading to optimizations in pathfinding calculations and AI behavior evaluations.

### Technical Difficulties
Several technical challenges were encountered during development. One issue was ensuring that AI components initialized in the correct order, as resource pools and bulletin boards often contained references that were not fully instantiated. This was resolved using delegate-based event binding. AI agents sometimes experienced task-switching conflicts, repeatedly changing objectives without completing them, which was addressed by introducing weighted priorities in the decision-making process. Another major challenge was pathfinding performance, as large node networks caused slowdowns. The solution involved optimizing heuristic calculations and pruning unnecessary nodes to maintain efficiency.

## Outcomes

### Source Code/Project Files
The complete source code for AG_AIMedievalSim is available on GitHub: AG_AIMedievalSim. The repository includes implementations of AI behavior trees, a resource management system, a task delegation mechanic, and an optimized pathfinding algorithm.

### Build Link
- Share a link to a playable or executable build of your project.  
- Ensure the build is accessible across relevant platforms and is publicly accessible.  
- Include any necessary instructions for running the build, such as system requirements or installation steps.

### Video Demonstration
- Embed a video or provide a link to a recorded demonstration of your project in action.  
- The video should showcase key features, functionality, and any unique elements of your project.  
- Include a brief commentary or text overlay in the video to explain the different aspects of your project as they are shown.

## Reflection

### Research Effectiveness  
- Assess the usefulness of the research conducted during the project.  
- Highlight which sources (games, academic, documentation) had the most significant impact on your work and explain why.  
- Identify any research gaps or areas where additional information could have improved your project outcomes.

### Positive Analysis 
- Reflect on the successful aspects of the project.  
- Highlight specific elements that worked well, such as technical solutions, creative decisions, or user feedback.  
- Provide evidence to support your analysis, such as test results, screenshots, or user comments.

### Negative Analysis  
- Identify the areas of the project that did not go as planned or could have been improved.  
- Discuss challenges you faced, whether technical, creative, or time-related, and evaluate their impact on the final product.  
- Reflect on any mistakes or missteps and what you learned from them.

### Next Time
- Outline what you would do differently if you were to undertake a similar project again.  
- Suggest improvements to your workflow, research methods, or implementation process based on your reflections.  
- Consider any new tools, techniques, or approaches you would explore in future projects to achieve better results.

## Bibliography  
Millington, I., & Funge, J. (2009). Artificial Intelligence for Games. CRC Press.
Patel, A. (n.d.). Introduction to A Pathfinding*. Available at: https://www.redblobgames.com/pathfinding/a-star/introduction.html
Unreal Engine Documentation. (n.d.). Behavior Trees. Available at: https://docs.unrealengine.com/4.27/en-US/InteractiveExperiences/ArtificialIntelligence/BehaviorTrees/QuickStart/

## Declared Assets
All AI logic, resource systems, and pathfinding algorithms were developed from scratch by Aimar Goñi. External references were adapted from academic literature and GDC talks, ensuring originality in implementation. Some of the code has been tuned and optimized using chat AIs help and most of the assets ussed on the project are purchased from syntystore at: https://syntystore.com/en-gb

