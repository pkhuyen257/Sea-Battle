# Sea Battle: Tactical Command

A C++ console strategy game

## What the game includes

- Console UI with a naval command-deck style
- Convoy escort mission
- Friendly ship movement
- Radar scan system
- Torpedo attacks
- Mine placement
- Enemy patrol and spotting logic
- Weather effects
- Save/load support

## Goal

Keep the convoy alive and escort it to safety before the mission timer runs out.

You lose if:

- the convoy is destroyed
- both support ships are destroyed
- the mission turn limit is exceeded before the convoy escapes

## Controls

Main menu:

- `1` New Game
- `2` Load Game
- `3` How To Play
- `4` Exit

In-game actions:

- `1` Move Ship
- `2` Radar Scan
- `3` Fire Torpedo
- `4` Deploy Mine
- `5` End Turn
- `6` Save Game

Movement input:

- `W` Up
- `A` Left
- `S` Down
- `D` Right

Coordinates use the format `A1` to `J10`.

## Enemy rules

Enemies do not always know where you are.

- Enemies can only chase or attack ships they can currently see.
- If they see nothing, they patrol left across the map.
- Each enemy does only one thing per turn:
  attack if in range, otherwise move.
- Enemies move one tile at a time.
- Enemy movement is not diagonal.
- Enemies prefer row movement first, then column movement.
- Mines explode when enemies move onto them.

Enemy stats:

- Patrol Boat: sight `3`, range `2`, damage `1`
- Raider: sight `4`, range `3`, damage `2`
- Submarine: sight `3`, range `4`, damage `2`

## Symbols

- `~` Water
- `D` Destroyer
- `R` Radar Ship
- `C` Convoy
- `X` Hit
- `o` Miss
- `?` Radar signal
- `M` Mine

## Build

### Option 1: PowerShell script

```powershell
./build.ps1
```

### Option 2: Makefile

This project includes a `Makefile` for MinGW:

```powershell
mingw32-make TARGET=sea_battle.exe
```

## Run

```powershell
./sea_battle.exe
```

## Save file

The game saves to:

```text
savegame.txt
```

## Project files

- `main.cpp`: program entry point
- `GameEngine.*`: game loop and turn handling
- `Board.*`: board state and rendering helpers
- `Ship.*`: ship data and HP handling
- `EnemyManager.*`: enemy spawning, movement, spotting, and attacks
- `Mission.*`: win/lose rules
- `UIRenderer.*`: terminal UI
- `SaveManager.*`: save/load logic
- `Types.h`: shared types and coordinate parsing

## Notes

- The UI uses ANSI color codes. It works best in PowerShell or Windows Terminal.
- If `sea_battle.exe` is open while compiling, Windows may block overwriting it. Close the game first, then rebuild.
