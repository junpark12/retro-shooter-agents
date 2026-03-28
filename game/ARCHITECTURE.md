# Galaxy Storm — Architecture Document (Bullet Hell Edition)

## Overview

Galaxy Storm is a **Bullet Hell** vertical scrolling arcade shooter (Gunvein/CAVE style) built with C++17, SDL2, SDL2_image, SDL2_mixer, and SDL2_ttf.
Resolution: 480×640 (portrait). Target: Windows native (.exe).

---

## Game State Machine

```
TITLE → SHIP_SELECT → PLAYING → STAGE_CLEAR → [next stage or VICTORY]
                               ↘ GAMEOVER
```

| State        | Description                                           |
|--------------|-------------------------------------------------------|
| TITLE        | Main menu, high score display                         |
| SHIP_SELECT  | 3-ship selection screen (Bagon/Damul/Gunex)           |
| PLAYING      | Active gameplay, spawning enemies/waves               |
| STAGE_CLEAR  | Brief transition screen between stages                |
| GAMEOVER     | Player lost all lives                                 |
| VICTORY      | All 3 stages cleared                                  |

---

## Module Dependency Graph

```
main.cpp
  └── Game
        ├── AssetManager   (SDL2_image, pre-loads all PNG sprites)
        ├── AudioManager   (SDL2_mixer, BGM + SFX)
        ├── Background     → AssetManager
        ├── Menu           → AssetManager, TTF_Font
        ├── ShipSelect     → AssetManager, TTF_Font
        ├── Stage          → Enemy, Boss
        ├── Player         → Bullet, BulletPattern
        ├── Enemy          → Bullet, BulletPattern
        ├── Boss           → Bullet, BulletPattern
        ├── BulletPool     (no deps)
        ├── BulletPattern  → BulletPool
        ├── PowerUp        → AssetManager
        ├── Collision      → Player, Enemy, Boss, Bullet, PowerUp, AudioManager
        ├── HUD            → Player, AssetManager, TTF_Font
        └── Sprites        → AssetManager (utility, no state)
```

All modules share `types.h` (common enums/structs) and `entity.h` (base Entity).

---

## Directory Layout

```
game/
├── CMakeLists.txt
├── src/
│   ├── types.h              ← shared types, ShipType, BulletPattern, etc.
│   ├── entity.h             ← base Entity with circular hitRadius
│   ├── main.cpp             ← entry point
│   ├── game.cpp             ← game loop + state machine (TITLE→SHIP_SELECT→PLAYING)
│   ├── player.cpp           ← 3-ship types, lock-on, charge shot, bomb
│   ├── enemy.cpp            ← 5 enemy types, pattern-based shooting
│   ├── bullet.cpp           ← object pool (MAX_BULLETS=500)
│   ├── bullet_pattern.cpp   ← Bullet Hell pattern generator
│   ├── boss.cpp             ← multi-phase boss (3 phases per boss, 3 bosses)
│   ├── collision.cpp        ← AABB + circular hitbox (small hitbox for player)
│   ├── stage.cpp            ← wave/stage manager
│   ├── powerup.cpp          ← 6 power-up types
│   ├── asset_manager.cpp    ← SDL2_image PNG loader with cache
│   ├── audio.cpp            ← SDL2_mixer BGM/SFX manager
│   ├── sprites.cpp          ← sprite rendering (asset-based + primitive fallback)
│   ├── hud.cpp              ← HUD with SDL2_ttf
│   ├── menu.cpp             ← title + ship select screen
│   └── background.cpp       ← parallax starfield + nebula
└── assets/
    ├── sprites/
    │   ├── player/          ← ship_bagon.png, ship_damul.png, ship_gunex.png
    │   ├── enemies/         ← enemy_small.png, enemy_medium.png, etc.
    │   ├── bosses/          ← boss_1.png, boss_2.png, boss_3.png
    │   ├── bullets/         ← bullet_player.png, bullet_enemy.png, etc.
    │   ├── effects/         ← explosion_0..3.png
    │   ├── powerups/        ← powerup_spread.png, etc.
    │   ├── backgrounds/     ← space_bg.png
    │   └── ui/              ← life_icon.png, bomb_icon.png, power_bar.png
    ├── bgm/                 ← bgm_title.ogg, bgm_stage1.ogg, etc.
    ├── sfx/                 ← sfx_shoot.wav, sfx_explode_small.wav, etc.
    ├── fonts/               ← arcade font TTF
    └── CREDITS.md
```

