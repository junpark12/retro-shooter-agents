# 🎮 Galaxy Storm — 90s Arcade Shooter

> **GitHub Copilot Coding Agent**로 개발하는 90년대 스타일 종스크롤 슈팅 게임

## 🏗️ Workflow

```
 GitHub Issues (역할별 태스크)
     │
     ├── 🧑‍💼 PL 이슈        → @copilot 할당 → PR 자동 생성
     ├── 👨‍💻 Dev 이슈       → @copilot 할당 → PR 자동 생성
     ├── 🎨 Design 이슈    → @copilot 할당 → PR 자동 생성
     └── 🧪 Test 이슈      → @copilot 할당 → PR 자동 생성
           │
           ▼
     Review & Merge → 🎮 게임 완성!
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

## 🤖 Agent 역할 (GitHub Issues Labels)

| Label | Role | 담당 |
|-------|------|------|
| `role:pl` | Project Leader | 아키텍처 설계, 기술 결정, 코드 리뷰 |
| `role:dev` | Developer | 게임 엔진, 로직, 물리, 충돌 구현 |
| `role:design` | UI Designer | 스프라이트, HUD, 메뉴, 시각 효과 |
| `role:test` | Tester | 빌드 검증, 코드 리뷰, 버그 리포트 |
| `phase:1` ~ `phase:4` | Phase | 개발 단계 |

## 📋 How to Use

1. 각 이슈를 **Copilot**에 할당 (`@copilot`)
2. Copilot이 자동으로 브랜치 생성 + PR 제출
3. PR 리뷰 후 머지
4. 다음 Phase 이슈 진행

> ⚠️ Phase 순서대로 진행하세요. Phase 1 (아키텍처) 머지 후 Phase 2 시작.

## 📜 License

MIT
