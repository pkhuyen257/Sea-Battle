# Sea Battle: Command Deck

## Quick Summary

You are protecting a convoy.

Your job is to:

- keep the convoy alive
- block enemy ships
- find threats with the radar ship
- destroy enemies before they reach the convoy

You win when the convoy reaches the safe zone.

You lose if:

- the convoy is destroyed
- both support ships are destroyed
- the mission timer runs out before the convoy escapes

## What Happens Each Turn

Every turn follows the same pattern:

1. You look at the board and choose one action.
2. Your action happens immediately.
3. The convoy moves forward automatically.
4. Enemies take their turn.
5. Weather may change.
6. The next turn begins.

Important:

- You only get one main action each turn.
- The convoy keeps moving, so waiting too long can be dangerous.

## Your Ships

### Destroyer

The destroyer is your main combat ship.

- It can move.
- It can fire torpedoes.
- Its torpedo range is longer than the radar ship's.
- It can deploy mines.
- It is best used to stay near the convoy and intercept enemies.

Best use:

- protect the convoy
- attack nearby enemies
- place mines in enemy approach paths

### Radar Ship

The radar ship is your scouting ship.

- It can move.
- It can actively scan any 3x3 square on the board.
- It also has passive 3x3 sight around itself.

This means:

- if an enemy is in the 3x3 area around the radar ship, you can detect it nearby
- you can also choose any location on the board and scan a 3x3 area there

Best use:

- search ahead of the convoy
- reveal likely enemy paths
- support the destroyer

### Convoy

The convoy is the ship you must protect.

- It moves automatically.
- It does not attack.
- If it is destroyed, the game ends.

Best use:

- keep the destroyer close to it
- clear threats before they get into attack range

## Actions

### 1. Move Ship

Move either:

- Destroyer
- Radar Ship

Movement keys:

- `W` up
- `A` left
- `S` down
- `D` right

You cannot move:

- outside the map
- into another friendly ship

### 2. Radar Scan

Radar scan checks a 3x3 area.

Current rule:

- you can scan any 3x3 square on the board

Use it when:

- you think enemies are approaching from a specific area
- you want to check far ahead of the convoy
- you need information before firing

### 3. Fire Torpedo

This attacks one coordinate.

- The destroyer has the better torpedo reach.
- Weather can reduce hit reliability.

Use it when:

- you already know or strongly suspect an enemy position
- an enemy is getting close to the convoy

### 4. Deploy Mine

This places a mine on the board.

- Mines are traps.
- Enemies that move onto them take damage immediately.

Use mines:

- in likely enemy paths
- in front of the convoy
- near choke points

### 5. End Turn

This skips your action and lets the enemy phase happen.

Usually only use this when:

- you already have good positioning
- you want the convoy to move
- you are setting up a mine trap

### 6. Save Game

This writes your current game to `savegame.txt`.

## Coordinates

The map uses coordinates like:

- `A1`
- `D5`
- `J10`

Columns:

- `A` to `J`

Rows:

- `1` to `10`

## Board Symbols

- `~` water
- `D` destroyer
- `R` radar ship
- `C` convoy
- `X` hit
- `o` miss
- `?` detected enemy activity
- `M` mine

## How Enemies Work

Enemies do not always know where you are.

They follow these rules:

- they can only chase or attack ships they can currently see
- if they cannot see anything, they patrol left
- they do only one thing per turn:
  attack if already in range, otherwise move
- they move one tile at a time
- they do not move diagonally
- they prefer row movement first, then column movement
- they do not stack on each other

## Enemy Types

### Patrol Boat

- sight: `3`
- attack range: `2`
- damage: `1`

This is a light enemy.
It is weaker, but still dangerous if it reaches the convoy.

### Raider

- sight: `4`
- attack range: `3`
- damage: `2`

This is a serious threat.
Raiders can damage the convoy quickly if ignored.

### Submarine

- sight: `3`
- attack range: `4`
- damage: `2`

This is the most dangerous enemy at range.
It can attack from farther away and is harder to track.

## Weather

Weather can change during the game.

Weather affects:

- scan reliability
- attack reliability

If the weather gets worse:

- radar information may be less precise
- attacks are less dependable

## Good Beginner Strategy

If you are new, use this plan:

1. Keep the destroyer near the convoy.
2. Use the radar ship to scout ahead.
3. Scan areas in front of the convoy, not random places.
4. Attack submarines and raiders first.
5. Place mines where enemies are likely to pass.
6. Do not let the convoy move forward without support.

## Common Mistakes

### Letting the destroyer drift too far away

If the destroyer leaves the convoy, the convoy becomes much easier to hit.

### Scanning random areas

Radar is strongest when you use it where enemies are likely to be.

### Saving mines for too long

Mines are strongest when placed early in useful lanes.

### Focusing only on one enemy

Even if one enemy is almost dead, another enemy may be a bigger convoy threat.

## Menu Controls

Main menu:

- `1` New Game
- `2` Load Game
- `3` How To Play
- `4` Exit

## Running the Game

Build:

```powershell
mingw32-make TARGET=sea_battle.exe
```

Run:

```powershell
./sea_battle.exe
```

## Final Advice

This game is about positioning, not just attacking.

If you keep the convoy protected, scan ahead, and stop enemies before they get close, the mission becomes much easier.
