# Tsuki and the Moon — Game Design Document

## 1. Project vision

**Genre:** Single-player, third-person 3D puzzle adventure  
**Engine:** Unreal Engine 5  
**Scope:** A polished, completable MegaJam vertical slice featuring three connected areas and an ending beat at the giant tree.

Tsuki wakes in a strange magical world and discovers a pair of mysterious scissors. With them, she can cut an abstract property from one object and attach it to another, changing how that object behaves. The adventure teaches this rule through a connected forest journey, gradually introducing traversal tools and more involved puzzle chains. The fairy who woke Tsuki accompanies her as a guide.

The game's connection to the **“Director’s Cut”** theme is its central mechanic: Tsuki edits properties in the world, changing the behavior of objects and the way a scene can unfold.

### Design goals

1. Make every property transfer understandable from the objects, their visual identifiers, and their physical response.
2. Encourage experimentation by making mistakes recoverable and failures low-cost.
3. Make the three areas feel like distinct places within one cohesive magical forest.
4. Give the vertical slice a complete progression arc, ending with a glimpse of the next area rather than an unfinished-feeling stop.

## 2. Player experience and controls

- **Camera and movement:** Player-controlled third-person camera with light framing assistance around important puzzle objects and traversal targets. Tsuki can walk, run, jump, interact, and swim. Movement is responsive and grounded, with forgiving platforming. She wades through shallow water and swims at the surface of deep water (no diving); from the water she can only climb out onto edges close to the waterline, so higher ledges must be reached another way. Small objects such as the pebble can be lifted and carried; large blocks are pushed and pulled.
- **Scissors targeting:** The player aims at an object. Valid targets receive a clear contextual highlight. Cuttable sources and compatible receivers use different visual identifiers.
- **Minimal HUD:** Show the currently carried property, relevant contextual prompts, and Moon Fragment count. Keep the world unobstructed.
- **Checkpoints:** Place checkpoints near significant puzzle or traversal sequences. Falling returns Tsuki nearby; preserve solved puzzle states where practical.
- **Puzzle recovery:** A carried property can be attached, returned, or removed through the scissors interaction loop. A valid transfer can be reversed by cutting the property off its target. The one-property carrying limit still applies.

## 3. Scissors and property rules

### Transfer loop

1. Aim at a valid source and cut one of its properties.
2. The source visibly loses that property; Tsuki carries it as the single active property.
3. Aim at a compatible receiver and attach the carried property to its socket.
4. The receiver visibly changes and exhibits the property’s consistent gameplay effect.
5. A property can be cut from its target again and carried, returned, or moved to another compatible receiver.

If Tsuki is carrying a property, she cannot cut another until she attaches or removes the carried one. Objects communicate their roles: sources have a property-colored glow or identifier, and receivers have a distinct neutral (initially white) glow. When targeted, the scissors make the currently valid interaction clear. If an object can be both a source and receiver, show both roles without obscuring which action is currently available.

### Property language

Each property has a stable color and visual effect shared by its source, carried state, and attached state. Use a legible effect in addition to color so the game remains readable at a glance.

| Property | Core effect | First planned use |
|---|---|---|
| Lightness | Makes a suitable object lighter and easier to move or causes a counterweight to rise. | Reduce the weight of the blocking stone; later manipulate the canopy lift. |
| Heaviness | Adds weight to a suitable object, helping it depress a pressure plate or sink. | Make a pebble heavy enough to trigger a plate; later sink a lily pad. |
| Buoyancy | Makes an object rise in water and float at the surface. | Raise a submerged platform and float a crate across water. |
| Slowness | Reduces the speed of a moving object. | Slow a moving platform and, later, a rotating wheel. |

Property effects should be consistent, learnable, and indicated by visible physical behavior. A pressure plate responds physically when sufficient weight is applied. When a plate activates an important mechanism, play a short reveal and musical cue in the spirit of classic adventure-game feedback.

## 4. Guide, story, and presentation

The fairy wakes Tsuki at the start, accompanies her, and acts as a friendly guide with a modest personality. It helps players notice puzzle logic without solving every puzzle immediately. Hints are contextual and layered: first a thematic nudge toward an object or property, then a more direct clue if the player requests more help or remains stuck.

Example: near a heavy blocking stone, the fairy might say, “Maybe something light could help us move this.” Hints should refer to things the player can see or has already learned.

