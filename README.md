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
| **타겟** | Windows 네이티브 (.exe) |
| **빌드** | CMake |

### Features
- 3단계 스테이지 + 보스전
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
