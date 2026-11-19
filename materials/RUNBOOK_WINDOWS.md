# Windows 실행 런북 — Unreal Vehicle ↔ Web MQTT

대상 OS: **Windows 전용**. macOS 내용은 이 문서에 없습니다.
프로젝트/모듈 이름: **MqttVehicle**

---

## 0. 시작 전에 — 사람이 직접 해야 하는 것

아래는 AI가 대신할 수 없는 항목입니다. **1~5는 A 단계 전에 끝나 있어야** 나머지가 진행됩니다.

| # | 항목 | 확인 방법 / 비고 |
| --- | --- | --- |
| 1 | **Epic Games 로그인 + Unreal Engine 5.7 설치** | Epic Games Launcher → Unreal Engine → 라이브러리. 계정 로그인이 필요합니다. |
| 2 | **Visual Studio 2022 + "C++를 사용한 게임 개발" 워크로드** | 없으면 B단계의 C++ 전환이 그 자리에서 실패합니다. 설치 관리자에서 "개별 구성 요소 → Unreal Engine 설치 관리자"도 함께 체크. |
| 3 | **Mosquitto Windows x64 설치** | https://mosquitto.org/download/ · 관리자 권한 필요 · 기본 경로 `C:\Program Files\mosquitto\` |
| 4 | **Node.js LTS 설치** | `npm -v` 로 확인 |
| 5 | **Python 3 설치 (py 런처 포함)** | `py --version` 으로 확인 |
| 6 | **Windows Defender 방화벽 허용** | 브로커/웹 서버 첫 실행 시 뜨는 창에서 **사설 네트워크** 체크 후 허용. 휴대폰 접속에 필요합니다. |
| 7 | **Unreal Editor의 C++ 전환 / 컴파일 승인 창** | "이 프로젝트를 C++로 변환할까요?", "모듈을 리빌드할까요?" 창이 뜨면 직접 승인해야 합니다. |
| 8 | **Editor에서 ▶ Play 클릭** | 자동화하지 않습니다. |
| 9 | **휴대폰이 PC와 같은 Wi-Fi** | 회사/학교 Wi-Fi는 기기 간 통신(AP isolation)이 막혀 있는 경우가 많습니다. 안 되면 휴대폰 핫스팟에 PC를 붙이세요. |

> 실제 로버는 연결하지 않습니다. `rover/command/move` 는 발행하지 않으며 `testvi/*` 토픽만 사용합니다.

---

## A. 환경 판별

```bat
where mosquitto
npm -v
py --version
ipconfig
```

- [ ] `ipconfig` 출력에서 **Wi-Fi 어댑터의 IPv4 주소**를 적어둡니다. 예: `192.168.0.42`
- [ ] Unreal Engine 5.7 설치 확인.
- [ ] Epic Games Launcher → **Games → Vehicle** 템플릿으로 새 프로젝트 생성.
  - Blueprint 선택 (C++ 전환은 B단계에서 합니다)
  - 프로젝트 이름: **`MqttVehicle`** ← 이 이름이 C++ 모듈명과 `MQTTVEHICLE_API` 매크로에 그대로 쓰입니다. 다르게 지으면 소스 3개를 전부 고쳐야 합니다.
- [ ] ▶ Play 후 키보드 WASD로 차가 움직이는지 먼저 확인합니다. **여기서 안 되면 그 다음은 전부 무의미합니다.**

---

## B. Unreal 프로젝트 설정

### B-1. 플러그인 활성화

Editor → Edit → Plugins

- [ ] `Chaos Vehicles Plugin` — Vehicle Template은 보통 이미 켜져 있습니다.
- [ ] `MQTT` — **Experimental**이라 목록에 안 보이면 톱니바퀴 아이콘에서 "Show Experimental Plugins"를 켜세요.

플러그인 이름이 다를 수 있습니다. 실제 이름 확인:

```bat
dir /s /b "C:\Program Files\Epic Games\UE_5.7\Engine\Plugins\*MQTT*.uplugin"
```

