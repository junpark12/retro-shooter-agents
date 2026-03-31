# QA Report — Galaxy Storm

**Review Date**: 2026-03-26 (v1) → 2026-05-01 (v2 — Full Re-review) → 2026-05-01 (v3 — Kenney Asset Integration Review)
**Reviewer**: @tester (QA Engineer Agent)  
**Build Status**: ✅ PASS (Linux / GCC 13.3 / SDL2 2.30 — all 14 source files compiled and linked)

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
| `game/src/bullet_pattern.h` / `bullet_pattern.cpp` | Bullet patterns |
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

## 발견 이슈

### 🔴 Critical — 즉시 수정 완료

#### [bullet_pattern.cpp] `#include <initializer_list>` 누락 → 빌드 불가
- **위치**: `bullet_pattern.cpp:27, 34, 75`
- **문제**: `patternSpread3`, `patternSpread5`, `patternAimedSpread` 함수에서 범위 기반 for 루프에 중괄호 초기화 목록(`{-20.0f, 0.0f, 20.0f}`)을 사용하나, `<initializer_list>` 헤더가 없어 GCC 13에서 컴파일 오류 발생.
  ```
  error: deducing from brace-enclosed initializer list requires '#include <initializer_list>'
  ```
- **수정**: `bullet_pattern.cpp` 상단에 `#include <initializer_list>` 추가.
- **심각도**: 🔴 Critical (빌드 불가)

#### [powerup.h] `#include "entity.h"` 누락 → 빌드 불가
- **위치**: `powerup.h:9`
- **문제**: `struct PowerUp : Entity` 선언에서 베이스 클래스 `Entity`가 정의된 `entity.h`를 포함하지 않고 `types.h`만 포함. `PowerUp`의 멤버(`active`, `pos`, `vel`, `bounds`, `hp`)를 전혀 사용할 수 없어 `powerup.cpp` 전체 컴파일 실패.
  ```
  error: expected class-name before '{' token
  error: 'struct galaxy::PowerUp' has no member named 'active'
  ```
- **수정**: `powerup.h`에서 `#include "types.h"` → `#include "entity.h"` 변경 (`entity.h`가 `types.h`를 포함하므로 포함 관계 유지).
- **심각도**: 🔴 Critical (빌드 불가)

---

### 🟠 Major — 수정 완료

#### [asset_manager.cpp] 에셋 경로 `"game/assets/..."` 하드코딩 → 런타임 에셋 로드 전체 실패
- **위치**: `asset_manager.cpp:12–35` (24개 `load()` 호출 전부)
- **문제**: 모든 스프라이트 로드 경로가 `"game/assets/sprites/..."` 형식으로 하드코딩. 그러나 `CMakeLists.txt`는 에셋을 빌드 디렉토리의 `assets/`로 복사(`$<TARGET_FILE_DIR>/assets`). 빌드 디렉토리에서 실행 시 `IMG_Load("game/assets/...")` 전부 실패 → 모든 스프라이트가 fallback 프리미티브로만 렌더링.
- **수정**: 모든 경로를 `"assets/sprites/..."` 형식으로 통일.
- **심각도**: 🟠 Major (에셋 전체 로드 실패)

#### [audio.cpp] 오디오 경로 `"game/assets/..."` 하드코딩 → BGM/SFX 전체 무음
- **위치**: `audio.cpp:14–34` (20개 `loadMusic`/`loadSound` 호출 전부)
- **문제**: 위와 동일 원인. `"game/assets/bgm/..."` / `"game/assets/sfx/..."` 경로가 빌드 디렉토리에서 존재하지 않아 모든 오디오 로드 실패.
- **수정**: 모든 경로를 `"assets/bgm/..."` / `"assets/sfx/..."` 형식으로 통일.
- **심각도**: 🟠 Major (오디오 전체 비활성)

#### [game.cpp:75] 폰트 경로 우선순위 역전
- **위치**: `game.cpp:75–78`
- **문제**: 첫 번째 시도가 `"game/assets/fonts/PressStart2P-Regular.ttf"`, fallback이 `"assets/fonts/PressStart2P-Regular.ttf"`. 빌드 디렉토리 실행 환경에서는 fallback이 실제로 동작하는 경로인데 우선순위가 낮아, 항상 첫 번째를 시도했다가 실패 후 fallback으로 동작하는 비효율 발생.
- **수정**: 순서 교체 — `"assets/fonts/..."` 우선, `"game/assets/fonts/..."` fallback.
- **심각도**: 🟠 Major (성능 낭비, 잘못된 경로 우선 적용)

