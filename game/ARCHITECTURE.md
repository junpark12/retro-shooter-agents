# Galaxy Storm — Architecture Document

## Overview

Galaxy Storm is a 90s-style vertical scrolling arcade shooter built with C++17 and SDL2.
Resolution: 480×640 (portrait). Target: Windows native (.exe).

---

## Game State Machine

```
TITLE → PLAYING → STAGE_CLEAR → [next stage or VICTORY]
                ↘ GAMEOVER
```

| State        | Description                              |
|--------------|------------------------------------------|
| TITLE        | Main menu, high score display            |
| PLAYING      | Active gameplay, spawning enemies/waves  |
| STAGE_CLEAR  | Brief transition screen between stages   |
| GAMEOVER     | Player lost all lives                    |
| VICTORY      | All 3 stages cleared                     |

---

## Module Dependency Graph

```
main.cpp
  └── Game
        ├── Background    (no deps)
        ├── Menu          (no deps)
        ├── Stage         → Enemy, Boss
        ├── Player        → Bullet
        ├── Enemy         → Bullet
        ├── Boss          → Bullet, Enemy
        ├── BulletPool    (no deps)
        ├── PowerUp       (no deps)
        ├── Collision     → Player, Enemy, Boss, Bullet, PowerUp
        ├── HUD           → Player (score, lives, power)
        └── Sprites       (utility, no deps)
```

All modules share `types.h` (common enums/structs) and `entity.h` (base Entity).

---

## Directory Layout

```
game/
├── CMakeLists.txt
├── src/
│   ├── main.cpp          ← entry point, SDL init
│   ├── game.cpp          ← game loop, state management
│   ├── player.cpp
│   ├── enemy.cpp
│   ├── bullet.cpp
│   ├── boss.cpp
│   ├── collision.cpp
│   ├── stage.cpp
│   ├── powerup.cpp
│   ├── sprites.cpp
│   ├── hud.cpp
│   ├── menu.cpp
│   └── background.cpp
└── include/              ← all .h headers live here
    ├── types.h
    ├── entity.h
    ├── game.h
    ├── player.h
    ├── enemy.h
    ├── bullet.h
    ├── boss.h
    ├── collision.h
    ├── stage.h
    ├── powerup.h
    ├── sprites.h
    ├── hud.h
    ├── menu.h
    └── background.h
```

---

## Module Descriptions

### `types.h` — Shared types
- `GameState` enum: TITLE, PLAYING, STAGE_CLEAR, GAMEOVER, VICTORY
- `EnemyType` enum: SMALL, MEDIUM, LARGE
- `PowerUpType` enum: SPREAD, LASER, MISSILE, SHIELD
- `BulletOwner` enum: PLAYER, ENEMY
- `Vec2` struct: float x, y + operators
- `Rect` struct: float x, y, w, h
- `rectsOverlap(a, b)` inline AABB helper
- Constants: SCREEN_W=480, SCREEN_H=640, FPS=60, MAX_BULLETS=256, MAX_ENEMIES=64

### `entity.h` — Base entity
- `Entity` struct: Vec2 pos, Vec2 vel, Rect bounds, bool active, int hp
- `worldBounds()` → world-space AABB

### `game.h` — Game controller
- `Game` class: `init()`, `run()`, `shutdown()`
- Private: `handleEvents()`, `update(float dt)`, `render()`
- Private: `startStage(int)`, `onStageClear()`, `onGameOver()`
- Owns all subsystem pointers (Player, BulletPool, EnemyPool, Boss, PowerUpPool, Stage, Background)

### `player.h/cpp` — Player ship
- `Player` extends Entity: lives, score, powerType, shieldTimer, fireTimer, invincibleTimer
- `initPlayer(Player&)`, `updatePlayer(Player&, float dt, BulletPool&)`
- `renderPlayer(SDL_Renderer*, const Player&)` — delegates to sprites
- 8-direction keyboard movement (SDL_GetKeyboardState), screen boundary clamping
- Spacebar fires; power type determines bullet pattern

### `enemy.h/cpp` — Enemy system
- `Enemy` extends Entity: EnemyType, moveTimer, pointValue
- `EnemyPool` struct: Enemy pool[MAX_ENEMIES]
- `spawnEnemy(EnemyPool&, EnemyType, Vec2 pos)`, `updateEnemies(EnemyPool&, float dt, const Player&)`
- `renderEnemies(SDL_Renderer*, const EnemyPool&)`
- SMALL: straight down. MEDIUM: sine wave. LARGE: tracks player.

### `bullet.h/cpp` — Bullet object pool
- `Bullet` extends Entity: BulletOwner owner, int damage
- `BulletPool` struct: Bullet pool[MAX_BULLETS]
- `fireBullet(BulletPool&, Vec2 pos, Vec2 vel, BulletOwner, int dmg)`
- `updateBullets(BulletPool&, float dt)`, `renderBullets(SDL_Renderer*, const BulletPool&)`