> **여기서 MQTT 플러그인이 아예 없으면 중단하고 알려주세요.** 5.7에서 제거·개명됐을 가능성이 있고, 그 경우 구현 방식을 다시 정해야 합니다.

Editor 재시작을 요구하면 재시작합니다.

### B-2. C++ 프로젝트로 전환

Blueprint 프로젝트를 손으로 C++화하지 말고, Editor가 만들게 하세요. 그래야 `Target.cs`, 모듈 `.h/.cpp`, `.uproject` 갱신이 한 번에 정확히 됩니다.

- [ ] Editor → Tools → **New C++ Class** → Show All Classes → 부모 클래스 **None** 선택
- [ ] 이름: `MqttVehicleTemp` → Create Class
- [ ] 컴파일/전환 승인 창이 뜨면 승인 (사람 항목 #7)
- [ ] 컴파일이 끝나면 **Editor를 완전히 종료**합니다.

### B-3. 소스 파일 배치

이 폴더의 `unreal\Source\MqttVehicle\` 안 3개 파일을 프로젝트의 같은 경로로 복사합니다.

```
<프로젝트>\Source\MqttVehicle\MqttVehicle.Build.cs                 ← 덮어쓰기
<프로젝트>\Source\MqttVehicle\MqttVehicleBridgeSubsystem.h         ← 새로 추가
<프로젝트>\Source\MqttVehicle\MqttVehicleBridgeSubsystem.cpp       ← 새로 추가
```

- [ ] B-2에서 만든 `MqttVehicleTemp.h` / `.cpp` 는 삭제합니다.
- [ ] `MqttVehicle.h` / `MqttVehicle.cpp` (모듈 구현부)와 `Source\*.Target.cs` 는 **건드리지 않습니다.**
- [ ] `MqttVehicle.uproject` 를 열어 `Plugins` 배열에 `ChaosVehiclesPlugin` 과 `MQTT` 가 `"Enabled": true` 로 있는지 확인합니다. 형식은 `unreal\MqttVehicle.uproject.reference.json` 참고.

### B-4. 빌드

- [ ] `MqttVehicle.uproject` 우클릭 → **Generate Visual Studio project files**
- [ ] `MqttVehicle.sln` 열기 → 구성 **Development Editor** / 플랫폼 **Win64** → 빌드

빌드가 MQTT 관련 심볼에서 깨지면 `MqttVehicleBridgeSubsystem.cpp` 상단의 "엔진 MQTT 플러그인 접점" 주석 블록을 보세요. 확인할 4가지와 헤더 목록 조회 명령이 적혀 있습니다.

---

## C. 구현 내용 (참고 — 이미 코드에 반영됨)

체크리스트 C단계 요구사항이 코드 어디에 있는지 대응표입니다.

| 요구사항 | 구현 위치 |
| --- | --- |
| Game World에서만 연결 | `DoesSupportWorldType()` + `Initialize()` 의 `IsGameWorld()` 가드 |
| `IMQTTCoreModule` / `IMQTTClient` 직접 사용 | `ConnectAndSubscribe()` (Blueprint wrapper 미사용) |
| `127.0.0.1:1883` 연결 | `ConnectAndSubscribe()` 의 `FMQTTURL` |
| `testvi/web/input` 구독 | `Subscribe()` — 재연결 시에도 붙도록 connect 완료 콜백에서 수행 |
| JSON 파싱 + 차량 컴포넌트 탐색 | `HandleMqttMessage()`, `ApplyWebInputToVehicle()` |
| Throttle / Steering 적용 | `ApplyWebInputToVehicle()` |
| 0.35초 무입력 시 정지 | `ApplyWebInputToVehicle()` 의 `WebInputTimeout` |
| Unreal 키 상태 읽기 | `PublishUnrealInput()` 의 `IsInputKeyDown(EKeys::W...)` |
| `testvi/unreal/input` 0.1초 발행 | `Tick()` 의 `PublishAccumulator` |

### 규약에서 의도적으로 벗어난 3곳

원래 명세대로 짜면 실제로 동작하지 않아서 바꾼 부분입니다. 되돌리려면 알려주세요.

1. **`Throttle = W - S` → 기어 + 브레이크 분리**
   Chaos의 `SetThrottleInput()` 은 0~1로 클램프됩니다. `W - S` 를 그대로 넣으면 **S가 아무 일도 하지 않습니다.**
   → W/S를 기어(`SetTargetGear`)와 브레이크(`SetBrakeInput`)로 나눠 처리합니다. 정지 판정 임계값은 50 cm/s.

2. **타임아웃 시 `throttle=0` → `throttle=0 + brake=1`**
   스로틀만 0으로 두면 관성으로 계속 굴러가서 "0.35초 이내 정지"(G 항목)를 만족하지 못합니다.

3. **웹이 3초 이상 조용하면 제어권 반환**
   이게 없으면 웹을 끈 뒤 브릿지가 매 프레임 브레이크를 걸어 **Play 창에서 키보드로 직접 운전하는 것까지 막습니다.**
   덤으로 입력 충돌 규칙이 정해집니다: **웹이 살아있는 동안은 웹이 이깁니다.**

기타: MQTT 수신 콜백은 게임 스레드가 아니므로 값만 저장하고 적용은 `Tick()` 에서 합니다. QoS는 0, retain은 false입니다.

---

## D~E. 웹 컨트롤러와 브로커

`web-controller\` 폴더가 그대로 준비돼 있습니다.

```bat
cd /d "<이 폴더>\web-controller"
npm install
```

`npm install` 이 `node_modules\mqtt\dist\mqtt.min.js` 를 만듭니다. 페이지는 이 로컬 파일만 참조하며 CDN을 쓰지 않습니다.

| 파일 | 역할 |
| --- | --- |
| `index.html` | 키보드 + 멀티터치 WASD, 100ms 하트비트 발행, Unreal 입력 표시 |
| `mosquitto.windows.conf` | TCP 1883 (127.0.0.1 전용) / WebSocket 9001 (0.0.0.0) |
| `start-broker.bat` | 서비스 중지 후 브로커 실행 |
| `start-web.bat` | IPv4 출력 + 정적 서버 5173 |

> WebSocket 리스너를 Wi-Fi IP로 못박지 않고 `0.0.0.0` 으로 연 이유: 그렇게 하면 같은 PC의 브라우저가 `localhost` 로 못 붙고, DHCP로 IP가 바뀔 때마다 conf를 고쳐야 합니다. 노출 범위는 방화벽(사설 네트워크만)으로 통제합니다.

---

## F. 실행 순서

창 3개를 띄웁니다.

**창 1 — 브로커**
```bat
cd /d "<이 폴더>\web-controller"
start-broker.bat
```
`Opening ipv4 listen socket on port 1883` 과 `... port 9001` 두 줄이 보여야 합니다.

**창 2 — 웹 서버**
```bat
cd /d "<이 폴더>\web-controller"
start-web.bat
```

**창 3 — 브로커 동작 확인 (선택이지만 강력 권장)**
```bat
"C:\Program Files\mosquitto\mosquitto_sub.exe" -h 127.0.0.1 -t "testvi/#" -v
```
문제가 생겼을 때 브로커/웹/Unreal 중 어디가 범인인지 여기서 바로 갈립니다.

**Unreal**
- `MqttVehicle.uproject` 실행 → ▶ **Play** 클릭 (사람 항목 #8)

---

## G. 검증

순서대로 하나씩 확인하세요. 앞 항목이 실패하면 뒤는 볼 필요가 없습니다.

- [ ] **G-1** 브로커가 1883과 9001에서 listen. → 창 1 로그
- [ ] **G-2** PC 브라우저에서 `http://127.0.0.1:5173` 접속 → 주소창 기본값 `ws://127.0.0.1:9001` 로 **연결** → 상태가 초록 **"MQTT 연결됨"**
- [ ] **G-3** 창 3에 `testvi/web/input {"source":"web","keys":{...}}` 가 0.1초마다 흐름
- [ ] **G-4** Unreal Output Log에 `MQTT vehicle bridge subscribed to testvi/web/input`
- [ ] **G-5** 웹에서 키보드 W/A/S/D → 차량이 움직임 (전진, 조향, 후진)
- [ ] **G-6** 휴대폰에서 `http://<Wi-Fi IPv4>:5173` 접속 → 주소를 `ws://<Wi-Fi IPv4>:9001` 로 바꾸고 연결 → 터치 버튼으로 차량 제어. W+A 동시 터치도 동작.
- [ ] **G-7** Unreal Play 창을 클릭해 포커스를 준 뒤 WASD를 누르면 웹 하단 칩에 초록으로 표시
- [ ] **G-8** 웹 탭을 닫거나 **해제** 버튼 → 차량이 0.35초 이내 정지

### G-7 관련 주의

`IsInputKeyDown` 은 **Play 창에 포커스가 있을 때만** true입니다. 확인하려고 브라우저를 클릭하는 순간 표시가 꺼지는데 이건 정상입니다. 휴대폰으로 보거나, 모니터 두 대에 나눠 띄우고 Play 창에 포커스를 준 채로 확인하세요.

---

## 사전 검증 결과 (전달 전 자동 테스트)

웹/브로커 쪽은 실제 mosquitto 2.0.18 + Chromium 으로 돌려서 확인했습니다. **Unreal C++ 은 UE 5.7 이 없어 컴파일 검증을 못 했습니다.**

| 항목 | 결과 |
| --- | --- |
| `mosquitto.windows.conf` 파싱 및 1883/9001 동시 listen | 통과 |
| 브라우저가 `ws://…:9001` 로 연결, 상태 표시 | 통과 |
| 하트비트 발행 1초간 10건, 최대 간격 104ms (0.35초 여유) | 통과 |
| 키보드 W, 동시 입력 W+A | 통과 |
| 멀티터치 W+D 및 해제 | 통과 |
| `testvi/unreal/input` 수신 → 칩 표시 | 통과 |
| 해제 시 전키 false 발행 + 발행 완전 중단 | 통과 |
| 페이지 JS 런타임 에러 | 없음 |
| Last Will | **불확실** — 브라우저 강제 종료 시 will 이 안 뜨는 경우 확인. 정지 보장은 Unreal 0.35초 페일세이프가 담당합니다. |
| Unreal C++ 컴파일 / 차량 거동 / G-4·G-5·G-7 | **미검증** — Hoon님 PC에서 확인 필요 |

---

## 막혔을 때

| 증상 | 확인할 것 |
| --- | --- |
| 브로커가 `Address already in use` 로 죽음 | mosquitto 서비스가 이미 1883 점유. 관리자 명령 프롬프트에서 `net stop mosquitto` |
| 웹이 계속 "연결 중"에서 멈춤 | 9001 방화벽. `ws://` 인지(`wss://` 아님) 확인. 창 3에서 브로커 자체는 살아있는지 확인 |
| 휴대폰만 접속 안 됨 | 같은 Wi-Fi인지, AP isolation인지. 핫스팟으로 바꿔 재시도 |
| 창 3에는 메시지가 흐르는데 차가 안 움직임 | Unreal 쪽 문제. Output Log에 G-4 로그가 있는지부터 |
| G-4 로그가 없음 | MQTT 플러그인 비활성 / 브로커 미실행 / Editor 월드에서 실행(반드시 ▶ Play) |
| S를 눌러도 후진 안 함 | 차량 BP의 Transmission 설정 확인. `SetTargetGear(-1)` 이 자동 변속에 덮이면 알려주세요 |
| 빌드가 MQTT 심볼에서 실패 | `.cpp` 상단 "엔진 MQTT 플러그인 접점" 주석의 확인 4가지 |

---

## 제외 범위 (건드리지 않음)

- 실제 로버 모터 제어
- `rover/command/move` 발행
- 공용 MQTT 브로커 / anonymous 공개
- Cloudflare에서 Mosquitto 실행