The opening is brief and atmospheric: Tsuki wakes in the clearing and the fairy greets her. She has no tools yet. The fairy flies ahead along a short, safe path to where the scissors rest; picking them up plays the tool pickup cue and one fairy line, and the first puzzle follows right after. The exact reason Tsuki is in this world remains a story mystery through this slice. The giant tree provides a meaningful lead toward what lies beyond.

Use original melodic fantasy music, area-specific motifs, and short puzzle-success cues. Pressure-plate reveals temporarily take control for a brief, focused camera shot, then return control promptly. Tool pickups receive a distinct visual/audio cue and one concise fairy explanation.

## 5. World route at a glance

The playable route is mostly linear:

**Forest Entrance → Sunken Crossing → Whispering Canopy → Giant tree summit → glimpse of the next area**

The three areas share a stylized, colorful fantasy-forest art direction but have distinct lighting, palettes, vegetation, silhouettes, and environmental moods. Shortcuts and selected backtracking opportunities reward players who remember earlier locations. The route is designed for a third-person camera: key landmarks should be visible from approach viewpoints, important objects should not be hidden behind required camera turns, and the player should be able to understand the intended route from composition and visual contrast.

---

# 6. Area design briefs

The following layouts define spatial relationships and intended visual reads. Exact dimensions and asset choices may change in production, but each area should preserve the described route, landmarks, sightlines, and puzzle logic.

## Area 1 — Forest Entrance

### Purpose and mood

The opening area teaches the player how the scissors work and introduces Lightness, Heaviness, and pressure plates. It should feel like a welcoming forest threshold: quiet, lush, and a little magical, with the path forward readable and the first puzzle obstruction immediately apparent.

### Spatial composition

Build the area as a compact, gently curving forest trail that leads from an open wake-up clearing to a closed gate. The player starts in the clearing, where Tsuki wakes and meets the fairy. A short, safe path with no puzzles or hazards, with the fairy flying slightly ahead, leads from the clearing to the scissors' resting place, then continues to the blocking stone. Frame a glimpse of the onward path beyond the gate so that the destination is visible before the player knows how to reach it.

The trail narrows at a heavy stone block that physically blocks the main route. Place a feather plant close by and within the same readable camera composition as the stone. This should feel like the first self-contained puzzle bay: enough room to circle both objects, aim the scissors, and push or pull the stone after changing it, but not so broad that the connection between plant and block is lost.

Past the block, lead the player into a second small clearing dominated by a gate and a visible pressure plate. Place a small pebble on or near the trail, with a large boulder nearby as the Heaviness source. The player should be able to see the gate, plate, pebble, and boulder together or understand their relationship from one short look around. The plate’s mechanism should be legible: when weighted, it depresses and the gate visibly opens.

After the gate, the path transitions naturally toward the watery terrain of Sunken Crossing. From the entrance side, show enough of this direction to establish a destination without revealing the full next area.

### Key landmarks and set dressing

- **Wake-up clearing:** A calm, sheltered pocket of grass and roots with a readable space for Tsuki and the fairy’s opening moment. Use a strong canopy opening or shaft of light to focus the initial view.
- **Scissors resting place:** A small, lit nook at the end of the fairy's path where the scissors rest in clear view. It should read as special at a glance and sit just before the first puzzle bay.
- **Feather plant:** A clearly recognizable magical plant with light, airy fronds and the Lightness identifier. It should look alive and distinct from ordinary foliage.
- **Blocking stone:** A large, weighty, visually grounded stone whose position obviously prevents passage. Its mass is communicated by size, contact with the earth, and a subdued Heaviness visual.
- **Large boulder:** A visibly heavier, rounded boulder with the Heaviness identifier. Keep it close enough to the gate puzzle that its intended use is discoverable.
- **Gate and pressure plate:** Make the closed gate a strong silhouette across the path. The plate sits in front of it with clear space for the pebble and a sightline to the gate’s movement.
- **Optional raised platform:** Place a wooden platform high enough to be unreachable on the first visit but visible from the trail or gate clearing. Put a closed treasure chest on it. Frame the platform so players can remember it when they later return with the Moonthread Hook.

### Puzzle beats

