# QA Report — Galaxy Storm (Improvements Iteration)

## 검토 범위

### 검토 파일 목록
| 파일 | 역할 |
|------|------|
| `game/src/game.cpp` | pause 시스템, 스크린쉐이크, 보스경고 SFX |
| `game/src/player.cpp` | 차지 시간 1.0초, 그레이즈 타이머 |
| `game/src/boss.cpp` | 공격 예고, 입장 애니메이션, 스테이지5 조정 |
| `game/src/stage.cpp` | 포메이션 스폰, 웨이브 디자인, 경고 플래그 |
| `game/src/collision.cpp` | 그레이즈 시스템, 넉백, 콤보, 파워업SFX |
| `game/src/audio.cpp` | 새 SFX 로딩 |
| `game/src/hud.cpp` | 차지바, 파워업타이머, 일시정지, 보스HP보간 |
| `game/src/particles.cpp` | 머즐플래시, 히트스파크, 그레이즈스파크 |
| `game/src/sprites.cpp` | 적 HP바 (`renderEnemyHPBar`) |
| `game/src/enemy.cpp` | ARMORED 적 HP바 렌더 |
| `game/src/enemy.h` | Enemy 구조체 |
| `game/src/entity.h` | Entity 기반 구조체 |
| `game/src/player.h` | Player 구조체 + 함수 선언 |
| `game/src/collision.h` | 충돌 함수 선언 |
| `game/src/hud.h` | HUD 함수 선언 |
| `game/src/stage.h` | Stage 구조체 |
| `game/src/audio.h` | AudioManager + SFX 상수 |
| `game/src/types.h` | 공용 타입, 열거형, 상수 |
| `game/CMakeLists.txt` | 빌드 시스템 |

### 검토 일자
2025-07-04

---

## 발견 이슈

### 🔴 Critical

#### [enemy.h / enemy.cpp:135,137] `Enemy` 구조체에 `maxHp` 필드 누락 → 컴파일 오류

**증상**: 빌드 시 아래 컴파일 오류 발생:
```
enemy.cpp:135:54: error: 'const struct galaxy::Enemy' has no member named 'maxHp'
enemy.cpp:137:42: error: 'const struct galaxy::Enemy' has no member named 'maxHp'
```

**원인**: `renderEnemies`에서 ARMORED 적의 HP바를 표시하기 위해 `e.maxHp`를 참조하는데,
`Enemy` 구조체(`enemy.h`)에도 기반 `Entity` 구조체(`entity.h`)에도 `maxHp` 필드가 없었음.

```cpp
// enemy.cpp:135 — 기존 코드 (빌드 불가)
if (e.type == EnemyType::ARMORED && e.hp < e.maxHp) {   // ❌ maxHp 미정의
    renderEnemyHPBar(renderer, ..., e.hp, e.maxHp);      // ❌ maxHp 미정의
}
```

**수정 내용**:
1. `enemy.h` — `Enemy` 구조체에 `int maxHp = 1;` 필드 추가
2. `enemy.cpp` — `setupEnemyStats()`의 각 `EnemyType` 분기에서 `e.maxHp = e.hp;` 설정 (스폰 시 hp와 동기화)

```cpp
// enemy.h — 수정 후
struct Enemy : Entity {
    ...
    int maxHp = 1;  // ✅ 추가: 스폰 시 설정된 최대 HP (HP바 비율 계산용)
};

// enemy.cpp — 수정 후 (ARMORED 예시)
case EnemyType::ARMORED:
    e.bounds = {0.0f, 0.0f, 36.0f, 32.0f};
    e.hp    = 10;
    e.maxHp = 10;  // ✅ 추가
    e.pointValue = 700;
    e.vel = {0.0f, 75.0f};
    break;
```

**빌드 결과**: 수정 후 `[100%] Built target GalaxyStorm` — 에러 없이 빌드 성공 ✅

---

### 🟠 Major

_(이 이터레이션에서 Critical 외의 Major 이슈 없음)_

---

### 🟡 Minor

#### [stage.cpp:3] 불필요한 `#include "audio.h"`

**설명**: `stage.cpp`에 `#include "audio.h"`가 포함되어 있으나,
`initStage()` / `updateStage()` 어디에도 `AudioManager`가 사용되지 않음.
보스 경고 SFX는 `game.cpp`에서 `stage_->bossWarningJustStarted` 플래그를 체크해 재생하도록 올바르게 분리되어 있음.

