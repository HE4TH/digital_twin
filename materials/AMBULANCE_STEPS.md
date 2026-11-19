# 앰뷸런스 시나리오 — 실행과 Unreal 연동

집 안 센서(가상)가 이상을 감지하면 **경보**가 울리고, **사람이 휴대폰에서 출동을 승인**하면 실물 로버와 Unreal 앰뷸런스가 함께 출동합니다.

```
① 사람이 쓰러진다  (Unreal 안에서)
        │  home/trigger
        ↓
   home_sim.py  ── home/alert (경보) ──┐
                                       │
        ┌──────────────┬───────────────┤
        ↓              ↓               ↓
  rover_ambulance   Unreal          home.html
   출동 대기        경고 표시        🚨 출동할까요?
   (안 움직임)      사람 쓰러짐       [출동] 버튼
                                       │
② 사람이 [출동] 버튼을 누른다          │  veh/dispatch {"action":"go"}
        ┌──────────────┬───────────────┘
        ↓              ↓
  rover_ambulance   Unreal
   실제로 출발      앰뷸런스 이동 + 사이렌
```

**핵심 설계.** `home/alert` 는 "무슨 일이 일어났다"는 **경보**일 뿐, 출동 명령이 아닙니다. 출동은 `veh/dispatch` 라는 **별도의 지령**으로 분리되어 있고, 그 사이에 사람의 판단이 들어갑니다.

> 왜 이렇게 나눴는가 — 실제 119도 신고가 들어왔다고 구급차가 자동으로 출발하지 않습니다. 상황실이 판단해서 출동을 지시합니다.
> 센서는 틀릴 수 있습니다. 자동화의 마지막 한 칸을 사람에게 남겨두는 것은 타협이 아니라 설계입니다. **수업에서 학생에게 물어볼 지점입니다.**

---

## 1. 라즈베리파이

USB로 `pi\` 폴더의 파일을 옮긴 뒤:

```bash
cp /media/pi/ESD-USB/pi/home_sim.py        ~/
cp /media/pi/ESD-USB/pi/rover_ambulance.py ~/
cp /media/pi/ESD-USB/pi/home.html          ~/rover-web/
```

**`rover_drive.py` 는 이제 쓰지 않습니다.** `rover_ambulance.py` 가 그 기능을 전부 포함합니다.

> GPIO 는 한 프로세스만 점유할 수 있어서 조종과 출동을 두 프로그램으로 나눌 수 없습니다.
> 예전 것이 돌고 있으면 반드시 끄세요 — 안 그러면 `GPIO busy` 가 납니다.
> ```bash
> pkill -f rover_drive.py
> ```

### 실행 (터미널 3개)

```bash
# 터미널 A — 로버 (조종 + 출동)
python3 ~/rover_ambulance.py

# 터미널 B — 가상 스마트홈 센서
python3 ~/home_sim.py

# 터미널 C — 웹 서버
cd ~/rover-web && python3 -m http.server 5173
```

### 화면

| 주소 | 용도 |
| --- | --- |
| `http://<Pi IP>:5173/home.html` | **관제 화면(휴대폰)** — 거주자 상태, 출동 승인, 앰뷸런스 상태 |
| `http://<Pi IP>:5173/index.html` | 수동 조종 화면 |

> 휴대폰에서 열 때 브로커 주소는 `ws://<Pi IP>:9001` 이어야 합니다. `127.0.0.1` 로 두면 휴대폰 자기 자신을 찾다가 `connack timeout` 이 납니다.

---

## 2. 동작 확인 — 승인 흐름

관제 화면에서 **연결** 후 **낙상 발생** 버튼.