0. **Find the scissors:** Follow the fairy along the short path and pick up the scissors. The pickup plays the tool cue and one fairy explanation. Scissors actions are unavailable before this.
1. **Move the blocking stone:** The player sees the stone blocking the route and the nearby feather plant. Cut Lightness from the plant and attach it to the stone. The plant loses its Lightness effect; the stone visibly becomes lighter, and Tsuki can push or pull it clear.
2. **Open the gate:** The small pebble is too light to hold down the pressure plate. Cut Heaviness from the large boulder and attach it to the pebble. Place or move the heavier pebble onto the plate. The plate depresses, a brief camera reveal and musical cue show the gate opening, and the player proceeds.
3. **Optional return visit:** After obtaining the Moonthread Hook in Area 2, the player can return to hook onto the high wooden platform and open the chest containing a Moon Fragment.

### Lighting, palette, and sound

Use warm leaf-filtered light, fresh greens, soft earth browns, and a few luminous accents for magical interactables. The starting clearing should be calm and inviting; the gate can introduce a slightly cooler shadow or stronger contrast to signal the first obstacle. Keep property colors distinct from the natural palette. Forest ambience should feel gentle, with subtle musical support rather than urgency.

### Readability checks

- From the start, the player should see or quickly discover the main route and the first obstruction.
- The fairy's path to the scissors is short and obvious.
- The feather plant and blocking stone should read as a plausible source/receiver pair.
- The boulder, pebble, plate, and gate must be visually connected as the second puzzle.
- The optional high chest should be noticeable but clearly unreachable until a traversal tool is acquired.

## Area 2 — Sunken Crossing

### Purpose and mood

This area introduces Buoyancy and a simple multi-step chain: raise a submerged platform, float a crate across water, make that crate heavy enough to activate a plate, reveal the Moonthread Hook, then use the Hook to leave the area. It should feel like a water-crossing hidden within the forest: tranquil on the surface, with submerged shapes and reflections suggesting a deeper space.

### Spatial composition

The player enters on a dry bank overlooking a broad but bounded pond or slow river. Put the exit platform on the far side, visible across the water but not reachable by ordinary jumping. The water should define a clear left-to-right or near-to-far crossing rather than an unbounded lake, so the player can understand the challenge.

Near the entrance shore, place a visible water bubble as the Buoyancy source. In the water, position a submerged wooden platform close enough to the shore that the player can identify it beneath the surface. Its intended route should carry the player toward the other side and form a usable bridge once it rises. The sunken platform should have a recognizable wooden silhouette and some visible submerged supports or details.

Place a crate on a dry ledge or shore where it can be pushed into the water. Its route should be readable: across the pond, toward a raised far ledge, then to a pressure plate. The crate should float after receiving Buoyancy, but it should not automatically reach the plate; the player must guide it across and then transfer Heaviness from the nearby boulder. Set the plate so the player can understand that it is connected to lowering the pond. The lowering water reveals a chest containing the Moonthread Hook.

Once the pond is lowered and the Hook is obtained, visually frame the exit platform as a clear target. The platform should have a distinct hook target that teaches the player how the new tool is used to reach the next area.

### Key landmarks and set dressing

- **Entrance bank:** A stable, dry staging zone with a clear view over the puzzle water and onward exit.
- **Buoyancy bubble:** A magical, floating bubble source with a distinctive property-colored shimmer. Place it near the submerged platform and within easy targeting range from safe ground.
- **Submerged wooden platform:** Visible through clear water. Its edges, support structure, or markings should help the player anticipate that it can rise and become a bridge.
- **Crate:** A sturdy, readable object with enough clearance to push, pull, and float. Its size should make the weight change legible without preventing the player from moving around it.
- **Heaviness source boulder:** A nearby boulder that provides the property for the crate’s final pressure-plate step. Position it so the crate, boulder, plate, and water mechanism can be understood as one chain.
- **Pressure plate and pond mechanism:** The plate should be physically connected to a visible water-control feature, such as a sluice, stone drain, or magically shifting channel. This connection can be suggested visually; avoid introducing a separate puzzle rule.
- **Moonthread Hook chest:** Revealed when the pond lowers. Give it a strong silhouette and a distinct pickup presentation.
- **Exit hook target:** Place on the far-side exit platform, visible from the puzzle area and highlighted once the Hook is available.

### Puzzle beats

