# 🎮 Galaxy Storm — 90s Arcade Shooter

> **GitHub Copilot Coding Agent**로 개발하는 90년대 스타일 종스크롤 슈팅 게임

## 🤖 Custom Agents (`.github/agents/`)

이 프로젝트는 **GitHub Copilot Custom Agents**로 팀을 구성합니다.
각 에이전트는 독립된 역할과 인스트럭션을 가지고 있습니다.

```
.github/agents/
  ├── pl.agent.md            🧑‍💼 Project Leader
  ├── developer.agent.md     👨‍💻 Senior Developer
  ├── ui-designer.agent.md   🎨 UI/Art Designer
  └── tester.agent.md        🧪 QA Engineer
```

### Agent 호출 방법
Copilot Chat에서 에이전트를 직접 호출:
```
@pl 아키텍처 설계를 시작해줘. types.h와 entity.h 인터페이스를 정의해.
@developer 플레이어 시스템을 구현해. player.h/cpp 작성해줘.
@ui-designer 스프라이트 렌더링 함수를 구현해. sprites.h/cpp 작성.
@tester 전체 코드 리뷰하고 qa-report.md 작성해줘.
```

## 🎯 Game Spec

| 항목 | 내용 |
|------|------|
| **이름** | Galaxy Storm |
| **장르** | 종스크롤 슈팅 (Vertical Scrolling Shooter) |
| **스타일** | 90년대 아케이드, 16비트 픽셀 느낌 |
| **기술** | C++ (C++17) + SDL2 |
| **타겟** | Windows 네이티브 (.exe), Linux / WSL 네이티브 |
| **빌드** | CMake |

### Features
- 5단계 스테이지 + 보스전
- 파워업 시스템 (스프레드샷, 레이저, 미사일, 쉴드)
- 레트로 네온 비주얼 (프로그래매틱 스프라이트)
- 하이스코어 시스템
- 키보드 조작

## 📁 Project Structure

```
game/
├── CMakeLists.txt        # 빌드 설정
├── src/                  # 소스 코드 (.cpp)
├── include/              # 헤더 파일 (.h)
└── assets/               # 게임 에셋
```

## 🔨 Build (Windows)

```bash
cd game
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

## Linux / WSL에서 실행

```bash
sudo apt-get install cmake g++ libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev
cmake -S game -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cd build
./GalaxyStorm
```

WSL에서는 WSLg 또는 X11 디스플레이가 필요합니다. 실행 파일 옆의 `assets/`를
사용하므로 빌드 디렉터리에서 실행합니다. C++/SDL2 게임을 그대로 실행하며,
브라우저용으로 다시 구현하지 않습니다.

### 조작

| 동작 | 키 |
|------|----|
| 메뉴 이동 / 기체 선택 | 방향키, WASD |
| 선택 확인 | Enter / Space |
| 이동 | 방향키 / WASD |
| 기본 사격 / 차지 공격 | X / Space를 누른 뒤 놓기 |
| 락온 / 발사 | Z를 누른 뒤 놓기 |
| 폭탄 | C |
| 일시정지 / 재개 | P / Esc |

메뉴 선택과 일시정지는 **새로 누른 키**에만 반응합니다. 게임 중 창의 포커스를
잃거나 최소화하면 자동으로 일시정지하며, 다시 창을 선택해도 자동 재개하지 않습니다.
P 또는 Esc로 재개합니다. Continue 화면의 Esc는 기존처럼 포기를 의미합니다.

## 재현 가능한 플레이테스트

```bash
cmake -S game -B build -DCMAKE_BUILD_TYPE=Release -DGALAXY_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure --no-tests=error
cd game
../build/GalaxyStormPlaytest --capture ../build/playtest-screens
```

Windows 다중 구성 빌드는 `--config Release`, CTest는 `-C Release`를 사용하고
캡처 실행 파일은 `build/Release/GalaxyStormPlaytest.exe`입니다.
회귀 검사는 SDL 소프트웨어 렌더러를 사용하므로 화면 서버 없이 실행할 수 있습니다.
메뉴, 세 기체의 선택 화면, 보스 HUD를 같은 조건에서 PNG로 저장합니다.
스테이지 검사는 적을 제거하는 테스트 픽스처이므로 자연 플레이 클리어나 난이도
평가를 의미하지 않습니다.

추가 도구로 실제 창을 조작하는 방법과 검증 범위는
[플레이테스트 가이드](docs/playtesting.md)를 참고하세요.

### 메뉴 자산 재생성

`game/assets/sprites/ui/menu_backdrop.png`는 외부 이미지가 아닌 CC0 원본 절차적
픽셀 아트입니다. Python과 Pillow가 설치된 환경에서 재생성·검증할 수 있습니다.

```bash
python3 game/tools/generate_menu_backdrop.py
python3 game/tools/generate_menu_backdrop.py --check
```

출처와 라이선스는 [CREDITS](game/assets/CREDITS.md)에 기록합니다.

## 🤖 Agent 팀 구성

| Agent | File | 역할 | Tools |
|-------|------|------|-------|
| 🧑‍💼 PL | `pl.agent.md` | 아키텍처 설계, 기술 결정, 코드 리뷰 | read, search, github |
| 👨‍💻 Developer | `developer.agent.md` | 게임 엔진, 로직, 물리, 충돌 구현 | read, edit, terminal, search |
| 🎨 UI Designer | `ui-designer.agent.md` | 스프라이트, HUD, 메뉴, 시각 효과 | read, edit, search |
| 🧪 Tester | `tester.agent.md` | 코드 리뷰, 빌드 검증, 버그 리포트 | read, search, terminal |

## 📋 개발 플로우

```
Phase 1: @pl         → 아키텍처 설계, 헤더 인터페이스 정의
Phase 2: @developer  → 게임 엔진 핵심 구현
         @ui-designer → 스프라이트, HUD, 메뉴 (병렬 작업)
Phase 3: @developer  → 보스 시스템, 전체 통합
Phase 4: @tester     → 코드 리뷰, 빌드 검증, QA 리포트
```

## 📜 License

MIT