---

## Module Descriptions

### `types.h` — Shared types
- `GameState`: TITLE, SHIP_SELECT, PLAYING, STAGE_CLEAR, GAMEOVER, VICTORY, QUIT
- `ShipType`: BAGON, DAMUL, GUNEX
- `EnemyType`: SMALL, MEDIUM, LARGE, FAST, ARMORED
- `PowerUpType`: SPREAD, LASER, MISSILE, SHIELD, BOMB, POWER
- `BulletOwner`: PLAYER, ENEMY
- `BulletPattern`: SINGLE, SPREAD_3/5, CIRCLE_8/16, SPIRAL_CW/CCW, AIMED, AIMED_SPREAD, CURTAIN, RANDOM_SPREAD, HOMING
- `Vec2`: float x/y + operators + length() + normalized()
- `Rect`: float x/y/w/h
- `rectsOverlap()`, `circlesOverlap()` inline helpers
- Constants: SCREEN_W=480, SCREEN_H=640, FPS=60, MAX_BULLETS=500, MAX_ENEMIES=64, MAX_LOCK_TARGETS=8
- `SCORE_PER_EXTRA_LIFE = 100000`

### `entity.h` — Base entity
- `Entity`: Vec2 pos/vel, Rect bounds, bool active, int hp, float hitRadius, center()
- `hitRadius` used for circular bullet collision (Bullet Hell small hitbox)

### `asset_manager.h/cpp` — SDL2_image asset loader
- Pre-loads all PNG sprites at init
- `get(key)` returns cached SDL_Texture* or nullptr
- Graceful fallback: nullptr → primitive rendering

### `audio.h/cpp` — SDL2_mixer audio manager
- `playBGM(key)` — loops BGM with fade-in
- `playSFX(key)` — plays one-shot sound effect
- Silently disabled if SDL2_mixer unavailable

### `game.h/cpp` — Game controller
- `Game::init()` — SDL2 + SDL2_image + SDL2_mixer + SDL2_ttf init, load assets
- State machine: TITLE → SHIP_SELECT → PLAYING
- Owns TTF_Font* for HUD/menu text rendering
- `selectedShip_` remembers chosen ship type

### `player.h/cpp` — Player ship (3 types)

| Ship | Name  | Fire Pattern | Special |
|------|-------|--------------|---------|
| BAGON | Focused | Single/Laser | Cone lock-on → homing laser burst |
| DAMUL | Wide   | Spread5/Beam | Full-screen lock-on → beam sweep |
| GUNEX | Explosive | Spread3+Missile | Charge bomb + homing missiles |

- Lock-on: hold Z → acquire targets → release → fire homing shots
- Charge: hold X (fire) → charge up → release for charged blast
- Bomb: press C → screen clear + invincibility (bombStock decrements)
- Small hitbox (4px radius center of ship)
- Score → Extra Life: every 100,000 points

### `bullet_pattern.h/cpp` — Bullet Hell pattern generator
- `firePattern(BulletPool, BulletPattern, origin, toward, baseAngle, speed, dmg, owner)`
- SINGLE, SPREAD_3/5, CIRCLE_8/16, SPIRAL_CW/CCW, AIMED, AIMED_SPREAD, CURTAIN, RANDOM_SPREAD, HOMING

### `enemy.h/cpp` — Enemy system (5 types)
- SMALL: straight down, SINGLE pattern
- MEDIUM: sine wave, SPREAD_3 pattern
- LARGE: tracks player, CIRCLE_8 pattern, drops power-up
- FAST: diagonal movement, AIMED pattern
- ARMORED: slow, high HP, CURTAIN pattern

### `boss.h/cpp` — Boss system (3 bosses, 3 phases each)
- Phase 1 (>66% HP): moderate attacks
- Phase 2 (>33% HP): faster, complex patterns (SPIRAL_CW + AIMED_SPREAD)
- Phase 3 (<33% HP): enraged, dense CIRCLE_16 + CURTAIN simultaneously
- Each boss has unique movement pattern and attack mix

