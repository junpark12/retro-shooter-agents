# QA Report — Galaxy Storm

**Review Date**: 2026-03-26  
**Reviewer**: @tester (QA Engineer Agent)  
**Build Status**: ✅ PASS (Linux/SDL2 2.0.x — all 13 source files compiled and linked)

---

## Files Reviewed

| File | Type |
|------|------|
| `game/src/types.h` | Header |
| `game/src/entity.h` | Header |
| `game/src/game.h` / `game.cpp` | Game loop |
| `game/src/player.h` / `player.cpp` | Player |
| `game/src/enemy.h` / `enemy.cpp` | Enemy |
| `game/src/bullet.h` / `bullet.cpp` | Bullets |
| `game/src/boss.h` / `boss.cpp` | Boss |
| `game/src/collision.h` / `collision.cpp` | Collision |
| `game/src/stage.h` / `stage.cpp` | Stage/wave |
| `game/src/powerup.h` / `powerup.cpp` | Power-ups |
| `game/src/sprites.h` / `sprites.cpp` | Sprite rendering |
| `game/src/hud.h` / `hud.cpp` | HUD |
| `game/src/menu.h` / `menu.cpp` | Menu |
| `game/src/background.h` / `background.cpp` | Starfield |
| `game/src/main.cpp` | Entry point |
| `game/CMakeLists.txt` | Build system |

---

## Issues Found & Fixed

### 🔴 Critical — Fixed

#### [CMakeLists.txt] SDL2 imported target incompatibility
- **Problem**: `target_link_libraries(... SDL2::SDL2 SDL2::SDL2main)` fails on SDL2 < 2.0.12 (e.g., Ubuntu 20.04) where the legacy `FindSDL2.cmake` is used and imported targets don't exist.
- **Fix**: Added modern/legacy conditional branch:
  ```cmake
  if(TARGET SDL2::SDL2)
      target_link_libraries(...SDL2::SDL2...)
  else()
      target_include_directories(...${SDL2_INCLUDE_DIRS})
      target_link_libraries(...${SDL2_LIBRARIES})
  endif()
  ```
  Also guarded the Windows DLL copy post-build command with `AND TARGET SDL2::SDL2`.

---

### 🟠 Major — Fixed

#### [menu.cpp] `SDL_Quit()` + `std::exit(0)` bypasses proper cleanup
- **Problem**: QUIT menu selection called `SDL_Quit()` and `std::exit(0)` directly, skipping `SDL_DestroyRenderer`, `SDL_DestroyWindow`, and all C++ destructors.
- **Fix**: Added `GameState::QUIT` to the enum in `types.h`; `handleMenuEvent()` now returns `GameState::QUIT`; `game.cpp::handleEvents()` responds by setting `running_ = false`, letting the main loop exit cleanly and `Game::shutdown()` run normally.

#### [collision.cpp] Laser piercing did not work
- **Problem**: An unconditional `break` at the end of the inner enemy-loop in `checkBulletEnemyCollision()` meant only one enemy could be hit per frame, regardless of weapon type.
- **Fix**: `break` is now conditional — executed only for non-laser bullets; laser bullets continue iterating through all enemies in the pool.

---

### 🟡 Minor — Not Fixed (Tracked)

| # | Location | Description |
|---|----------|-------------|
| 1 | `sprites.cpp` | `renderExplosion` alpha value ignored — `SDL_BLENDMODE_NONE` is default; blend mode not set before drawing semi-transparent particles |
| 2 | `hud.cpp`, `menu.cpp` | 200+ lines of pixel-font rendering code duplicated in both files (DRY violation) — recommend extracting to `font_utils.h/.cpp` |
| 3 | `player.cpp` | SPREAD is also the default weapon; power-up pickup adds shot count but no visible difference — recommend differentiating base weapon |
| 4 | `stage.cpp` | `allEnemiesDefeated()` iterates all MAX_ENEMIES (64) every frame — could be O(1) with a kill counter check |
| 5 | `game.cpp` | `~Game()` uses default destructor; `shutdown()` not called on abnormal exit — recommend `unique_ptr` for subsystems |

---

## Build Verification Results

| Check | Result |
|-------|--------|
| C++17 standard set | ✅ |
| All 13 source files included in CMake | ✅ |
| SDL2 modern target (≥ 2.0.12) | ✅ |
| SDL2 legacy variable fallback (< 2.0.12) | ✅ (after fix) |
| `WIN32` executable flag | ✅ |
| Compiled without errors | ✅ |
| Compiled without warnings | ✅ |
| Linked successfully | ✅ |

---

## Overall Assessment

**Score: 8.0 / 10**

**Strengths:**
- Object pool pattern applied consistently (BulletPool 256, EnemyPool 64, PowerUpPool 16) — no heap allocation at runtime
- Clear header/implementation separation with forward declarations eliminating circular dependencies
- `#pragma once` on all headers
- AABB `rectsOverlap()` with correct boundary conditions
- Delta-time applied consistently to all movement and timers
- Fixed-rate accumulator loop implemented correctly
- SDL resource teardown order in `shutdown()` is correct (after the QUIT fix)
- `namespace galaxy` applied consistently across all sources

**Areas for Future Improvement:**
- Extract shared pixel-font code to avoid duplication between HUD and menu
- Add alpha blending for explosion particles
- Differentiate default weapon from SPREAD power-up
- Migrate subsystem ownership to `unique_ptr` for RAII safety