#### [player.cpp:170] `updatePlayer()` 내 static 지역변수 — 게임 재시작 시 입력 상태 오염
- **위치**: `player.cpp:170–172` (수정 전)
- **문제**: `prevLockHeld`, `prevFireHeld`, `prevBombHeld`가 `static bool`으로 선언되어, `initPlayer()`가 호출되어도 이전 게임의 버튼 상태가 유지됨.
  - 특히 `bombHeld && !prevBombHeld` 조건에서: 이전 게임 종료 시 C키를 누른 채로 재시작하면 `prevBombHeld = true`가 유지되고, 첫 프레임에 C키를 떼면 폭탄이 `prevBombHeld=true → bombHeld=false` 조건 없이 발동 안 함. 반대로 이전 게임에서 C키를 누르지 않고 종료 후 재시작 첫 프레임에 C키를 누른 경우 `!prevBombHeld=true` → 폭탄 즉시 발동 (정상 동작). 실질적으로 게임 재시작 첫 프레임 lock-on 해제/차지 발사 오동작 가능성.
- **수정**:
  1. `player.h`의 `Player` 구조체에 `prevLockHeld`, `prevFireHeld`, `prevBombHeld` bool 멤버 추가 (기본값 `false`).
  2. `player.cpp`의 `static bool` 선언을 제거하고 `bool& prevLockHeld = p.prevLockHeld;` 등 멤버 참조로 교체.
  3. `initPlayer()`에서 `p = {}` 초기화 시 자동으로 `false`로 리셋.
- **심각도**: 🟠 Major (게임 재시작 시 입력 논리 오작동)

---

### 🟡 Minor — 미수정 (추후 개선 권고)

| # | 위치 | 설명 |
|---|------|------|
| 1 | `stage.cpp:83–89` | 신규 스폰 적에 wave 패턴 적용 시 `moveTimer <= 0.0001f` 조건으로 탐색. 같은 프레임에 `moveTimer`가 0에 가까운 기존 적이 있으면 잘못된 적에게 패턴 할당 가능. `spawnEnemy()` 반환값을 Enemy*로 변경하면 안전. |
| 2 | `sprites.cpp:295` | `renderExplosion()` 파티클 반투명 렌더링 시 `SDL_BLENDMODE_BLEND`를 설정하지만 복귀 시 `SDL_BLENDMODE_NONE`으로만 리셋. 다른 렌더러 상태와 상호작용 위험. |
| 3 | `hud.cpp` | `renderBossHP()`에서 깜빡임에 `SDL_GetTicks()` 직접 사용 — 외부에서 dt를 받아 `timer` 기반 제어로 교체 권고. |
| 4 | `game.cpp:260–263` | 1UP 체크 루프가 1프레임에 1회만 실행되어, 점수가 두 threshold를 한 번에 넘어도 1개만 지급. 점수 점프가 200k를 초과하는 보스 킬(최대 30,000점) 상황에서는 실질적 문제 없음. |
| 5 | `game.cpp:38–39` | `~Game()` 기본 소멸자로 정의 → 비정상 종료 시 `shutdown()` 미호출. `delete`/`unique_ptr` RAII 패턴 권장. |
| 6 | `asset_manager.cpp` / `game.cpp` | `IMG_Init()`이 `game.cpp::init()`에서 1회, `AssetManager::init()`에서 1회 총 2회 호출됨. SDL2 내부 참조 카운터로 안전하나, `game.cpp`의 `IMG_Init` 호출을 제거하거나 AssetManager에 위임하도록 정리 권장. |
| 7 | `powerup.h` | `#include "asset_manager.h"`가 헤더에 포함되어 있으나, `PowerUp` 구조체 정의에는 불필요. `renderPowerUps` 함수 시그니처에만 필요하므로 전방 선언으로 교체 가능. |

---

## 수정 사항 요약