1. **Raise the bridge:** Cut Buoyancy from the water bubble floating on the pond and attach it to the submerged wooden platform. The platform rises to the surface and settles into a stable bridge across the water. Tsuki can swim in the pond, but the far side is too high to climb from the water; from the risen bridge she can jump onto it.
2. **Float the crate:** Cut Buoyancy from another water bubble, then move the crate into the water and attach Buoyancy. Buoyancy causes it to rise and float at the surface to a high ledge across; the player remains able to push or pull it horizontally.
3. **Lower the pond and reveal the Hook:** The crate is too light to activate the pressure plate. Cut Heaviness from the nearby boulder and attach it to the crate. Move the crate onto the plate. The plate depresses; a short camera reveal and musical cue accompany the pond lowering and the chest becoming accessible. Only the separate basin holding the chest drains; the main pond stays full.
4. **Leave with the Hook:** Open the chest and acquire the Moonthread Hook. Aim at the marked exit target and hook across to the next area.
5. **Return opportunity:** The Hook enables a return trip to the high platform in Forest Entrance for its optional Moon Fragment chest.

### Lighting, palette, and sound

Use cool greens and blue-greens in the water, with warm wood and stone accents on the banks. Let shafts of light, reflections, ripples, and glimpses of the submerged platform create depth while preserving visibility. The lowered-pond reveal should be visually clear: show the waterline descending and the new chest becoming accessible. Audio should emphasize water, soft bubble sounds, wood creaks, and a satisfying mechanism cue when the plate activates.

### Readability checks

- The player can see the submerged platform before solving it.
- Buoyancy’s behavior is obvious: the platform rises, and the crate floats.
- The crate’s route across the water and toward the pressure plate is understandable.
- The plate’s relationship to the pond lowering is visually telegraphed.
- The Hook’s pickup and marked exit target make the next action clear.

## Area 3 — Whispering Canopy

### Purpose and mood

This is the largest and most vertically layered area of the slice. It introduces Slowness, combines previously learned properties, adds Hook traversal, provides the required Moonlit Bombs route, and culminates at the giant hollow tree. It should feel elevated, airy, and mysterious, with the forest floor far below and a clear visual destination at the tree.

### Spatial composition

Compose the area as connected canopy platforms and natural perches around a deep forest hollow. The player enters at a lower canopy level and should be able to see the giant hollow tree in the distance or above the route, making it a persistent destination landmark.

The first traversal puzzle sits along the initial route: a magical snail provides Slowness, and a fast-moving platform travels between reachable positions. Place a safe waiting perch where Tsuki can observe its movement. The platform’s full path should be visible from that perch. Once slowed, the platform gives the player a safe timing window to Hook onto the next marked platform.

Beyond this section, place a pulley lift with a counterweight. A Hook-accessible perch beside the counterweight holds the feather plant used to transfer Lightness. Set the counterweight and lift so their opposite vertical movement is easy to read. The player reaches the lift, makes the counterweight lighter so it rises and the lift descends, then cuts Lightness off the counterweight while aboard. It becomes heavy again and drops, raising the lift and player to an upper level.

At the upper level, place a pressure plate that opens a nearby gate. The gate creates a shortcut back toward the first puzzle area, letting the player revisit it without repeating the lift. From the gate and upper route, make the optional/required secret path discoverable as a branching route back toward a heavy boulder beside a tree, then onward to a pond.

In the pond section, place a submerged pressure plate beneath a lily pad. The boulder supplies Heaviness; transferring it to the lily pad makes the pad sink and press the submerged plate. The activated plate reveals a concealed path. This is a secret-feeling route, but it is **required for main progression** because the Moonlit Bombs at its end are needed at the giant tree.

The revealed path leads to a rapidly rotating wheel over a drop or difficult-to-cross gap. Put a magical snail nearby as the Slowness source. The wheel’s rotation is visually prominent and clearly too fast to stand on before it is slowed. After the player attaches Slowness, it turns slowly enough to ride upward and jump to a chest platform containing the Moonlit Bombs.

Return to the main canopy route and lead toward the giant tree’s base. Put a pressure plate at the base that activates a sequence of temporary Hook targets across the branches. Make the climbing route readable as a sequence with generous timing, visible target transitions, and a nearby recovery point. At the top, place the cracked opening in the hollow tree. Using a Moonlit Bomb opens the passage; Tsuki jumps down and the camera reveals a glimpse of the next area.