### `collision.h/cpp` — Collision detection
- Bullet-Player: circular (4px player hitRadius) — Bullet Hell standard
- Bullet-Enemy/Boss: AABB
- Player-Enemy: AABB
- PowerUp-Player: AABB (generous hitbox)
- Triggers AudioManager SFX calls

### `stage.h/cpp` — Stage/wave manager
- 3 stages, each with 5–7 waves
- Wave entries: EnemyType, count, spawnInterval, pattern, entryDelay
- After all waves: short delay → boss spawns
- Stage 3 has denser waves + mixed enemy types

### `hud.h/cpp` — HUD with SDL2_ttf
- Score + Hi-Score, Stage indicator
- Life icons + bomb stock (sprite-based)
- Power level bar
- Boss HP bar during boss fights
- Stage Clear / Game Over / Victory overlays

### `menu.h/cpp` — Title + Ship Select
- Title: "GALAXY STORM" logo, START/QUIT options
- Ship Select: 3 ships with animated previews, stats, description text

### `sprites.h/cpp` — Sprite rendering
- Asset-based: loads SDL_Texture from AssetManager, draws with SDL_RenderCopy
- Fallback: SDL2 primitive shapes in neon palette if texture is nullptr
- Extras: lock-on reticle, bomb flash overlay

### `background.h/cpp` — Parallax starfield + nebula
- 3-layer star parallax
- Optional nebula texture scroll (background sprite)
- Color-tinted stars (white/cyan/blue/yellow variants)

### `powerup.h/cpp` — 6 power-up types
- SPREAD, LASER, MISSILE, SHIELD, BOMB (restore 1 bomb), POWER (power level +1)
- Bobbing animation

---

## Data Flow (one frame)

```
SDL_PollEvent → handleEvents()
                    │
            ┌───────▼─────────────┐
            │  state == TITLE      │ → updateMenu / renderMenu
            │  state == SHIP_SELECT│ → updateShipSelect / renderShipSelect
            └───────┬─────────────┘
                    │ state == PLAYING
            ┌───────▼────────────────────────────────────┐
            │ updateBackground(dt)                        │
            │ updatePlayer(dt, bullets, enemies)          │
            │ updateStage(dt, enemies, boss)              │
            │ updateEnemies(dt, player, bullets)          │
            │ updateBoss(dt, bullets, player)             │
            │ updateBullets(dt)                           │
            │ updatePowerUps(dt)                          │
            │ checkCollisions(audio)                      │
            └───────┬────────────────────────────────────┘
                    │
            ┌───────▼────────┐
            │   render()     │ bg → enemies → boss → powerups
            │                │   → player → bullets → HUD
            └────────────────┘
```

---

## Bullet Hell Design Notes

### Small Hitbox
- Player hitRadius = 4px (center of ship)
- Enemy bullet collision uses `circlesOverlap()` with this radius
- Visual ship sprite is ~28×36px (much larger than hitbox)

### Lock-on System
- Hold Z key: enter lock-on mode, reticles appear on enemies
- Targets acquired while Z held (up to 8 targets)
- Release Z: fire homing shots at all locked targets simultaneously
- Different ship types have different lock-on area (cone vs. full-screen)

### Charge Shot
- Hold X (fire): charge meter fills over 1.5 seconds
- Release when charged: fire powerful blast
- BAGON: charge laser. DAMUL: charge beam sweep. GUNEX: charge bomb

### Bomb
- C key: activate bomb (requires bombStock > 0)
- Clears all enemy bullets on screen
- 2-second invincibility
- Full-screen white flash effect

### Score → Extra Life
- 100,000 points = +1 life (displayed as 1UP flash on HUD)
- Hi-Score tracked this session

---

## Coding Conventions (Team-wide)

| Item | Style |
|------|-------|
| Namespace | `galaxy` (all code) |
| Structs / Classes | `PascalCase` |
| Functions / variables | `camelCase` |
| Constants | `UPPER_SNAKE_CASE` |
| Header guard | `#pragma once` |
| Asset loading | SDL2_image PNG; fallback to SDL2 primitives |
| Object pooling | Bullets (500), enemies (64), power-ups (16), stars (128) |
| Timestep | Fixed 60 FPS; `float dt` in seconds passed to every `update()` |
| Collision | Circular for player hitbox; AABB for everything else |
