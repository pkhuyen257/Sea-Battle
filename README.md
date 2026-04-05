# Sea Battle: Tactical Command


## Included systems

- Command-deck style UI
- Mission-based escort objective
- Friendly fleet movement
- Radar scan clues
- Torpedo attacks
- Mine deployment
- Hidden enemy phase
- Dynamic weather
- Save/load support

## Build

From PowerShell:

```powershell
./build.ps1
```

Then run:

```powershell
./sea_battle.exe
```

## Gameplay notes

- The convoy advances automatically each turn.
- The mission ends in victory when the convoy reaches the safe zone.
- The mission fails if the convoy is destroyed, both combat ships are lost, or the turn limit expires.
- Weather changes each turn and affects tactical reliability.