- [ ] 거주자 상태가 붉게 **낙상 감지**, 심박 105 이상
- [ ] 앰뷸런스 상태 **출동 대기** — **아직 움직이지 않는다**
- [ ] 출동 지령 패널이 붉게 점멸하며 **"🚨 앰뷸런스를 출동시킬까요?"**
- [ ] **🚑 출동** 버튼을 누르면 → **출동 중**, LEFT/RIGHT `+0.50`
- [ ] 3초 뒤 **현장 도착**, `+0.00`
- [ ] **↩ 복귀** → **복귀 중** → 3초 뒤 다시 **출동 대기**
- [ ] **정상 복귀**(상황 종료) → **대기**

가장 중요한 항목은 두 번째입니다. **경보가 왔는데 안 움직이는 것이 정상**입니다.

### 로버 모드

| 모드 | 뜻 | 움직임 |
| --- | --- | --- |
| `idle` | 대기 | 정지 |
| `standby` | 경보 수신, 승인 대기 | **정지** |
| `dispatched` | 출동 중 | 전진 |
| `arrived` | 현장 도착 | 정지 |
| `returning` | 복귀 중 | 후진 |
| `manual` | 사람이 조종 중 | 조종대로 |

### 사람 개입 확인

출동 중에 조종 화면(`index.html`)에서 `A` 를 누르면 **즉시 수동 조종으로 전환**되고 자동 시퀀스가 취소됩니다. 손을 떼면 `standby` 로 돌아갑니다.

응급 상황에서 사람이 개입하려는데 로봇이 자기 시퀀스를 고집하면 안 되기 때문입니다.

### 출동 시간 조정

트랙 길이에 맞춰 `rover_ambulance.py` 위쪽 상수만 고치면 됩니다.

```python
DISPATCH_SPEED   = 0.5    # 출동 속도
DISPATCH_SECONDS = 3.0    # 전진 시간
RETURN_SECONDS   = 3.0    # 복귀 시간
```

---

## 3. Unreal 연동

### C++ 은 통신만, 연출은 Blueprint

브릿지가 `home/alert` 와 `veh/dispatch` 를 구독해서 **각각 다른 Blueprint 이벤트로 넘겨줍니다.**

| 이벤트 | 언제 | 여기서 할 일 |
| --- | --- | --- |
| `On Home Alert` (Type, Room, Level) | 사람이 쓰러졌을 때 | **경고 UI만.** 앰뷸런스는 아직 움직이지 않는다 |
| `On Dispatch` (Action) | 휴대폰에서 승인했을 때 | **앰뷸런스 이동 + 사이렌** |

이 둘을 섞지 마세요. 섞으면 승인 단계가 사라지고 예전 자동 출동으로 되돌아갑니다.

**C++ 파일 3개를 프로젝트에 복사하고 빌드하세요.**

```powershell
$s="C:\Users\pnpau\Downloads\AI_HANDOFF_MQTT_VEHICLE\AI_HANDOFF_MQTT_VEHICLE\unreal\Source\MqttVehicle"
$d="C:\Unreal\MqttVehicle\Source\MqttVehicle"
Copy-Item "$s\*" $d -Force
Get-ChildItem $d -Filter *.cs,*.h,*.cpp | ForEach-Object { $_.LastWriteTime = Get-Date }
Remove-Item "C:\Unreal\MqttVehicle\Intermediate\Build\BuildRules" -Recurse -Force -ErrorAction SilentlyContinue
Write-Host "OK"
```

> **복사 후 날짜 갱신은 필수입니다.** `Copy-Item` 은 원본 날짜를 유지해서, 빌드 시스템이 "안 바뀌었다"고 판단하고 예전 규칙을 씁니다.

`Config\DefaultGame.ini` 에 `DispatchTopic=veh/dispatch` 한 줄이 추가됐습니다 (`DefaultGame.ini.snippet` 참고). 없어도 기본값이 같아서 동작하지만, 적어두면 학생이 토픽 구조를 눈으로 봅니다.

빌드 후 Play 하면 Output Log 에:

```
MQTT vehicle bridge subscribed to veh/cmd, home/alert, veh/dispatch
```