| 파일 | 수정 내용 | 심각도 |
|------|-----------|--------|
| `src/bullet_pattern.cpp` | `#include <initializer_list>` 추가 | 🔴 Critical |
| `src/powerup.h` | `#include "types.h"` → `#include "entity.h"` 교체 | 🔴 Critical |
| `src/asset_manager.cpp` | 24개 에셋 경로 `"game/assets/..."` → `"assets/..."` | 🟠 Major |
| `src/audio.cpp` | 20개 오디오 경로 `"game/assets/..."` → `"assets/..."` | 🟠 Major |
| `src/game.cpp` | 폰트 경로 우선순위 수정 (build-relative 우선) | 🟠 Major |
| `src/player.h` | `prevLockHeld`, `prevFireHeld`, `prevBombHeld` 멤버 추가 | 🟠 Major |
| `src/player.cpp` | `static bool` 제거 → Player 멤버 참조 사용 | 🟠 Major |

---

## 헤더-구현 일관성 검증 결과

| 헤더 | 선언 함수 | 구현 확인 |
|------|-----------|-----------|
| `player.h` | `initPlayer`, `updatePlayer`, `releaseLockOn`, `activateBomb`, `renderPlayer` | ✅ 전부 구현 |
| `enemy.h` | `spawnEnemy`, `updateEnemies`, `renderEnemies`, `allEnemiesDefeated` | ✅ 전부 구현 |
| `bullet.h` | `fireBullet`, `fireHomingBullet`, `updateBullets`, `renderBullets`, `clearEnemyBullets` | ✅ 전부 구현 |
| `bullet_pattern.h` | 12개 pattern 함수 + `firePattern` dispatch | ✅ 전부 구현 |
| `boss.h` | `initBoss`, `updateBoss`, `renderBoss` | ✅ 전부 구현 |
| `collision.h` | 5개 충돌 함수 | ✅ 전부 구현 |
| `stage.h` | `initStage`, `updateStage` | ✅ 전부 구현 |
| `powerup.h` | `spawnPowerUp`, `updatePowerUps`, `renderPowerUps` | ✅ 전부 구현 |
| `asset_manager.h` | `init`, `get`, `load`, `shutdown` | ✅ 전부 구현 |
| `audio.h` | `init`, `playBGM`, `stopBGM`, `playSFX`, `loadMusic`, `loadSound`, `setBGMVolume`, `setSFXVolume`, `shutdown` | ✅ 전부 구현 |
| `sprites.h` | `renderPlayerSprite`, `renderEnemySprite`, `renderBossSprite`, `renderBulletSprite`, `renderPowerUpSprite`, `renderExplosion`, `renderLockOnReticle`, `renderBombFlash`, `renderShipPreview` + 5개 primitive | ✅ 전부 구현 |
| `hud.h` | `renderHUD`, `renderBossHP`, `renderStageClear`, `renderGameOver`, `renderVictory`, `renderText` | ✅ 전부 구현 |
| `menu.h` | `updateMenu`, `handleMenuEvent`, `renderMenu`, `updateShipSelect`, `handleShipSelectEvent`, `renderShipSelect` | ✅ 전부 구현 |
| `background.h` | `initBackground`, `updateBackground`, `renderBackground` | ✅ 전부 구현 |
| `game.h` | `Game::init`, `run`, `shutdown`, `handleEvents`, `update`, `render`, `startStage`, `onStageClear`, `onGameOver` | ✅ 전부 구현 |

---

## 빌드 검증

```bash
# 환경: Ubuntu 24.04 / GCC 13.3 / SDL2 2.30.0 / SDL2_image 2.8.2 / SDL2_mixer 2.8.0 / SDL2_ttf 2.22.0
cd game && mkdir build_test && cd build_test
cmake .. -DCMAKE_BUILD_TYPE=Debug   # → Generating done
make -j4                             # → [100%] Built target GalaxyStorm ✅
```

| 검사 항목 | 결과 |
|-----------|------|
| C++17 표준 설정 | ✅ |
| 모든 소스 파일 CMake 포함 | ✅ (14개) |
| SDL2 modern/legacy 조건부 링크 | ✅ |
| SDL2_image / SDL2_mixer / SDL2_ttf 링크 | ✅ |
| `WIN32` 실행 파일 플래그 | ✅ |
| 에셋 빌드 디렉토리 복사 규칙 | ✅ |
| 컴파일 오류 없음 (수정 후) | ✅ |
| 컴파일 경고 없음 | ✅ |
| 링크 성공 | ✅ |