**영향**: 컴파일 에러는 아니나 불필요한 의존성으로 빌드 시간 소폭 증가,
`AudioManager` 변경 시 불필요하게 stage.cpp 재컴파일 유발.

**수정 내용**: `stage.cpp`에서 해당 include 제거.

```cpp
// 수정 전
#include "stage.h"
#include "audio.h"   // ← 불필요한 include
#include "boss.h"
#include "enemy.h"

// 수정 후
#include "stage.h"
#include "boss.h"
#include "enemy.h"
```

---

#### [enemy.cpp:136] ARMORED 적 HP바 width 파라미터 불일치

**설명**: `renderEnemyHPBar` 호출 시 spriteW를 하드코딩 값 `40`으로 전달하지만,
ARMORED 적의 실제 스프라이트 bounding box width는 `36.0f` (`setupEnemyStats` 참조).

```cpp
renderEnemyHPBar(renderer, x, y,
                 40,       // ← 40px, but ARMORED bounds.w = 36px
                 e.hp, e.maxHp);
```

**영향**: HP바가 스프라이트보다 4px 더 넓게 표시됨. 기능 오류는 아니나 시각적 불일치.

**수정 제안**:
```cpp
renderEnemyHPBar(renderer, x, y,
                 static_cast<int>(e.bounds.w),  // 36px — 스프라이트 크기와 일치
                 e.hp, e.maxHp);
```

---

#### [game.cpp:453,469] `CHARGE_TIME` 리터럴 중복 정의

**설명**: `player.cpp` 내부에 `constexpr float CHARGE_TIME = 1.0f;`가 정의되어 있으나,
`game.cpp` render() 함수에서 같은 값을 매직 넘버로 인라인 재정의.

```cpp
// game.cpp:453,469 (PLAYING/PAUSED 분기 각각)
const float CHARGE_TIME = 1.0f;  // 로컬 복사 — player.cpp 상수와 독립
renderChargeBar(..., std::clamp(player_->chargeTimer / CHARGE_TIME, 0.0f, 1.0f));
```

**영향**: 차지 시간 변경 시 양쪽을 모두 수정해야 하는 유지보수 위험.

**수정 제안**: `CHARGE_TIME`을 `types.h` 또는 `player.h` 헤더에 공용 `constexpr`로 선언하거나,
game.cpp가 player.h에서 공유 상수를 참조하도록 구조 개선.

---

### 🔵 Info

#### [stage.cpp:199] `bossWarningTimer = 2.5f` 초기화 후 즉시 덮어씀

경고 시작 시 `bossWarningTimer = 2.5f`로 초기화하지만, 같은 프레임 내 바로 다음 블록에서
`bossWarningTimer = s.bossDelay`로 덮어씀. 초기값 할당이 실질적으로 무의미.
기능적 오류는 없으나 코드 가독성을 낮춤. 초기화 블록을 합치거나 주석으로 의도를 명확히 표기 권장.

#### [boss.cpp:73] `displayHp` lerp 후 `std::max` 적용 — 의도적 동작 확인

```cpp
b.displayHp += (static_cast<float>(b.hp) - b.displayHp) * lerpRate;
b.displayHp = std::max(b.displayHp, static_cast<float>(b.hp));
```

`std::max`는 HP가 올라가는 경우(회복) displayHp를 즉시 동기화하고,
HP가 줄어드는 경우(피격) displayHp가 smooth drain 되도록 허용하는 의도적 설계.
버그 아님. 참고 기록.

---

## 수정 사항 요약

| 파일 | 수정 유형 | 설명 |
|------|-----------|------|
| `game/src/enemy.h` | 🔴 Critical 버그 수정 | `Enemy` 구조체에 `int maxHp = 1;` 필드 추가 |
| `game/src/enemy.cpp` | 🔴 Critical 버그 수정 | `setupEnemyStats()` 5개 분기 각각에 `e.maxHp = e.hp;` 설정 추가 |
| `game/src/stage.cpp` | 🟡 Minor 개선 | 불필요한 `#include "audio.h"` 제거 |

---

## 검사 항목 결과

