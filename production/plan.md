# Tsuki and the Moon — Implementation Plan

Source of truth: `design/gdd/tsuki-and-the-moon.md` (the GDD). This plan adds no mechanics, enemies, bosses or progression beyond it. Feel reference: the Ocarina of Time remake, for *how things feel* only — no assets, layouts or content copied.

## 0. Where the project is (2026-11-13)

- UE 5.8 C++ project, one runtime module (`Tsuki_and_the_Moon`, Enhanced Input linked). Source is the empty template module.
- `Content/` is empty, and the editor confirms `/Game` has no assets. Default map is the engine's OpenWorld template.
- Unreal MCP is connected (all editor toolsets available).
- The earlier M1 build (TsukiCharacter, scissors, plates, Area 1 graybox) is **not in this folder or any sibling project**. The repo was re-initialised; only the old `plan.md` survives on `origin/claude/project-thread-5r1dx6`. This plan starts from zero and reuses that plan's architecture.
- Claude Code Game Studios framework is installed (`.claude/`, `project.yaml`), but the engine is not yet set in `project.yaml` and `CLAUDE.md` still shows `[CHOOSE]` placeholders.
- `design.md` was moved to `design/gdd/tsuki-and-the-moon.md` (git shows it as a delete until committed).
- Git LFS is installed but `.gitattributes` does not track `*.uasset` / `*.umap`.

Studio commands: `/project-stage-detect`, `/adopt`, `/create-epics` and `/create-stories` are installed. To keep docs minimal, their output is folded into this file: stage = **Pre-production, empty engine project**; each milestone below is an epic, each numbered task a story with its check.

## 1. Architecture

C++ owns rules and state; Blueprints own content, tuning, feedback and level scripting. Gameplay numbers live in data assets / a settings object, not code.

| C++ class | Job | Milestone |
|---|---|---|
| `EMoonProperty`, `UMoonPropertyData` | Property enum; one data asset per property (color, icon shape, Niagara, SFX) | M1 |
| `UPropertyHolderComponent` | Properties an actor has, which are cuttable / attachable, effective weight, Slowness time scale, original source for returns | M1 |
| `UScissorsComponent` | Aim, highlight, valid action (cut / attach / none), one carried property | M1 |
| `ATsukiCharacter` | Movement, camera, input, owns the components | M1 |
| `UGrabComponent` | Push/pull, allowed only under a weight limit | M1 |
| `APressurePlate` | Sums effective weight vs threshold, depresses, fires events | M1 |
| `ACheckpoint`, `UTsukiProgressSubsystem` | Respawn on fall; tools, fragments, solved puzzles | M1 |
| `AWaterBody` | Water height, Buoyancy rise/float, lowerable surface | M2 |
| `UHookComponent`, `AHookTarget` | Moonthread Hook, enable/disable targets | M2 |
| `UBombComponent`, `ACrackedSurface` | Moonlit Bombs on marked cracked surfaces | M3 |
| `UTsukiSaveGame` | Save at checkpoints | M4 |

Maps: one persistent map `L_World` with streamed `L_ForestEntrance`, `L_SunkenCrossing`, `L_WhisperingCanopy`, so the Area 1 return and canopy shortcut are seamless.

Defaults carried from the earlier plan: an object can hold several properties (Lightness + Heaviness cancel); small objects such as the pebble are lifted overhead and carried (Interact to pick up / put down, free walking), Ocarina-style; large blocks are pushed and pulled.

## 2. Feel targets (OoT remake as reference)

| Thing | Target feel | Tuning knobs |
|---|---|---|
| Movement | Quick acceleration, short stop, tight turn; jump is forgiving (coyote time, small ledge assist). Camera recentres behind Tsuki on demand and frames puzzle objects lightly. | Walk/run speed, accel, jump height, coyote time, camera lag |
| Push/pull box | Grab, then the object moves slowly at a steady speed with a scrape loop and dust; it stops dead on release. Too-heavy objects shudder and refuse. | Push speed, weight limit, optional step length |
| Pressure plate | Plate sinks with a clunk; for key plates a short camera cut shows the result, a solve jingle plays, control returns in about 2–3 s. | Activation weight, reveal duration |
| Moonthread Hook | Reticle changes when a valid target is in range; the line fires straight, then pulls Tsuki fast in a short arc and lands her on the anchor's ledge. | Range, pull speed, aim assist cone |
| Moonlit Bomb | Tsuki takes out a bomb, places or tosses it at a marked cracked surface; fuse flashes faster before a burst. Unlimited, only affects cracked surfaces (per GDD). | Fuse time, toss distance |

## 3. Milestones

Each one ends when the matching GDD §10 criteria can be shown in a graybox playthrough. Art and audio come in M4.