세 개가 다 나와야 합니다.

---

### ① 사람이 쓰러진다 → 경고

레벨 블루프린트:

```
Event BeginPlay
   └─ Get World Subsystem (MqttVehicleBridgeSubsystem)  →  변수 Bridge 로 저장
         └─ Bind Event to On Home Alert   → 커스텀 이벤트 HandleAlert
         └─ Bind Event to On Dispatch     → 커스텀 이벤트 HandleDispatch
```

> 실행 핀은 **하나에서 하나로만** 이어집니다. 두 개를 잇고 싶으면 `Sequence` 노드를 쓰세요.

```
HandleAlert (AlertType, Room, Level)
   └─ Branch (Level >= 1)
        True  → 사람 액터를 쓰러진 자세로  (Set Actor Rotation / 애니메이션)
              → 경고 위젯 Add to Viewport   (붉은 "낙상 감지 — 거실")
              → 경고음 Play Sound 2D (짧게)
        False → 위젯 Remove from Parent, 사람 원위치
```

쓰러지는 연출을 아직 안 만들었다면 `Set Actor Rotation` 으로 Pitch 90도만 줘도 충분합니다. Timeline 으로 0 → 90 을 0.4초에 보간하면 훨씬 낫습니다.

> **액터가 `Movable` 이어야 합니다.** `Static` 이면 `Set Actor Location/Rotation` 이 조용히 무시됩니다.

### ② 승인 → 출동 + 사이렌

```
HandleDispatch (Action)
   └─ Switch on String (Action)
        "go"     → Sequence
                     ├─ 사이렌 Spawn Sound Attached (앰뷸런스에 붙임, Looping)
                     │     반환값을 변수 SirenComp 에 저장
                     └─ Timeline  Play        (앰뷸런스 이동)
        "return" → Sequence
                     ├─ SirenComp → Stop
                     └─ Timeline  Reverse
        "cancel" → SirenComp → Stop, Timeline Stop
```

**사이렌은 `Spawn Sound Attached` 로 만드세요.** `Play Sound at Location` 은 한 번 재생하면 끌 수가 없고, 앰뷸런스를 따라 움직이지도 않습니다. `Spawn Sound Attached` 는 컴포넌트를 돌려주므로 나중에 `Stop` 할 수 있고 소리가 차를 따라다닙니다.

사운드 애셋은 엔진 기본에 없으므로 하나 준비해야 합니다.

1. 무료 사이렌 `.wav` 를 받아 콘텐츠 브라우저에 드래그
2. 임포트된 Sound Wave 우클릭 → **Create Cue**
3. Cue 를 열어 **Looping** 체크
4. 이 Cue 를 `Spawn Sound Attached` 에 넣는다

Looping 을 안 켜면 3초짜리 사이렌이 한 번 울리고 조용해집니다.

### ③ 앰뷸런스 이동 (Timeline)

1. 레벨에 **Spline Actor** 배치 — 도로 밖에서 집 앞까지 경로
2. 앰뷸런스 액터에 **Timeline** 추가 (0 → 1, 재생 시간 = `DISPATCH_SECONDS` = 3.0초)
3. Timeline 값 × Spline 길이 → `Get Location at Distance Along Spline` → `Set Actor Location`
4. 방향까지 맞추려면 `Get Rotation at Distance Along Spline` → `Set Actor Rotation`

Timeline 길이를 로버의 `DISPATCH_SECONDS` 와 맞추면 **실물과 화면이 같은 타이밍에 도착합니다.** 시연에서 이 장면이 제일 잘 먹힙니다.

### ④ Unreal 에서 상황을 발생시키기

`PublishHomeTrigger("fall", "living")` 을 호출하면 Unreal 이 **원인**이 됩니다. 키 입력이나 트리거 볼륨에 물려두세요.

```
Event Key F  →  Bridge → Publish Home Trigger (TriggerType="fall", Room="living")
Event Key G  →  Bridge → Publish Home Trigger (TriggerType="clear", Room="living")
```