Behind or around the giant tree, place a separate cracked hole for the optional Moon Fragment chest. It should be discoverable from the tree’s exterior route but not confused with the main cracked opening at the summit.

### Key landmarks and set dressing

- **Canopy entry perch:** Safe arrival platform with a view toward the giant tree and the first moving-platform puzzle.
- **Magical snail:** Clearly readable source of Slowness. Distinguish it from ordinary scenery, and place it near both the moving-platform puzzle and the later rotating-wheel puzzle as appropriate to the planned return route.
- **Fast moving platform:** Its endpoints and movement arc should be visible from a waiting perch. Use a clear visual rhythm so the player can judge timing before transferring Slowness.
- **Hook targets:** Consistent, high-contrast target markings integrated into branches, platforms, or magical anchors. They should be visible against foliage and only guide the intended route.
- **Pulley and counterweight:** Large enough to read across the vertical space. Keep the lift and counterweight in the same composition when possible so the inverse motion is obvious.
- **Feather plant:** Place on a Hook-accessible perch beside the counterweight. Its location should make the intended Lightness transfer apparent.
- **Shortcut gate and pressure plate:** On the upper level, visible shortly after the lift puzzle. Show enough of the route beyond the gate to communicate that it reconnects to the earlier area.
- **Heavy boulder:** Place near a recognizable tree on the return path. It is the Heaviness source for the lily-pad puzzle.
- **Pond, lily pad, and submerged plate:** Make the plate visible under clear water or legible through markings beneath the pad. The pad should be broad enough for the weight change and sinking behavior to read clearly.
- **Secret-path reveal:** Use a physical change to expose the passage when the submerged plate activates—such as roots parting, a stone wall sliding, or a concealed opening becoming lit. Do not obscure where the route begins.
- **Rotating wheel:** A large, fast-spinning object positioned as an obvious traversal obstacle. Place the snail source nearby, and show the chest platform above or beyond the wheel.
- **Moonlit Bomb chest:** On a stable platform reached by riding the slowed wheel and jumping. Make the reward and its cracked-surface use understandable at pickup.
- **Giant hollow tree:** A towering, visually distinctive landmark visible from multiple points in the area. Its trunk, branches, and hollow crown should guide navigation and frame the finale.
- **Summit cracked opening:** A clear target for the Moonlit Bombs on top of the tree.
- **Optional rear cracked hole:** A separate bombable surface behind the tree, leading to the optional Moon Fragment chest.

### Puzzle beats

1. **The restless platform:** Cut Slowness from the magical snail and attach it to the fast moving platform. Once its travel is slow enough to board or time safely, use the Moonthread Hook to reach the next platform.
2. **The hanging counterweight:** Use the Hook to reach the perch beside the counterweight. Cut Lightness from the feather plant and attach it to the counterweight. It rises and the lift descends. Stand on the lift, cut Lightness off the counterweight (counterweight has to be close enough so the player can reach it), and let it become heavy and drop; this raises Tsuki to the upper level.
3. **Open the shortcut:** Step on the nearby pressure plate to open a gate back toward the first puzzle area, creating a route that avoids repeating the pulley.
4. **The sunken secret route:** Return through the shortcut to the heavy boulder beside the tree. Cut Heaviness from it and carry the property to the lily pad over the submerged plate. Attach Heaviness; the lily pad sinks and activates the plate, revealing the secret passage.
5. **Slow the wheel and claim the bombs:** In the passage, cut Slowness from the nearby magical snail and attach it to the rapidly rotating wheel. Ride the slower wheel upward, jump to the chest platform, and collect the Moonlit Bombs. This route is hidden but required for progression.
6. **Climb to the giant tree:** At the tree’s base, step on the pressure plate. Temporary Hook targets activate in a clear sequence across the branches. Hook through the route before the targets disappear. If Tsuki falls, return her to a nearby starting point and allow another attempt.
7. **Open the summit passage:** At the top, use a Moonlit Bomb on the cracked opening and jump down. End on a brief view and story hint toward the next area.
8. **Optional Moon Fragment:** Use a bomb on the separate cracked hole behind the giant tree and enter the short route to a treasure chest containing a Moon Fragment.

### Lighting, palette, and sound

