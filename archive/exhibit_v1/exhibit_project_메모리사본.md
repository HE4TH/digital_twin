---
name: exhibit-project
description: "전시회 프로젝트 (팀 과제): QR -> 언리얼 가상 구급차 조작 + 실물 RC카 동기화, 고령자 집 낙상 시뮬레이션. Same Pi4 hardware as digital_twin_project, separate codebase."
metadata: 
  node_type: memory
  type: project
  originSessionId: 1ca2ab8a-ae44-45b3-b375-1c08f713da42
  modified: 2026-09-22T04:12:11.031Z
---

팀 캡스톤/과제용 전시회 프로젝트. [[digital_twin_project]]와 **같은 물리 Pi4**를 쓰지만 별도 코드베이스(`~/Exhibit/`)로 관리 중.

**팀 구조 (역할 분담)**
- 개발 A(데이터): `home_sim.py` — 고령자 집 낙상 감지 시뮬레이션, `home/alert` 발행
- 개발 B(로버, 본인 담당 추정): RC카 — `car/control` 구독해 실제 모터 구동, `car/telemetry`/`car/mode` 발행
- 개발 C(언리얼): 가상 구급차 조작 앱, QR 진입 — **아직 미개발** (2026-09-22 기준), 아키텍처 스펙은 확정됨(아래)
- 디자인 D(블렌더): 고령자 집 그레이박스 3D 모델
- 디자인 E(화면): 알림 수신 UI

**아키텍처 핵심 결정 (2026-09-22 확정) — "언리얼이 항상 주도, 실물은 항상 따라감"**
- 전시장 바닥엔 실제 차선/트랙이 없음 — 도로는 언리얼 가상 세계에만 존재. 그래서 실물 RC카는 스스로 인식/판단하지 않고, 모든 자율주행 판단(3단계 방문객 조작이든 5단계 자율복귀든)은 언리얼이 계산해서 `car/control`로 계속 발행 -> 실물은 그걸 그대로 받아 모터에 반영(미러링)만 함. 데이터 흐름 방향이 전 단계에서 일관됨.
- **자율주행 복귀 방식**: 언리얼이 방문객 조작 중 매틱(또는 0.1~0.2초 간격) `(steering, throttle)` 명령을 배열에 기록 -> 도착(`car/mode: arrived`) 후 배열을 뒤집고 부호 반전(`steer *= -1`, `throttle *= -1`)해서 같은 간격으로 재생 -> "왔던 길을 거꾸로" 감. Transform 기반 웨이포인트 추종보다 훨씬 단순해서 이 방식으로 하기로 함.
- 언리얼 UI 구상: 고정 탑다운 카메라(SpringArm, 회전 안 함, 차량이 화면 중앙 하단), `home/alert` 수신 시 화면 중앙 빨간 배너("위험 신호"), `car/mode:arrived` 수신 시 초록 배너("위험 해제")로 전환 후 페이드아웃, 우측 하단 D-pad(방문객 조작, 잠금은 alert~arrived 구간에만 활성화).
- 언리얼 쪽 미구현 4가지(다음에 개발 C가 할 일): ① 위험/해제 배너, ② D-pad, ③ 조작 기록+역재생, ④ 역재생 완료 시 `test/trigger/home` 발행(추후 정식 토픽으로 교체 예정).

**MQTT 설계 (최종본, paho-mqtt 2.x, localhost 브로커)**
- `home/alert` — 낙상 감지 시 1회만 (`{"type": "risk_start"}`) — 이벤트 전용
  - **주의**: 과제 원본 PDF 합격기준엔 "home/vitals 1초마다"가 명시돼 있었는데 팀 논의로 폐기함 — 담당 교수/조교 확인 필요할 수 있는 미해결 리스크로 남아있음
- `car/control` — {steering, throttle}, 언리얼이 발행(방문객 조작이든 자율주행 재생이든 동일 채널), RC카가 구독 — **MANUAL과 AUTONOMOUS 상태 둘 다 이 토픽을 받아서 반영함** (ARRIVED 전환 구간 1초만 무시)
- `car/telemetry` — RC카가 초당 2회 발행, 언리얼 동기화용 (`{"status": ..., "mode": ...}`, 위치 센서 없어 상태값만)
- `car/mode` — {mode: "arrived"|"autonomous"}, 상태 전환 시 1회씩
- `test/trigger/fall` — 테스트 대시보드 전용, `home_sim.py`가 구독. 사람이 누르는 트리거라 **매번 강제로 재발행**(force=True, 이전 상태 무관) — 실제 센서였다면 안 그랬겠지만 데모 버튼 특성상 이렇게 함
- `test/trigger/arrival` — 테스트 대시보드 전용, `car_subscriber.py`가 구독. MANUAL -> ARRIVED -> AUTONOMOUS 전환 트리거 (실제로는 초음파 등 센서로 교체 예정)
- `test/trigger/home` — 테스트 대시보드 전용, `car_subscriber.py`가 구독. AUTONOMOUS -> MANUAL 리셋 트리거 (실제로는 언리얼의 "가상 원위치 도착" 신호로 교체 예정)

