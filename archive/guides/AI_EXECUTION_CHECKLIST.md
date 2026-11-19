# AI 실행 체크리스트: Unreal Vehicle ↔ Web MQTT

## 역할과 완료 목표

당신은 빈 Unreal Engine 5.7 Vehicle Template 프로젝트에서 아래 결과를 재현한다.

1. 웹/모바일의 W/A/S/D 입력이 MQTT를 통해 Unreal 차량을 제어한다.
2. Unreal Play 창의 W/A/S/D 상태가 웹에 보인다.
3. macOS와 Windows에서 실행 기준을 분리한다.
4. 실제 로버는 연결하지 않는다. 테스트 토픽만 쓴다.

## 입력 자료

아래 `references/` 파일은 설계 참고용이다. 코드를 그대로 복사하지 말고, 현재 요구사항에 맞춰 구현한다.

| 파일 | 읽는 목적 |
| --- | --- |
| `references/ch12_mqtt.pdf` | MQTT Broker, Publish, Subscribe, 디지털 트윈 개념 |
| `references/mqtt_1_publish.py` | Publish 흐름 |
| `references/mqtt_2_subscribe.py` | Subscribe와 명령 처리 흐름 |
| `references/mqtt_3_subscribe_try.py` | 재연결 후 재구독 필요성 |
| `references/mqtt_4_motor.py` | FORWARD/BACKWARD/LEFT/RIGHT/STOP 명령 매핑 참고 |

## 고정 규약 — 변경하지 말 것

```text
웹 → Unreal 토픽: testvi/web/input
Unreal → 웹 토픽: testvi/unreal/input
웹 JSON: {"source":"web","keys":{"w":true,"a":false,"s":false,"d":false}}
Unreal JSON: {"source":"unreal","keys":{"w":true,"a":false,"s":false,"d":false}}
발행 간격: 0.1초
웹 입력 타임아웃: 0.35초
Throttle: W - S
Steering: D - A
```

## 실행 단계

### A. 환경 판별 — 반드시 먼저 수행

- [ ] 운영체제를 확인한다: macOS 또는 Windows.
- [ ] Unreal Engine 5.7 설치 여부를 확인한다.
- [ ] 프로젝트가 없으면 Games → Vehicle Template으로 새 프로젝트를 생성한다.
- [ ] 기본 Vehicle Template에서 Play 후 키보드 WASD 차량 제어가 되는지 확인한다.

### B. Unreal 프로젝트 설정

- [ ] `.uproject`에서 `ChaosVehiclesPlugin`과 `MQTT`를 활성화한다.
- [ ] Blueprint-only 프로젝트를 Runtime C++ 모듈 프로젝트로 전환한다.
- [ ] Build.cs 의존성에 `Core`, `CoreUObject`, `Engine`, `InputCore`, `Json`, `JsonUtilities`, `ChaosVehicles`, `MQTTCore`를 추가한다.
- [ ] `UTickableWorldSubsystem` 기반 MQTT bridge를 만든다.

### C. Unreal MQTT bridge 구현

- [ ] `GetWorld()->IsGameWorld()`가 true일 때만 연결한다. Editor 월드에서는 연결하지 않는다.
- [ ] `IMQTTCoreModule`과 `IMQTTClient`를 사용한다. MQTT Blueprint wrapper 객체를 C++에서 직접 호출하지 않는다.
- [ ] 로컬 MQTT TCP broker `127.0.0.1:1883`에 연결한다.
- [ ] `testvi/web/input`을 구독한다.
- [ ] JSON keys를 파싱하고 Player Pawn의 `UChaosWheeledVehicleMovementComponent`를 찾는다.
- [ ] Throttle과 Steering을 적용한다.
- [ ] 메시지가 0.35초 이상 없으면 throttle/steering을 0으로 적용한다.
- [ ] `APlayerController::IsInputKeyDown(W/A/S/D)`로 Unreal 입력을 읽는다.
- [ ] `testvi/unreal/input`에 0.1초마다 JSON을 발행한다.
- [ ] 프로젝트를 UE 5.7으로 빌드한다.