| # | Milestone | Done when |
|---|---|---|
| M0 | Project setup | Template character runs in `L_World`; LFS tracking assets; `project.yaml` engine set |
| M1 | Core loop + Forest Entrance | Criterion 1 |
| M2 | Sunken Crossing + Hook | Criteria 2–3 |
| M3 | Whispering Canopy + ending | Criteria 4–6 |
| M4 | Whole-slice pass | Criterion 7 and §10 closing paragraph |

### M0 — Setup
1. Set the engine in `project.yaml` (Unreal 5.8, C++ + Blueprint). ✔ `/project-settings` shows it.
2. Add Git LFS tracking for `*.uasset`, `*.umap` and common source art. ✔ `git lfs track` lists them.
3. Add the Third Person content pack (placeholder mannequin, anims, input). ✔ Mannequin runs in a test map.
4. Create folders and `L_World` with the three streamed sublevels; make it the default map. ✔ PIE starts there.

### M1 — Core loop + Forest Entrance
1. Property enum, data assets (all four; Buoyancy and Slowness unused yet). ✔ Assets show colors.
2. `UPropertyHolderComponent` + automation test (weight, add/remove events). ✔ Test passes.
3. Source glow (property color + shape cue) and receiver glow (neutral white). ✔ Boulder and pebble read differently.
4. `ATsukiCharacter` + `BP_Tsuki` with the movement feel targets. ✔ Feel check in PIE.
5. `UScissorsComponent`: aim, cut, attach, one-carried limit, cut back off, return. ✔ All five GDD §3 steps work.
6. `WBP_HUD`: carried property, contextual prompt, fragment count. ✔ Updates through a transfer.
7. `UGrabComponent` push/pull with the feel targets. ✔ Stone refuses; with Lightness it moves.
8. `APressurePlate` + `BP_Gate` + camera reveal and jingle. ✔ Heavy pebble opens gate, control back in ~3 s.
9. Area 1 graybox: wake clearing → short fairy path → scissors resting place → stone bay with feather plant → gate clearing (plate, pebble, boulder) → glimpse toward Area 2; high platform with chest out of reach. ✔ Area 1 readability checks.
10. Checkpoints and fall recovery; solved puzzles stay solved. ✔ Fall after the gate; gate stays open.
11. Opening beat: Tsuki wakes with no scissors, the fairy greets her and flies ahead along a short, safe path to the scissors' resting place; pickup plays the tool cue (hold-up pose, jingle) and one fairy line. Scissors input does nothing before pickup. ✔ Control in under ~30 s; scissors reached in under ~1 min.
12. Fairy hints, two tiers, for both puzzles. ✔ Both tiers appear.
13. Playtest and fix blockers. ✔ Criterion 1.

### M2 — Sunken Crossing + Hook
1. `AWaterBody`: Buoyancy rise/float, lowerable surface. ✔ Buoyant crate floats and can be pushed.
2. Area 2 graybox: bank, bubbles, submerged platform, crate ledge, boulder, plate + sluice, hidden chest, exit target. ✔ Area 2 readability checks.
3. Puzzle chain: raise bridge, float crate, Heaviness, plate lowers pond with reveal. ✔ Chest becomes reachable.
4. Chest + Hook pickup cue + fairy line. ✔ Hook in progress state.
5. `UHookComponent` / `AHookTarget` with the feel targets. ✔ Hook to exit target.
6. Area 1 high chest becomes reachable; first Moon Fragment. ✔ Count shows 1.
7. Hints for Area 2; playtest. ✔ Criteria 2–3.

### M3 — Whispering Canopy + ending
1. Slowness on moving platform (time scale). ✔ Platform slows; hook to next.
2. Pulley lift + counterweight driven by effective weight. ✔ Ride up after cutting Lightness while aboard.
3. Upper plate + shortcut gate. ✔ Route back avoids the lift.
4. Lily pad sinks onto submerged plate; secret path reveal. ✔ Passage opens.
5. Rotating wheel + Slowness; bomb chest. ✔ Bombs obtained.
6. `UBombComponent` / `ACrackedSurface` with the feel targets. ✔ Cracked surface breaks.
7. Tree base plate + timed hook target sequence with recovery point. ✔ Summit reachable, fall resets.
8. Summit opening, jump down, glimpse of next area, slice end. ✔ Ending plays.
9. Rear cracked hole + optional fragment chest. ✔ Count increments.
10. Area 3 graybox and hints; playtest. ✔ Criteria 4–6.

### M4 — Whole-slice pass
1. Save/load at checkpoints (area, tools, fragments, puzzles). ✔ Reload restores state.
2. Art pass per GDD §12; lighting per area brief. ✔ Screenshots in `production/qa/evidence/`.
3. Music, area motifs, SFX list from the area briefs. ✔ Every listed cue plays.
4. Full playtest by a new player. ✔ All §10 criteria.