Use layered greens, pale canopy light, distant atmospheric haze, and occasional moonlit or magical highlights. The area should feel more vertical and spacious than the first two, with wind moving leaves and distant forest ambience reinforcing height. Make interactive objects stand apart from foliage through silhouette, value contrast, and their property identifiers. Use a distinct sound for the wheel slowing, the pulley shifting, the secret passage opening, temporary target activation, and the bomb opening cracked wood or stone.

### Readability checks

- The giant tree remains a recognizable destination throughout the area.
- The moving platform’s path, safe observation point, and next Hook target are visible.
- The counterweight’s rise and lift’s descent are easy to understand, as is the reversed motion when Lightness is removed.
- The upper gate clearly communicates a return shortcut.
- The lily-pad pressure plate and resulting secret passage are discoverable and understandable.
- The Moonlit Bomb route is hidden as a discovery but does not become optional or missable for progression.
- The wheel’s danger before Slowness and safe ride afterward are visually obvious.
- The summit target sequence communicates order, timing, and recovery clearly.
- The main summit opening and optional rear opening are visually distinct.

---

## 7. Items and collectibles

### Moonthread Hook

Introduced in the Sunken Crossing chest. The player aims at clearly marked Hook targets and pulls Tsuki through a short, direct arc. Range and target recognition should be forgiving; target feedback should make a valid anchor clear. Use it for the Area 2 exit, the optional return chest in Area 1, traversal in Area 3, and the summit climb.

### Moonlit Bombs

Found in the Whispering Canopy secret-feeling but progression-required route. They are an unlimited ability intended for clearly marked cracked surfaces. The summit cracked opening is required for the slice’s ending; the separate cracked opening behind the tree is optional and leads to a Moon Fragment.

### Moon Fragments

Optional, tracked collectibles found in chests. Place a small, achievable set across this slice, with the collectible system able to expand as future areas are added. Collecting every Moon Fragment in the full game unlocks a secret ending. The full-game secret ending is not included in this vertical slice and fragments do not gate its main route.

## 8. Ending and save behavior

After Tsuki uses a Moonlit Bomb on the summit opening, she jumps down through it. Show a brief glimpse and story hint of the next area, then end the slice before that area becomes playable. This gives the journey a forward-looking payoff while keeping scope to the three designed areas.

Save area progress, obtained tools, Moon Fragment count, and major puzzle states at checkpoints. Preserve resolved puzzle states where practical so a fall or reload does not force unnecessary repetition.

## 9. Art and implementation principles

- Use an original stylized fantasy look: colorful, readable shapes and materials, expressive environments, and a clear adventure feel. Treat the Ocarina of Time remake as inspiration, not a requirement to copy its exact assets or look.
- Keep source and receiver identifiers consistent across all areas. Do not rely on color alone; pair color with shape, glow, particles, or an icon.
- Compose puzzle spaces for third-person camera readability. Show the relevant source, receiver, mechanism, and intended result from a natural player viewpoint where possible.
- Establish the route with landmarks, lighting, silhouettes, and framing instead of relying on repeated text instructions.
- Keep puzzle solutions physically observable: plants react when cut, stones become easier to move, buoyant objects rise, heavy objects sink or press plates, platforms slow, counterweights move, and mechanisms visibly respond.
- Make recovery forgiving. Encourage players to experiment with transfers and traversal without losing substantial progress.

## 10. Vertical-slice completion criteria

The slice is complete when a new player can:

1. Learn the scissors transfer loop and Lightness/Heaviness in the Forest Entrance.
2. Cross the Sunken Crossing by applying Buoyancy, solve its crate/plate chain, and obtain the Moonthread Hook.
3. Return to the Forest Entrance chest if desired.
4. Reach and navigate the Whispering Canopy using the Hook, Lightness, and Slowness.
5. Find the hidden lily-pad route, obtain Moonlit Bombs, and understand that they are needed to continue.
6. Reach the top of the giant tree, open the summit passage, and see a clear glimpse of what lies beyond.
7. Find optional Moon Fragments without needing them to finish the main route.

The experience should communicate its puzzle rules through consistent behavior and visuals, offer optional contextual help, recover gracefully from mistakes, and feel like a complete three-area adventure rather than an isolated mechanics test.

## 11. Property Interaction Matrix