### Windows 빌드 가이드

```powershell
# vcpkg로 SDL2 라이브러리 설치
vcpkg install sdl2 sdl2-image sdl2-mixer sdl2-ttf

# CMake 구성 (Visual Studio 2022 기준)
cmake -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
      -DVCPKG_TARGET_TRIPLET=x64-windows

cmake --build build --config Release
# 빌드 결과: build/Release/GalaxyStorm.exe
# SDL2 DLL은 POST_BUILD 규칙으로 자동 복사됨
```

---

## 게임 로직 검증 결과

| 항목 | 검사 내용 | 결과 |
|------|-----------|------|
| nullptr 역참조 | `audio_`, `assets_`, `font_` 호출 전 null 체크 | ✅ 안전 |
| BulletPool 오버플로 | 풀이 꽉 찬 경우 `fireBullet()` silently ignored | ✅ 안전 |
| EnemyPool 오버플로 | `spawnEnemy()` 비활성 슬롯 없으면 skip | ✅ 안전 |
| SDL 자원 이중 해제 | `shutdown()`에서 nullptr 체크 후 delete | ✅ 안전 |
| AABB 충돌 경계조건 | `rectsOverlap()` `<` / `>` 조건 정확 | ✅ 정확 |
| 원형 충돌 (플레이어 히트박스) | `circlesOverlap()` dist² < radSum² | ✅ 정확 |
| waveIndex 경계 초과 | `waveIndex < waveCount` 가드로 방지 | ✅ 안전 |
| Boss phase 전환 | `hp <= maxHp/3` → phase3, `hp <= maxHp*2/3` → phase2 | ✅ 정확 |
| delta-time 적용 | 모든 `pos += vel * dt` / timer에 dt 적용 | ✅ 일관성 |
| 화면 경계 체크 | player: `std::clamp`, bullets: 오프스크린 비활성화 | ✅ 정상 |
| SHIP_SELECT → PLAYING | `shipSelect_` 항상 `init()`에서 할당됨 | ✅ 안전 |

---

## 에셋 검증 결과

| 에셋 카테고리 | 파일 수 | 파일 존재 여부 | 코드 경로 일치 |
|---------------|---------|----------------|----------------|
| 플레이어 스프라이트 (`player/`) | 3 PNG | ✅ | ✅ (수정 후) |
| 적 스프라이트 (`enemies/`) | 5 PNG | ✅ | ✅ (수정 후) |
| 보스 스프라이트 (`bosses/`) | 3 PNG | ✅ | ✅ (수정 후) |
| 총알 스프라이트 (`bullets/`) | 3 PNG | ✅ | ✅ (수정 후) |
| 효과 스프라이트 (`effects/`) | 4 PNG | ✅ | ✅ (수정 후) |
| 파워업 스프라이트 (`powerups/`) | 6 PNG | ✅ | ✅ (수정 후) |
| 배경 (`backgrounds/`) | 1 PNG | ✅ | ✅ (수정 후) |
| UI (`ui/`) | 3 PNG | ✅ | ✅ (수정 후) |
| BGM (`bgm/`) | 7 OGG | ✅ | ✅ (수정 후) |
| SFX (`sfx/`) | 13 WAV | ✅ | ✅ (수정 후) |
| 폰트 (`fonts/`) | 1 TTF | ✅ | ✅ (수정 후) |
| CREDITS.md | 1 | ✅ | N/A |
| 에셋 누락 시 fallback | — | ✅ 프리미티브 렌더링 | ✅ |

---

---

## v3 — Kenney Asset Integration Review (2026-05-01)

### 검토 파일 (변경 대상)

| 파일 | 변경 내용 |
|------|-----------|
| `game/src/asset_manager.h` | 스프라이트 키 상수 추가 (Kenney 에셋) |
| `game/src/asset_manager.cpp` | 로드 경로 Kenney 에셋으로 전면 교체 |
| `game/src/audio.h` | `SFX_SHIELD_UP`, `SFX_SHIELD_DOWN`, `SFX_LASER_ALT`, `SFX_ZAP` 상수 추가 |
| `game/src/audio.cpp` | 4개 신규 SFX 파일 로드 추가 |
| `game/src/sprites.h` | `renderEngineExhaust` 함수 선언 추가 |
| `game/src/sprites.cpp` | Kenney 스프라이트에 맞춘 렌더 크기 조정, `renderEngineExhaust` 구현 추가 |