## 4. Assets to import, per milestone

Placeholders are fine until M4. You don't need to import anything for M0 or M1 beyond the Third Person pack; the rest can be engine primitives.

| Milestone | Import (you) | I'll build in-engine |
|---|---|---|
| M0 | Third Person content pack (Add Feature or Content Pack) | Folders, maps |
| M1 | Optional now: a Fab stylized forest pack (trees, rocks, grass) | Stone, boulder, pebble, plate, gate, platform, chest, feather plant from primitives; glow materials; Lightness/Heaviness Niagara; HUD |
| M2 | Optional: a Fab crate and water/riverbed pack | Water body, bubble (sphere + translucent material), sunken platform, sluice, hook targets |
| M3 | Optional: modular wood/tree pieces for canopy platforms | Moving platform, pulley, lily pad, wheel, cracked surfaces, tree blockout |
| M4 | Tsuki (AI mesh + auto-rig), fairy mesh, Moon Scissors, feather plant, treasure chest, Moonthread Hook, magical snail, giant lily pad, Moonlit Bombs (all per GDD §12 prompts); Fab environment packs; music and SFX | Materials, Niagara polish, swaps of placeholders |

### M4 import checklist

Look: original stylized fantasy in the spirit of the Ocarina remake: chunky readable shapes, saturated but soft colors, hand-painted-looking textures, warm light. No assets or content copied from it.

**Rules for every import:** FBX (or GLB) in centimeters, Z-up, facing +X, pivot at the bottom center (chest lid: pivot at the hinge); textures 2K max; names `SK_` skeletal, `SM_` static, `T_` textures, `A_` anims, `S_`/`MUS_` audio. Drop into `Content/Tsuki/Art/{Characters,Props,Environment}` and `Content/Tsuki/Audio/{Music,SFX}`.

| # | Asset | How to make it | Notes |
|---|---|---|---|
| 1 | **Tsuki** (`SK_Tsuki`) | AI 3D generator (Meshy / Tripo / Rodin) with the GDD §12 prompt, A-pose → rig with Mixamo auto-rigger (or UE IK Retargeter onto Manny) | ~1.5 m tall; I retarget the anims |
| 2 | **Tsuki animations** | Mixamo (same rig) | idle, walk, run, jump start/loop/land, push, pull, carry idle + carry walk, throw, pick up/put down, swim idle + swim, item-get (both arms up), aim (one arm forward), climb up ledge, fall |
| 3 | **Fairy** | AI generator with the GDD prompt, or I build it in-engine (glow sphere + wings + Niagara) | Static mesh is enough |
| 4 | Moon Scissors | AI generator, GDD prompt | Static; ~40 cm |
| 5 | Feather plant | AI generator: "magical plant with pale airy feather fronds, stylized" | Two states look nice (fronds up / drooped) but one mesh is fine |
| 6 | Treasure chest | AI generator **or** Fab | Body and lid as **separate** meshes |
| 7 | Moonthread Hook | AI generator | Static; held item |
| 8 | Magical snail | AI generator: "cute glowing fantasy snail, slow, sleepy" | Static |
| 9 | Giant lily pad | AI generator or Blender (flat disc + notch) | ~3 m wide, flat underside |
| 10 | Moonlit Bomb | AI generator: "round moon-blue bomb with crescent fuse" | ~35 cm |
| 11 | Cracked wall / cracked lid | Blender or AI ("cracked stone slab with glowing seams") | One wall, one floor slab |
| 12 | Forest environment | Fab: one free/paid **stylized** forest pack (trees, bushes, grass, rocks, cliffs, wooden planks) | Pick one pack so the style matches |
| 13 | Crate, stone blocks, boulders | Same Fab pack, or Fab "stylized props" | |
| 14 | Water | I set up a stylized water material; a Fab stylized water pack is optional | |
| 15 | Music | Original or properly licensed: 3 area loops (Forest, Sunken, Canopy), puzzle-solved jingle (2-3 s), item-get fanfare (3-4 s), ending piece | OGG/WAV 44.1 kHz |
| 16 | SFX | Freesound (CC0) / Sonniss GDC bundles | snip (cut), attach chime, return, plate clunk, gate rumble, water splash/swim, bubble pop, stone scrape (push), hook fire / latch / reel, bomb fuse / explosion, rubble, chest open, footsteps (grass/stone/wood), wheel creak, pulley creak, wind |

## 5. Questions

- The earlier M1 build is gone from this folder. If you have it elsewhere (another PC, a backup), tell me and I'll pick it up instead of rebuilding.