### `boss.h/cpp` — Boss system
- `Boss` extends Entity: stageNum, phase (1–3), attackTimer, moveTimer
- `initBoss(Boss&, int stageNum)`, `updateBoss(Boss&, float dt, BulletPool&, const Player&)`
- `renderBoss(SDL_Renderer*, const Boss&)` — delegates to sprites

### `collision.h/cpp` — Collision detection (AABB)
- `checkBulletEnemyCollision(BulletPool&, EnemyPool&, Player&)` → score
- `checkBulletPlayerCollision(BulletPool&, Player&)`
- `checkPlayerEnemyCollision(Player&, EnemyPool&)`
- `checkBulletBossCollision(BulletPool&, Boss&, Player&)`
- `checkPowerUpPickup(Player&, PowerUpPool&)`

### `stage.h/cpp` — Stage/wave manager
- `Stage` struct: stageNum, waveIndex, spawnTimer, enemiesKilled, bossSpawned, stageCleared
- `initStage(Stage&, int num)`, `updateStage(Stage&, float dt, EnemyPool&, Boss&)`

### `powerup.h/cpp` — Power-up items
- `PowerUp` extends Entity: PowerUpType type
- `PowerUpPool` struct: PowerUp pool[MAX_POWERUPS]
- `spawnPowerUp(PowerUpPool&, Vec2 pos, PowerUpType)`
- `updatePowerUps(PowerUpPool&, float dt)`, `renderPowerUps(SDL_Renderer*, const PowerUpPool&)`

### `sprites.h/cpp` — Programmatic sprite rendering
- Pure rendering utilities — no state, all SDL2 primitives
- `renderPlayerSprite(SDL_Renderer*, int x, int y)` — cyan triangle fighter
- `renderEnemySprite(SDL_Renderer*, int x, int y, EnemyType)` — shape/colour by type
- `renderBossSprite(SDL_Renderer*, int x, int y, int stageNum)`
- `renderBulletSprite(SDL_Renderer*, int x, int y, BulletOwner)`
- `renderPowerUpSprite(SDL_Renderer*, int x, int y, PowerUpType)`
- `renderExplosion(SDL_Renderer*, int x, int y, int frame)` — animated particle burst
- 90s neon palette: cyan (#00FFFF), magenta (#FF00FF), neon-green (#39FF14), yellow (#FFE800)

### `hud.h/cpp` — Heads-Up Display
- `renderHUD(SDL_Renderer*, const Player&, int stageNum)` — score, lives, power bar
- `renderStageClear(SDL_Renderer*, int stageNum, int score)`
- `renderGameOver(SDL_Renderer*, int score)`
- `renderVictory(SDL_Renderer*, int score)`
- Pixel font drawn with SDL_RenderFillRect

### `menu.h/cpp` — Title/Menu screen
- `Menu` struct: selectedOption, blinkTimer
- `renderMenu(SDL_Renderer*, const Menu&)` — "GALAXY STORM" title, START, QUIT
- `updateMenu(Menu&, const SDL_Event&)` → returns GameState

### `background.h/cpp` — Starfield
- `Star` struct: Vec2 pos, float speed, Uint8 brightness, int layer
- `Background` struct: Star stars[MAX_STARS]
- `initBackground(Background&)`, `updateBackground(Background&, float dt)`
- `renderBackground(SDL_Renderer*, const Background&)` — 3-layer parallax, neon-tinted stars

---

## Data Flow (one frame)

```
SDL_PollEvent → handleEvents()
                    │
            ┌───────▼────────┐
            │  state == TITLE │ → updateMenu / renderMenu
            └───────┬────────┘
                    │ state == PLAYING
            ┌───────▼────────────────────────────────────┐
            │ updateBackground(dt)                        │
            │ updatePlayer(dt, bullets)                   │
            │ updateStage(dt, enemies, boss)              │
            │ updateEnemies(dt, player)                   │
            │ updateBoss(dt, bullets, player)             │
            │ updateBullets(dt)                           │
            │ updatePowerUps(dt)                          │
            │ checkCollisions(...)                        │
            └───────┬────────────────────────────────────┘
                    │
            ┌───────▼────────┐
            │   render()     │ bg → enemies → boss → powerups
            │                │   → player → bullets → HUD
            └────────────────┘
```

---

## Coding Conventions (Team-wide)

| Item | Style |
|------|-------|
| Namespace | `galaxy` (all code) |
| Structs / Classes | `PascalCase` |
| Functions / variables | `camelCase` |
| Constants | `UPPER_SNAKE_CASE` |
| Header guard | `#pragma once` |
| Assets | SDL2 primitives only — no external files |
| Object pooling | Bullets, enemies, power-ups, stars |
| Timestep | Fixed 60 FPS; `float dt` in seconds passed to every `update()` |