### 신규 에셋 파일

| 카테고리 | 파일 수 | 예시 |
|----------|---------|------|
| 플레이어 스프라이트 | 6 PNG | `ship_bagon_k.png`, `ship_damage1-3.png` |
| 적 스프라이트 | 7 PNG | `enemy_small_k.png` ~ `enemy_ufo_red.png` |
| 총알/레이저 | 4 PNG | `laser_player.png`, `laser_player_wide.png` |
| 엔진 화염 프레임 | 8 PNG | `engine_fire00-07.png` |
| 메테오 | 4 PNG | `meteor_brown_big.png` 등 |
| 파워업 | 6 PNG | `powerup_spread_k.png` ~ `powerup_power_k.png` |
| 신규 SFX | 4 OGG | `sfx_shield_up.ogg`, `sfx_shield_down.ogg`, `sfx_laser_alt.ogg`, `sfx_zap.ogg` |

---

### 🔴 Critical — 없음

신규 코드에서 크래시 또는 빌드 불가 이슈 없음.

---

### 🟠 Major — 없음

로직 오류, 메모리 릭, 잘못된 동작 없음.

---

### 🟡 Minor — 없음

코드 스타일·최적화 수준의 개선 기회도 발견되지 않음.

---

### ✅ 검증 항목 전체 통과

#### 1. 컴파일 안전성

| 항목 | 결과 |
|------|------|
| `#pragma once` in asset_manager.h, audio.h, sprites.h | ✅ |
| `#include <array>` in sprites.cpp (std::array 사용) | ✅ 4번 줄에 존재 |
| 함수 선언(sprites.h:59-60) ↔ 구현(sprites.cpp:362-386) 시그니처 일치 | ✅ |
| 신규 SFX 상수 선언(audio.h:82-85) ↔ 사용(audio.cpp:36-39) 일치 | ✅ |
| 신규 스프라이트 키 상수(asset_manager.h:76-83) ↔ 로드 경로(asset_manager.cpp:47-54) 일치 | ✅ |
| 타입 캐스팅 안전성 (`static_cast` 사용, narrowing 없음) | ✅ |

#### 2. 에셋 파일 존재 여부

모든 에셋에 대해 `[ -f path ]` 로 디스크 존재 확인 — **41개 파일 전원 존재**.

| 카테고리 | 확인 |
|----------|------|
| 플레이어 6 PNG | ✅ |
| 적 7 PNG | ✅ |
| 보스 3 PNG | ✅ |
| 총알 4 PNG | ✅ |
| 폭발 4 PNG | ✅ |
| 엔진화염 8 PNG | ✅ |
| 메테오 4 PNG | ✅ |
| 파워업 6 PNG | ✅ |
| 배경·UI 4 PNG | ✅ |
| 신규 SFX 4 OGG | ✅ |

#### 3. 파일 포맷 무결성

- **PNG 매직 바이트** (`89 50 4E 47`): 신규 Kenney 스프라이트 전원 통과 ✅
- **OGG 매직 바이트** (`4F 67 67 53`): `sfx_shield_up/down.ogg`, `sfx_laser_alt.ogg`, `sfx_zap.ogg` 전원 통과 ✅
- **IEND 마커** (PNG 완전성): `engine_fire00-07.png` 8프레임 전원 IEND 존재 — 파일 잘림(truncation) 없음 ✅

#### 4. OGG via `Mix_LoadWAV` 호환성

SDL2_mixer의 `Mix_LoadWAV`는 내부적으로 `SDL_RWops`를 통해 포맷을 자동 감지하며, OGG 파일도 SDL2_mixer의 OGG 지원이 활성화된 경우 (표준 패키지 기본값) 정상 디코드됨. `.ogg` 확장자를 `.wav` 로더로 여는 것은 완전히 유효한 SDL2_mixer 관용 패턴. ✅

