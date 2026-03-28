# Copilot Instructions — Galaxy Storm

## Project Context
이 프로젝트는 Gunvein 스타일의 Bullet Hell 종스크롤 슈팅 게임 "Galaxy Storm"입니다.
C++ (C++17)과 SDL2 + SDL2_image + SDL2_mixer + SDL2_ttf를 사용하며,
Windows 네이티브 실행파일(.exe)을 빌드 타겟으로 합니다.
3D 렌더링된 스프라이트, BGM, SFX를 사용하는 고품질 아케이드 슈팅 게임입니다.

## Tech Stack
- **언어**: C++17
- **그래픽**: SDL2 + SDL2_image (PNG 스프라이트 로딩)
- **오디오**: SDL2_mixer (BGM + SFX)
- **폰트**: SDL2_ttf (TTF 폰트 렌더링)
- **빌드**: CMake 3.16+
- **타겟 OS**: Windows (Visual Studio 또는 MinGW)

## Architecture Rules
- 헤더 파일은 `game/src/`에, 소스 파일은 `game/src/`에 배치
- 모든 헤더는 `#pragma once` 사용
- 스프라이트는 에셋 기반 (SDL2_image로 PNG 로드, `game/assets/sprites/`)
- 오디오는 SDL2_mixer (BGM: `game/assets/bgm/`, SFX: `game/assets/sfx/`)
- 폰트는 SDL2_ttf (TTF: `game/assets/fonts/`)
- 에셋 로드 실패 시 SDL2 프리미티브로 fallback 렌더링
- 게임 루프: 고정 60fps (16.67ms per frame), delta-time 기반 업데이트
- 총알/이펙트는 오브젝트 풀 패턴 사용 (MAX_BULLETS=500)
- 게임 상태: TITLE → SHIP_SELECT → PLAYING → STAGE_CLEAR → GAMEOVER → VICTORY

## Game Design (Gunvein 스타일 Bullet Hell)
- 화면 해상도: 480x640 (세로 모드)
- 3단계 스테이지, 각 스테이지 끝에 보스전
- 3기체 선택 (Type A: 집중형, B: 광역형, C: 폭발형)
- Lock-on 시스템 + 차지 블라스트 + 폭탄
- 탄막 시스템 (원형, 나선, 부채꼴, 유도 패턴)
- 파워업: 스프레드샷, 레이저, 미사일, 쉴드
- Score → Extra Lives
- 플레이어 소형 히트박스 (Bullet Hell 표준)

## Code Style
- 네임스페이스: 최상위 `galaxy` 네임스페이스
- 구조체/클래스 이름: PascalCase
- 함수/변수: camelCase
- 상수: UPPER_SNAKE_CASE
- 한 파일에 하나의 클래스/모듈
- 코드 내 주석은 영문으로 작성

## File Structure
```
game/
├── CMakeLists.txt
├── ARCHITECTURE.md
├── src/
│   ├── main.cpp            # 엔트리포인트, SDL 초기화
│   ├── game.cpp            # 게임 상태 관리, 메인 루프
│   ├── player.cpp          # 플레이어 (3기체, Lock-on, 차지샷, 폭탄)
│   ├── enemy.cpp           # 적 로직
│   ├── bullet.cpp          # 총알 풀 관리
│   ├── bullet_pattern.cpp  # 탄막 패턴 생성기
│   ├── boss.cpp            # 보스 시스템 (다중 Phase)
│   ├── powerup.cpp         # 파워업 아이템
│   ├── collision.cpp       # 충돌 감지 (AABB + 원형)
│   ├── stage.cpp           # 스테이지/웨이브 시스템
│   ├── asset_manager.cpp   # 에셋 로딩 (SDL2_image)
│   ├── audio.cpp           # BGM/SFX (SDL2_mixer)
│   ├── sprites.cpp         # 스프라이트 렌더링
│   ├── hud.cpp             # HUD (SDL2_ttf)
│   ├── menu.cpp            # 메뉴 화면 (기체 선택 포함)
│   └── background.cpp      # 스크롤 배경
├── include/
│   └── (각 .cpp에 대응하는 .h)
└── assets/
    ├── sprites/            # PNG 스프라이트
    │   ├── player/         # 플레이어 기체
    │   ├── enemies/        # 적
    │   ├── bosses/         # 보스
    │   ├── bullets/        # 탄막
    │   ├── effects/        # 폭발 등
    │   ├── powerups/       # 파워업
    │   ├── backgrounds/    # 배경
    │   └── ui/             # HUD 요소
    ├── bgm/                # 배경 음악 (OGG/WAV)
    ├── sfx/                # 효과음 (WAV)
    ├── fonts/              # TTF 폰트
    └── CREDITS.md          # 에셋 라이선스 기록
```

## CMake Guidelines
- minimum version: 3.16
- find_package(SDL2 REQUIRED)
- find_package(SDL2_image REQUIRED)
- find_package(SDL2_mixer REQUIRED)
- find_package(SDL2_ttf REQUIRED)
- C++17 표준 설정
- Windows에서 콘솔 창 숨기기: WIN32_EXECUTABLE
- 에셋 디렉토리를 빌드 출력에 복사 (file(COPY) 또는 post-build)
