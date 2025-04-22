
# Bomber Game

A terminal-based bombing game written in C using ncurses. Control a plane that drops bombs to destroy the city below!

![Gameplay Demo](demo.png) *(Bomber running)*

## Features
- **Zig-zag bomber movement** with directional flipping
- **Area-of-effect bombs** (3-block damage radius)
- **Destructible city terrain**
- **Smooth animations** with optimized rendering
- **Score system** (10 points per block destroyed)
- **Simple controls** (Space to bomb, Q to quit)

## How to Play
1. **Compile**:  
   ```bash
   gcc -o bomber bomber.c -lncurses
   ```
2. **Run**:  
   ```bash
   ./bomber
   ```
3. **Controls**:  
   - `Space` = Drop bomb  
   - `Q` = Quit game  

## Game Rules
- Destroy all city blocks (`#`) to win
- Crash into the city → Game Over
- Bomber automatically:
  - Moves left/right
  - Descends when hitting edges
  - Flips direction (`^==-` ↔ `-==^`)

## Customization
Edit `bomber.c` to change:
```c
#define DAMAGE_RADIUS 1    // Increase for bigger explosions
struct timespec ts_bomb   // Adjust bomb speed (nanoseconds)
int world[COLS]           // Modify city generation
```

## Dependencies
- `ncurses` library  
  Install with: 
  - Fedora: `sudo dnf install ncurses`
  - Ubuntu: `sudo apt install libncurses-dev`
  - Mac: `brew install ncurses`