#### 5. `renderEngineExhaust` 배열 경계 분석

```cpp
static constexpr std::array<const char*, 8> fireKeys = { /* 8 elements, index 0–7 */ };
const int idx = std::clamp(fireFrame % 8, 0, 7);
SDL_Texture* tex = assets.get(fireKeys[idx]);  // idx는 항상 [0, 7]
```

- `fireFrame`이 양수: `% 8` → `[0, 7]`, clamp 유지 ✅
- `fireFrame`이 음수 (예: -1): C++11 나머지 연산 결과 `-1`, `std::clamp(-1, 0, 7) = 0` → 인덱스 0으로 폴백 ✅
- **배열 오버플로우 불가능**. 이중 방어(`% 8` + `clamp`)로 안전성 이중 보장.

#### 6. 스프라이트 렌더 크기 적합성

| 스프라이트 | 원본 해상도 | 렌더 크기 | 판정 |
|-----------|-------------|-----------|------|
| `ship_bagon_k.png` | 99×75 | 48×48 | ✅ 적절한 다운스케일 |
| `enemy_small_k.png` | 93×84 | 24×24 | ✅ 적절한 다운스케일 |
| `laser_player.png` | 9×54 | 9×36 | ✅ 너비 일치, 세로 약간 축소 (SDL 스케일) |
| `engine_fire00.png` | 16×40 | 20×24 | ✅ 의도적 크기 조정, fallback 프리미티브도 동일 영역 |
| `powerup_spread_k.png` | 34×33 | 24×24 | ✅ 적절한 다운스케일 |

모든 크기가 480×640 게임 화면 기준으로 합리적임. SDL2의 `SDL_RenderCopy`는 dst rect로 자동 스케일.

#### 7. `renderEngineExhaust` 위치 로직

- `fy = shipY + 36`: 플레이어 선박 렌더 높이 48px 기준, 선박 하단에서 12px 위에서 시작 → 시각적 오버랩으로 화염이 선박 아래쪽에서 분출되는 자연스러운 연출. ✅
- `fx = shipX + shipW/2 - fw/2`: 선박 폭 중앙에 화염 정렬. ✅

#### 8. CMakeLists.txt — 신규 파일 포함 확인

`sprites.cpp`, `asset_manager.cpp`, `audio.cpp` 모두 기존 SOURCES 목록에 포함됨. 신규 `.cpp` 파일 없으므로 CMake 변경 불필요. ✅

---

### v3 수정 사항

**없음** — 모든 검사 항목 통과. 코드 수정 불필요.

---

## 전체 품질 평가

**v3 점수: 9.0 / 10** (v2 대비 +0.5 — Kenney 통합으로 에셋 완전성 향상)

### 강점
- 오브젝트 풀 패턴(`BulletPool`, `EnemyPool`, `PowerUpPool`)으로 런타임 동적 할당 없음
- 헤더-구현 분리 철저, 전방 선언으로 순환 의존성 없음
- 모든 헤더에 `#pragma once` 적용
- `namespace galaxy` 일관 적용
- delta-time 기반 물리가 모든 시스템에 일관되게 적용
- Fixed-rate accumulator 루프 (60 FPS) 올바르게 구현
- `shutdown()` 자원 해제 순서 올바름 (audio → assets → SDL)
- 충돌 판정: 총알-플레이어는 원형(Bullet Hell 소형 히트박스), 나머지는 AABB로 목적에 맞게 분리
- Boss phase 3 Stage 3에서 12가지 패턴을 동시 발사하는 하드코어 Bullet Hell 연출 의도적 구현
- 에셋 로드 실패 시 SDL2 프리미티브 fallback 렌더링으로 무결성 보장

### 개선 제안
1. `spawnEnemy()` 반환값을 `Enemy*`로 변경하여 wave 패턴 직접 할당
2. `~Game()` 소멸자에서 `shutdown()` 호출 또는 `unique_ptr` 전환
3. `IMG_Init()` 이중 호출 정리 (AssetManager에 위임 일원화)
4. `powerup.h`에서 `AssetManager` 전방 선언으로 헤더 의존성 경량화
5. 폭발 파티클 alpha blending 버그 확인 필요 (`SDL_BLENDMODE_BLEND` 복귀 일관성)