Property    |	    Target	          |       Gameplay response
------------------------------------------------------------------------
Lightness   |	    Heavy block	      |       Enables push/pull
Lightness   |	    Counterweight	  |       Changes lift position
Heaviness   |	    Pebble	          |       Activates pressure plate
Heaviness   |	    Crate	          |       Increases effective weight
Heaviness   |	    Lily pad	      |       Sinks to underwater switch
Buoyancy    |	    Platform	      |       Rises to surface
Buoyancy    |	    Crate	          |       Floats on water
Slowness    |	    Moving platform	  |       Reduces movement speed
Slowness    |	    Rotating wheel	  |       Reduces rotation speed



## 12. Asset Production Guide

Characters                        |         How I'd make it
----------------------------------------------------------------------------
Tsuki                             |         AI-generated humanoid mesh + auto-rig + Unreal animations
Fairy guide                       |         Simple 3D mesh + emmisive material + Niagara effects

Tsuki prompt for AI:
Full body original stylized 3D fantasy adventure game protagonist named Tsuki, a young adventurous girl with short dark lavender hair, expressive eyes, simple cream tunic, midnight blue short cape with a crescent moon emblem, leather boots, small belt pouch, whimsical hand-painted storybook style, clean chunky forms, game-ready humanoid proportions, arms slightly extended in an A-pose, legs separated, symmetrical neutral standing pose, plain neutral background, no objects held, no text.

Fairy prompt for AI:
An adorable tiny magical moon fairy companion for a stylized 3D fantasy adventure game. Small rounded glowing lavender body, expressive large eyes, two delicate translucent crescent-shaped wings, subtle celestial markings, whimsical cozy storybook art style, simple clean geometry, soft hand-painted colors, friendly curious personality, floating neutral pose, isolated character, plain background, no environment, no text.


Area 1 - Forest Entrance

Asset	                          |         How I'd make it
----------------------------------------------------------------------------
Trees, bushes, grass, rocks	      |         Fab environment pack
Paths and terrain	              |         Unreal Landscape
Feather plant	                  |         AI-generated
Heavy boulder	                  |         Fab rock, modified material
Blocking stone	                  |         Simple Unreal geometry
Pebble	                          |         Reuse a scaled rock mesh
Pressure plate	                  |         Blender or Unreal Modeling Mode
Ancient gate	                  |         Fab asset or Blender
Moon Scissors                     |         AI-generated static 3D mesh + Unreal effects

Key custom asset: Feather Plant.
This is one of the first magical things the player encounters, so give it a unique appearance.

Magical Scissor prompt for AI:
A magical pair of moon-themed scissors for a whimsical stylized 3D fantasy adventure game. Elegant oversized silver blades, crescent moon shaped handles, a small glowing lavender moonstone at the central pivot, subtle engraved celestial patterns, chunky readable silhouette, soft hand-painted textures, original fantasy design, isolated object, blades slightly open, neutral background, no hands, no text.


Area 2 — Sunken Crossing

Asset	                          |         How I'd make it
----------------------------------------------------------------------------
Water, riverbed, rocks	          |         Unreal + Fab environment pack
Magical water bubble	          |         Unreal sphere and traslucent material
Sunken platform	                  |         Blender/basic wooden meshes
Crate	                          |         Fab or primitive meshes
Treasure chest                    |         AI-generated
Moonthread Hook                   |         AI-generated
Hookshot targets                  |         Simple custom meshes

Key custom asset: Moonthread Hook.
I'd generate the decorative hook model, but keep its gameplay logic completely separate.

Area 3 — Whispering Canopy

Asset	                          |         How I'd make it
----------------------------------------------------------------------------
Giant hollow tree	              |         Modular tree meshes, custom assembly
Canopy platforms	              |         Modular wood pieces
Magical snail                     |         AI-generated
Fast-moving platform	          |         Blender/basic mesh
Counterweight pulley              |         Blender/basic mesh
Giant lily pad                    |         AI-generated
Rotating waterwheel               |         Blender
Moonlit Bombs                     |         AI-generated
Cracked tree openings             |         Separate custom destructible-looking meshes

Key custom asset: Magical Snail and Moonlit Bombs
I'd be particularly careful with the giant tree. It's a major landmark, so you want a distinctive silhouette, but don't rely on one AI-generated tree mesh to provide all the internal collision, climbing surfaces and hollow passages.
Build its functional layout with Unreal geometry and wrap the exterior in decorative tree meshes, roots, and foliage.

