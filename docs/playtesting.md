# Galaxy Storm 플레이테스트

## 실행 방식

Windows 빌드를 유지하면서 동일한 C++17/SDL2 소스를 Linux/WSL에서도 실행합니다.
브라우저 포팅이나 별도의 JavaScript 게임 구현은 포함하지 않습니다.

검증은 다음 두 층으로 분리합니다.

| 방식 | 확인하는 내용 | 확인하지 않는 내용 |
|------|---------------|--------------------|
| 네이티브 창 + OS 키보드 입력 | 타이틀, 기체 선택, 이동·사격, 일시정지, 창 포커스 전환 | 사람의 반응 속도, 재미, 사운드 감상 |
| 결정적 C++ 픽스처 | 입력 이벤트 정책, 실제 렌더링 픽셀, 파워업·폭탄·사망/초기화·보스·5개 스테이지 진행 | 자연 플레이로 전 스테이지 클리어, 밸런스 판정 |

## 빌드와 회귀 검사

프로젝트 루트에서 실행합니다.

```bash
cmake -S game -B build -DCMAKE_BUILD_TYPE=Release -DGALAXY_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure --no-tests=error
```

`GalaxyStormCore`는 실제 게임과 검사 실행 파일이 공유하는 코드입니다.
검사용으로 복제한 게임 로직을 실행하지 않습니다. `GALAXY_BUILD_TESTS`는 기본적으로
꺼져 있으므로 일반 빌드에 테스트 실행 파일이 추가되지 않습니다.

Windows에서는 빌드에 `--config Release`, CTest에 `-C Release`를 추가합니다.
SDL 확장 DLL과 그 종속 DLL이 실행 파일 디렉터리 또는 PATH에 있어야 합니다.

16개 검사는 다음 동작을 확인합니다.

- 반복 Enter/Space가 메뉴·기체 선택을 넘어가지 않고, 반복 방향키가 선택을 변경하지 않음.
- 새 키 입력은 세 기체 선택과 High Score 복귀를 정상 처리함.
- P/Esc는 새로 눌렀을 때만 일시정지/재개하며, 창 복귀만으로 게임을 재개하지 않음.
- 보스 HP를 그린 뒤에도 화면 상단 24px의 점수 행이 픽셀 단위로 동일함.
- 480x640 메뉴 자산 로드, 자산 누락 시 START 표시, 실제 폰트 폭에 따른 가운데 정렬.
- 폭탄은 적 탄환만 제거하고, 파워업 획득은 파워 레벨을 변경함.
- 마지막 생명 소진과 새 플레이어 초기화가 체력·기체·버튼 상태를 올바르게 처리함.
- 보스 격파의 점수 중복 지급 방지와 5개 스테이지의 보스 등장/클리어 상태.

## 동일 조건 화면 캡처

```bash
cd game
../build/GalaxyStormPlaytest --capture ../build/playtest-screens
```

Windows 실행 파일 경로는 `../build/Release/GalaxyStormPlaytest.exe`입니다.
타이틀, 세 기체 선택 화면과 보스 HUD를 실제 렌더 함수로 PNG에 저장합니다.
SDL 소프트웨어 렌더러를 사용하므로 이 명령에는 X 서버가 필요하지 않습니다.
이 캡처는 렌더링 픽스처이며, 플레이 영상으로 표시하지 않습니다.

## 실제 창 입력 검사 (Linux / WSL)

Pillow, xdotool, Xvfb가 설치된 환경에서 다른 창에 영향을 주지 않도록
별도 디스플레이를 사용합니다.

```bash
sudo apt-get install xvfb xauth xdotool python3-pil
xvfb-run -a python3 game/tools/native_playtest.py \
  --executable "$(realpath build/GalaxyStorm)" \
  --output "$(realpath build)/native-playtest"
```

스크립트는 지정한 실행 파일의 프로세스와 창만 대상으로 사용합니다.
게임 종료 시 입력을 해제하고 소유한 프로세스를 정리합니다. 환경에 따라 실행
속도와 생존 시간이 달라지므로, 실제 확인된 범위는 출력되는 보고서와 캡처를
기준으로 판단합니다. 음소거/dummy 오디오 환경은 사운드 품질 검증이 아닙니다.

## 자산

`menu_backdrop.png`는 원본 CC0 절차적 픽셀 아트입니다. 타이틀과 기체 선택에
같은 배경을 사용하며, 게임 중 탄막·판정·공격력은 변경하지 않습니다.

```bash
python3 game/tools/generate_menu_backdrop.py --check
```

생성기는 Python/Pillow를 사용합니다. 최종 게임은 생성된 PNG만 읽습니다.
배경 로드 실패 시 경고를 기록하고 어두운 단색 메뉴로 표시합니다.

## 개선 항목과 검토 기준

- [#39 — 키 반복으로 인한 화면 전환](https://github.com/junpark12/retro-shooter-agents/issues/39)
- [#40 — 보스 HP와 점수 HUD 겹침](https://github.com/junpark12/retro-shooter-agents/issues/40)
- [#41 — 메뉴 시각·조작 안내 개선 제안](https://github.com/junpark12/retro-shooter-agents/issues/41)

재현된 기능 문제와 주관적 UI 제안을 별도 이슈로 관리합니다. 자산은 파일만
올리지 않고 로딩, 렌더링, 출처 기록, 재현 가능한 생성 방법을 함께 제공합니다.
난이도 조절이나 보호막 기능 변경은 이번 개선에 포함하지 않습니다.