**하드웨어/코드 현황 (2026-09-22 기준)**
- Pi4는 유동 IP라 세션마다 바뀜 (`10.75.214.100` → `10.245.228.100` → `10.124.84.100`, 계속 바뀔 수 있음). **mDNS 호스트명 `raspi4.local`로 고정 접속 가능** (avahi-daemon 활성 확인됨) — 단, 가끔 이 Windows PC에서 mDNS 해석이 실패할 때가 있어 안 되면 IP 직접 확인 필요.
- **systemd 서비스로 전환 완료** (재부팅해도 자동 시작, `Restart=on-failure`):
  - `exhibit-home.service` → `home_sim.py`
  - `exhibit-car.service` → `car_subscriber.py`
  - `exhibit-dashboard.service` → `dashboard_server.py` (테스트 대시보드, 포트 5050, `/ip` 엔드포인트로 현재 IP 표시)
  - **`digital-twin-car.service`는 disable(비활성화)해둠** — Exhibit과 같은 GPIO를 쓰는 motor_module이라 충돌함. `digital-twin-web`(웹/카메라)은 그대로 살아있음. **재부팅할 때마다 digital-twin-car가 다시 활성화되어 있진 않은지 확인할 것** (disable 했으니 정상이면 안 그럴 것).
- `~/Exhibit/car_subscriber.py`는 [[digital_twin_project]]의 `motor_module.py`(`pi-rover/ch09_project` 버전)를 재사용, `~/Digital_Twin/venv`를 그대로 빌려씀 — Exhibit 전용 venv는 여전히 안 만듦(문제 없음)
- **테스트 대시보드 "Rescue Relay Console"** (`~/Exhibit/dashboard.html` + `dashboard_server.py`): Pi4에서 평문 HTTP로 직접 서빙(`http://<pi-ip-or-raspi4.local>:5050`) — 처음엔 Claude Artifact(HTTPS)로 만들었다가 `ws://`가 mixed-content로 막히고 자체서명인증서 `wss://`도 mosquitto가 일반 GET에 응답 안 해서 브라우저 인증서 수락 UI 자체가 안 떠서 실패 → **Pi4 자체 HTTP 서빙으로 전환한 게 정답**이었음 (Digital_Twin 웹페이지와 같은 패턴). 5단계 파이프라인 시각화, MQTT 실시간 로그, 낙상/도착/복귀완료 테스트 트리거 버튼, 방문객 D-pad(잠금 연동) 전부 포함.
- 로컬 백업(개발 PC): `C:\Users\tmdal\Downloads\{car_subscriber.py, home_sim.py, dashboard.html, dashboard_server.py}` — 항상 Pi4에 재배포 후 systemd 재시작 필요 (`sudo systemctl restart exhibit-car` 등)
- **실제 하드웨어로 end-to-end 검증 완료**: 낙상 트리거 → home/alert 1회 → D-pad 조작(커브 회전 포함) → 도착 트리거 → arrived → autonomous → (지금은 언리얼이 아직 없어서) `car/control` 계속 수신 대기 상태로 둠 → `test/trigger/home` → MANUAL 리셋, 전체 사이클 정상 동작

**RC카 조작 세부**
- SPEED = 0.3 (계속 바뀜, 최근값)
- 좌/우회전은 제자리 회전이 아니라 **자연스러운 커브 회전**(`motor.move_curve_left/right`, `inner_ratio=0.6`)로 구현 — 안쪽 바퀴 속도를 `SPEED * inner_ratio`로 줄이는 방식
- steering/throttle을 4방향 이산 동작(forward/backward/left/right/stop)으로 매핑 중 — continuous 제어 아님

**해결된 버그/교훈**
- SSH로 백그라운드 프로세스 시작할 때 `pkill -f`와 `setsid ... &`를 **같은 커맨드 안에 같이 넣으면 안 됨** — pkill 패턴이 그 커맨드 자신의 텍스트(뒤에 나오는 setsid 대상 스크립트명)에 걸려서 자기 자신(부모 쉘)을 죽여버림. pkill과 setsid 시작은 **반드시 별도의 ssh 호출로 분리**.
- 대시보드에서 `disabled` 버튼도 브라우저가 `pointerdown`을 흘려보내는 경우가 있어, JS에서 `btn.disabled` 직접 체크 필요(CSS만으로는 클릭 자체를 못 막음).
- home_sim.py의 "정확히 1회만 발행" 래치는 테스트 버튼엔 부적합 — 사람이 누르는 명시적 트리거는 이전 상태 무관하게 항상 반응해야 함(force=True 패턴).

**아직 남은 것**
- `home_sim.py`(A파트)와 `car_subscriber.py`(B파트) 실제 통합은 검증됐지만, 도착 감지/자율복귀 종료는 여전히 테스트 토픽(`test/trigger/*`)으로 대체 중 — 언리얼이 실제로 이 신호들을 보내게 되면 테스트 토픽에서 정식 토픽으로 교체 필요
- 디자인 D(3D 방문객 방문), E(화면) 진행 상황 파악 안 됨

**How to apply**: 다음 세션에서 이어갈 때 먼저 `raspi4.local` 또는 최근 IP로 접속 확인 → `systemctl is-active exhibit-home exhibit-car exhibit-dashboard`로 서비스 상태 확인 → 대시보드(`:5050`)로 전체 흐름 테스트. 언리얼 쪽 작업이 시작되면 이 문서의 "아키텍처 핵심 결정" 섹션을 개발 C에게 그대로 브리핑하면 됨.