| 항목 | 결과 | 비고 |
|------|------|------|
| 컴파일 안전성 | ❌→✅ | `e.maxHp` 미정의 Critical 오류 발견 및 수정 |
| 함수 시그니처 일관성 (`checkBulletPlayerCollision`) | ✅ | `collision.h`와 `collision.cpp` 완전 일치 |
| 함수 시그니처 일관성 (`updatePlayer`) | ✅ | `player.h`와 `player.cpp` 완전 일치 |
| ParticleSystem* null 체크 | ✅ | 모든 `ps` / `audio` 파라미터 사용 전 null 검사 |
| `GameState::PAUSED` handleEvents | ✅ | P/Escape 키로 토글, SFX_PAUSE_IN/OUT 재생 |
| `GameState::PAUSED` update | ✅ | 조기 return으로 게임 로직 정지 |
| `GameState::PAUSED` render | ✅ | HUD + renderPaused 오버레이 표시 |
| `renderBossHP` 시그니처 일치 | ✅ | `hud.h`와 `hud.cpp` 완전 일치 |
| `bossWarningJustStarted` 필드 (stage.h) | ✅ | `stage.h:38`에 선언됨 |
| FormationType include (stage.cpp) | ✅ | stage.h→types.h 체인으로 정의 포함 |
| ARMORED 적 maxHp 설정 | ❌→✅ | 필드 추가 및 setupEnemyStats에서 초기화로 수정 |
| CMakeLists.txt 완전성 | ✅ | 모든 소스 파일 포함, SDL2 4종 find_package 정상 |
| C++17 표준 설정 | ✅ | `CMAKE_CXX_STANDARD 17` 설정됨 |
| WIN32_EXECUTABLE 설정 | ✅ | `add_executable(...WIN32...)` 설정됨 |

---

## 빌드 검증

### CMake 설정 확인 결과

| 항목 | 상태 |
|------|------|
| `cmake_minimum_required(VERSION 3.16)` | ✅ |
| `CMAKE_CXX_STANDARD 17` | ✅ |
| `find_package(SDL2 REQUIRED)` | ✅ |
| `find_package(SDL2_image REQUIRED)` | ✅ |
| `find_package(SDL2_mixer REQUIRED)` | ✅ |
| `find_package(SDL2_ttf REQUIRED)` | ✅ |
| 소스 파일 누락 여부 | ✅ (14개 .cpp 모두 포함) |
| `WIN32` 빌드 타겟 | ✅ (`add_executable(${PROJECT_NAME} WIN32 ...)`) |
| Windows DLL 복사 커맨드 | ✅ (POST_BUILD copy_if_different) |
| assets 디렉토리 복사 | ✅ (POST_BUILD copy_directory) |

### 빌드 결과

```
[100%] Built target GalaxyStorm   ← 수정 후 에러/경고 없이 성공
```

### Windows 빌드 가이드

```bash
# 1. SDL2 개발 패키지 설치 (vcpkg 권장)
vcpkg install sdl2 sdl2-image sdl2-mixer sdl2-ttf

# 2. CMake 구성
cmake -B build -S game/ -DCMAKE_TOOLCHAIN_FILE=[vcpkg_root]/scripts/buildsystems/vcpkg.cmake

# 3. 빌드
cmake --build build --config Release

# 4. 실행 (assets/ 폴더가 실행 파일과 같은 위치에 복사됨)
build/Release/GalaxyStorm.exe
```

---

## 전체 품질 평가

- **점수: 8.5 / 10**

### 종합 의견

이번 이터레이션에서 추가된 기능들(pause 시스템, 스크린쉐이크, 그레이즈, ARMORED HP바, 보스 입장 애니메이션, 포메이션 스폰 등)은 설계 수준이 높고 코드 구조가 일관적으로 유지되고 있음.

**긍정적 평가:**
- 함수 시그니처가 헤더와 구현 간 완전히 일치
- `ParticleSystem*` / `AudioManager*` 파라미터 전체에 null 체크 적용
- `GameState::PAUSED` 처리가 handleEvents / update / render 세 곳에서 모두 올바르게 구현됨
- 보스 경고 SFX를 `bossWarningJustStarted` 원샷 플래그로 처리하는 구조가 깔끔함
- CMake 빌드 시스템이 modern imported-target 방식과 레거시 변수 방식을 모두 지원하는 호환 구조

**발견된 주요 문제:**
- **Critical 1건**: `Enemy::maxHp` 필드 누락으로 빌드 자체 불가 상태. ARMORED 적 HP바 기능이 헤더 정의 없이 구현 파일에만 작성됨. 수정 완료.

**권고 사항:**
- 공유 게임 상수(`CHARGE_TIME` 등)는 `types.h` 또는 별도 `constants.h`로 중앙 집중화
- `Enemy` 구조체처럼 신규 필드 추가 시 헤더 → 구현 → 사용처를 동시에 검토하는 개발 체크리스트 운용 권장