> Play 로 실행해야 키 입력이 먹습니다. **Simulate 는 PlayerController 가 없어서 안 됩니다.** 그리고 뷰포트를 한 번 클릭해 포커스를 줘야 합니다.

휴대폰 없이 Unreal 안에서 출동까지 승인하고 싶으면 `PublishDispatch("go")` 도 있습니다. 다만 **시연에서는 휴대폰으로 누르는 편이 훨씬 설득력 있습니다** — 사람의 판단이 시스템 밖에 있다는 게 눈에 보이니까요.

---

## 4. 검증 — 전체 연동

- [ ] Pi에서 세 프로그램 실행
- [ ] Unreal ▶ Play → Output Log 에 `subscribed to veh/cmd, home/alert, veh/dispatch`
- [ ] Unreal 에서 `F` → Pi 터미널 B 에 `ALERT -> fall`
- [ ] Output Log 에 `HOME ALERT: fall @ living (level 2)` + 화면에 경고
- [ ] **로버는 아직 안 움직인다** (`mode -> standby`)
- [ ] 휴대폰 관제 화면이 붉게 점멸, **🚑 출동** 버튼 활성화
- [ ] 출동 누름 → Output Log 에 `DISPATCH: go`
- [ ] **실물 로버가 출동하고, 화면 속 앰뷸런스가 사이렌을 울리며 움직인다**
- [ ] **↩ 복귀** → 둘 다 되돌아오고 사이렌이 멈춘다

---

## 시연 대본 (4분)

| 시간 | 하는 일 | 말할 것 |
| --- | --- | --- |
| 0:00 | 관제 화면 연결, 평상시 상태 | "집 안 센서가 1초마다 상태를 보냅니다" |
| 0:40 | Unreal 에서 `F` — 사람이 쓰러짐 | "가상 세계에서 사건이 일어났습니다" |
| 1:00 | 휴대폰이 붉게 점멸 | "경보가 왔습니다. **그런데 구급차는 안 나갑니다**" |
| 1:20 | — | "왜 자동으로 안 보낼까요?" ← **여기서 멈추고 물어보기** |
| 2:00 | 🚑 출동 누름 | "사람이 판단해서 승인합니다" |
| 2:10 | 로버 + 화면 동시 출발 | "같은 메시지 하나를 셋이 받았습니다" |
| 3:00 | 복귀 | "브로커는 구독자가 몇 개인지 모릅니다" |

---

## 통신 규약 (최종)

```
home/vitals    1초마다   {"state":"normal|no_motion|fall","level":0~2,
                          "room":"living","hr":68,"motion":true,"held":12.0}
home/alert     이벤트     {"type":"fall","room":"living","level":2}
home/trigger   상황 발생  {"type":"fall"|"no_motion"|"clear","room":"living"}

veh/dispatch   이벤트     {"action":"go"|"return"|"cancel"}     사람의 출동 승인
veh/cmd        100ms     {"th":-1..1,"st":-1..1}               사람의 수동 조종
veh/state      200ms     {"mode":"idle|standby|dispatched|arrived|returning|manual",
                          "alert":0~2,"l":..,"r":..,"live":bool}

QoS 0, retain false
수동 조종 타임아웃 0.3초 — 명령이 끊기면 로버와 Unreal 모두 정지
```

**상태는 주기 발행, 사건은 이벤트 발행.** 늦게 접속한 구독자도 `vitals` 와 `state` 로 현재 상황을 알 수 있고, 즉시 반응이 필요한 쪽은 `alert` / `dispatch` 를 씁니다.

**그리고 경보와 지령은 다른 토픽입니다.** `home/alert` 는 센서가 관측한 사실이고, `veh/dispatch` 는 사람이 내린 결정입니다. 하나로 합치면 편하지만, 합치는 순간 누가 결정했는지가 사라집니다.