### D. 웹 컨트롤러 구현

- [ ] 단일 HTML 페이지를 만든다.
- [ ] `mqtt` npm 패키지를 로컬 의존성으로 설치한다. 외부 CDN에 의존하지 않는다.
- [ ] MQTT-over-WebSocket URL 입력칸과 연결 상태를 만든다.
- [ ] 키보드 WASD 입력을 구현한다.
- [ ] 모바일 Pointer Events 기반 W/A/S/D 버튼을 구현한다.
- [ ] 멀티터치(W+A 같은 동시 입력)를 지원한다.
- [ ] 현재 누르는 키를 표시한다.
- [ ] 마지막 MQTT 송신 상태와 시간을 표시한다.
- [ ] `testvi/unreal/input`을 구독해 Unreal 입력 상태를 표시한다.
- [ ] `blur`, `pointerup`, `pointercancel`에서 키를 해제하고 STOP 상태를 발행한다.

### E. MQTT 브로커 구현

- [ ] Mosquitto를 설치한다.
- [ ] TCP listener 1883은 `127.0.0.1`로 제한한다.
- [ ] WebSocket listener 9001은 현재 Wi-Fi IPv4에 바인딩한다.
- [ ] 개발 중에는 anonymous 접속을 허용할 수 있으나, 공용 네트워크에는 배포하지 않는다.

### F. 운영체제별 명령

#### macOS

```bash
brew install mosquitto
ipconfig getifaddr en0
/opt/homebrew/opt/mosquitto/sbin/mosquitto -c web-controller/mosquitto.local.conf -v
cd web-controller && npm install
python3 -m http.server 5173
```

#### Windows

```bat
REM Mosquitto Windows 설치본을 설치한다.
ipconfig
mosquitto -c web-controller\mosquitto.windows.conf -v
cd web-controller
npm install
py -m http.server 5173
```

### G. 검증

- [ ] MQTT broker가 TCP 1883과 WebSocket 9001에서 listen한다.
- [ ] 웹에서 `MQTT 연결됨` 상태가 보인다.
- [ ] Unreal Editor를 재시작한 뒤 ▶ Play를 직접 실행한다.
- [ ] Unreal Output Log에 `MQTT vehicle bridge subscribed to testvi/web/input`가 보인다.
- [ ] 웹 키보드 W/A/S/D가 차량을 움직인다.
- [ ] 모바일 터치 W/A/S/D가 차량을 움직인다.
- [ ] Unreal Play 창의 WASD가 웹에 나타난다.
- [ ] 웹을 닫거나 메시지를 멈추면 차량이 0.35초 이내 멈춘다.

## 사람에게 반드시 요청할 항목 — AI가 임의로 넘기지 말 것

| 상황 | 요청할 내용 |
| --- | --- |
| Unreal Editor가 C++ 전환/컴파일 승인 창을 보임 | 승인 요청 |
| Editor에서 Play를 시작해야 함 | 사용자에게 ▶ Play 클릭 요청 |
| Homebrew/Mosquitto/Node 설치가 필요함 | 설치 권한 요청 |
| macOS/Windows 방화벽이 수신 연결을 막음 | Python/Mosquitto 사설 네트워크 허용 요청 |
| 휴대폰 접속 필요 | 현재 Wi-Fi IP와 동일 Wi-Fi 여부 확인 요청 |
| Cloudflare 공개 배포 | 공개 MQTT broker, TLS/WSS, 인증 방식 선택 요청 |

## 제외 범위

- 실제 로버 모터 제어
- `rover/command/move` 발행
- 공용 MQTT broker로 anonymous 제어 공개
- Cloudflare에서 Mosquitto 자체를 실행하는 구성

## 최종 보고 형식

작업 완료 시 다음만 보고한다.

1. 생성/수정 파일 목록
2. 빌드 결과
3. 현재 MQTT 주소와 토픽
4. 사용자가 수행해야 하는 마지막 테스트 단계
5. 알려진 미검증 항목 또는 차단 요인
