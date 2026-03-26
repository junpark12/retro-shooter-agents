# Copilot Instructions — Galaxy Storm

## Project Context
이 프로젝트는 90년대 아케이드 스타일 종스크롤 슈팅 게임 "Galaxy Storm"입니다.
C++ (C++17)과 SDL2를 사용하며, Windows 네이티브 실행파일(.exe)을 빌드 타겟으로 합니다.

## Tech Stack
- **언어**: C++17
- **그래픽**: SDL2 (SDL_Renderer 기반)
- **빌드**: CMake 3.16+
- **타겟 OS**: Windows (Visual Studio 또는 MinGW)

## Architecture Rules
- 헤더 파일은 `game/include/`에, 소스 파일은 `game/src/`에 배치
- 모든 헤더는 `#pragma once` 사용
- 스프라이트는 프로그래매틱 생성 (외부 이미지 파일 없음, SDL2 프리미티브로 렌더링)
- 게임 루프: 고정 60fps (16.67ms per frame), delta-time 기반 업데이트
- 총알/이펙트는 오브젝트 풀 패턴 사용
- 게임 상태: TITLE → PLAYING → STAGE_CLEAR → GAMEOVER → VICTORY

## Game Design
- 화면 해상도: 480x640 (세로 모드)
- 3단계 스테이지, 각 스테이지 끝에 보스전
- 파워업: 스프레드샷, 레이저, 미사일, 쉴드
- 적 유형: 소형(직선 이동), 중형(사인파), 대형(추적)
- 90년대 네온 컬러 팔레트 (시안, 마젠타, 네온그린, 옐로우)

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
├── src/
│   ├── main.cpp          # 엔트리포인트, SDL 초기화
│   ├── game.cpp          # 게임 상태 관리, 메인 루프
│   ├── player.cpp        # 플레이어 로직
│   ├── enemy.cpp         # 적 로직
│   ├── bullet.cpp        # 총알 풀 관리
│   ├── boss.cpp          # 보스 시스템
│   ├── powerup.cpp       # 파워업 아이템
│   ├── collision.cpp     # 충돌 감지
│   ├── stage.cpp         # 스테이지/웨이브 시스템
│   ├── sprites.cpp       # 프로그래매틱 스프라이트
│   ├── hud.cpp           # HUD 렌더링
│   ├── menu.cpp          # 메뉴 화면
│   ├── background.cpp    # 스타필드 배경
│   └── sound.cpp         # 사운드 이펙트 (선택)
├── include/
│   └── (각 .cpp에 대응하는 .h)
└── assets/
    └── (필요 시 에셋 파일)
```

## CMake Guidelines
- minimum version: 3.16
- SDL2 찾기: find_package(SDL2 REQUIRED)
- C++17 표준 설정
- Windows에서 콘솔 창 숨기기: WIN32_EXECUTABLE
