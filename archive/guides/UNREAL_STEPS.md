# Unreal 연결 단계 (Windows)

지금까지 완성된 것 위에 Unreal을 얹는 절차입니다. 로버는 이미 `veh/cmd` 를 구독하고 있으므로, **Unreal도 같은 토픽을 구독하면 웹 버튼 한 번에 가상 차량과 실물 로버가 동시에 움직입니다.**

```
휴대폰/웹 ──ws:9001──┐
                     ├─→ [Pi: mosquitto] ─┬─→ rover_drive.py → 실물 로버
Unreal(Windows) ─tcp:1883─┘                └─→ (Unreal 이 여기 붙는다)
```

---

## 0. 전제조건 — 사람이 해야 하는 것

| # | 항목 | 확인 |
| --- | --- | --- |
| 1 | **PC에서 Pi가 보이는지** | `ping 192.168.219.51` 이 응답해야 함. 이게 안 되면 뒤가 전부 막힙니다 |
| 2 | Epic Games 로그인 + **Unreal Engine 5.7** | Epic Games Launcher |
| 3 | **Visual Studio 2022** + "C++를 사용한 게임 개발" 워크로드 | 없으면 C++ 전환이 그 자리에서 실패 |
| 4 | Pi에서 `rover_drive.py` 와 브로커가 켜져 있음 | `ss -tlnp \| grep 1883` |

### 1번이 안 될 때

어제 `Destination Host Unreachable` 이 났던 문제입니다. 휴대폰은 붙는데 PC만 안 붙으면 **PC가 다른 네트워크로 나가 있는 것**입니다.

```bat
ipconfig
```

- 유선랜과 Wi-Fi를 동시에 쓰고 있지 않은지
- Wi-Fi IPv4가 `192.168.219.x` 대역인지 (다른 대역이면 다른 공유기에 붙은 것)
- 회사/학교망이면 Wi-Fi를 끊고 휴대폰 핫스팟에 **PC와 Pi를 같이** 붙이는 것이 가장 빠릅니다

---

## 1. 프로젝트 생성

Epic Games Launcher → **Games → Vehicle** 템플릿 → Blueprint → 프로젝트 이름 **`MqttVehicle`**

> 이름이 모듈명과 `MQTTVEHICLE_API` 매크로에 그대로 들어갑니다. 다르게 지으면 소스 3개를 전부 고쳐야 합니다.

▶ Play 해서 **키보드 WASD로 차가 움직이는지 먼저 확인하세요.** 여기서 안 되면 뒤는 무의미합니다.

## 2. 플러그인

Edit → Plugins

- `Chaos Vehicles Plugin` — Vehicle Template은 보통 이미 켜져 있음
- `MQTT` — **Experimental**이라 안 보이면 톱니바퀴에서 "Show Experimental Plugins" 켜기

실제 이름 확인:

```bat
dir /s /b "C:\Program Files\Epic Games\UE_5.7\Engine\Plugins\*MQTT*.uplugin"
```

> **MQTT 플러그인이 아예 없으면 여기서 멈추고 알려주세요.** 5.7에서 제거·개명됐을 수 있고, 그러면 구현 방식을 다시 정해야 합니다.

## 3. C++ 프로젝트로 전환

손으로 하지 말고 Editor가 만들게 하세요. `Target.cs`, 모듈 파일, `.uproject` 갱신이 한 번에 정확히 됩니다.

1. Tools → **New C++ Class** → Show All Classes → 부모 **None** → 이름 `Temp` → Create
2. 컴파일/전환 승인 창이 뜨면 승인
3. 끝나면 **Editor 완전 종료**

## 4. 소스 배치

```
<프로젝트>\Source\MqttVehicle\MqttVehicle.Build.cs                  ← 덮어쓰기
<프로젝트>\Source\MqttVehicle\MqttVehicleBridgeSubsystem.h          ← 새로 추가
<프로젝트>\Source\MqttVehicle\MqttVehicleBridgeSubsystem.cpp        ← 새로 추가
```

- 3단계에서 만든 `Temp.h` / `Temp.cpp` 는 삭제
- `MqttVehicle.h` / `.cpp` (모듈 구현부)와 `*.Target.cs` 는 건드리지 않음

## 5. 브로커 주소 설정 ★

`DefaultGame.ini.snippet` 의 내용을 프로젝트의 **`Config\DefaultGame.ini`** 맨 아래에 붙여넣으세요.

```ini
[/Script/MqttVehicle.MqttVehicleBridgeSubsystem]
BrokerHost=192.168.219.51
BrokerPort=1883
CmdTopic=veh/cmd
InputTimeout=0.3
ControlReleaseAfter=3.0
```

**`BrokerHost` 를 Pi의 실제 IP로 맞추세요.** Pi에서 `hostname -I` 로 확인한 값입니다.

> 이 값들은 **재빌드 없이** 바뀝니다. IP가 달라져도 ini 한 줄만 고치고 Play를 다시 하면 됩니다. 수업에서 조마다 Pi 주소가 다를 테니 이 구조가 필요합니다.

## 6. 빌드

1. `MqttVehicle.uproject` 우클릭 → **Generate Visual Studio project files**
2. `MqttVehicle.sln` → **Development Editor** / **Win64** → 빌드

빌드가 MQTT 심볼에서 깨지면 `.cpp` 상단의 "엔진 MQTT 플러그인 접점" 주석을 보세요. 확인할 4가지와 헤더 조회 명령이 있습니다.

---

## 7. 검증

순서대로 하나씩. 앞이 실패하면 뒤는 볼 필요 없습니다.

- [ ] **V-1** Pi에서 브로커와 `rover_drive.py` 가 켜져 있다
- [ ] **V-2** PC에서 `ping 192.168.219.51` 응답
- [ ] **V-3** Unreal ▶ Play → Output Log에 `MQTT vehicle bridge subscribed to veh/cmd`
- [ ] **V-4** 휴대폰/웹에서 `W` → **Unreal 차량과 실물 로버가 동시에 움직인다**
- [ ] **V-5** `W`+`D` → 둘 다 우측으로 휘어짐
- [ ] **V-6** 손 떼면 둘 다 정지
- [ ] **V-7** 브라우저 탭을 닫으면 0.3초 안에 둘 다 정지

V-4가 이 프로젝트의 목표입니다. 웹 버튼 하나로 가상과 실물이 같이 움직이는 순간이 디지털 트윈입니다.

---

## 막혔을 때

| 증상 | 확인 |
| --- | --- |
| V-3 로그가 안 뜸 | MQTT 플러그인 비활성 / 브로커 미실행 / **Editor 월드에서 실행** (반드시 ▶ Play) |
| V-3은 뜨는데 차가 안 움직임 | `CmdTopic` 이 `veh/cmd` 인지, Pi에서 `mosquitto_sub -h localhost -t 'veh/#' -v` 로 메시지가 흐르는지 |
| 로버는 움직이는데 Unreal만 안 움직임 | `BrokerHost` 오타. Output Log의 `connecting to ...` 줄에 찍힌 주소 확인 |
| Unreal은 움직이는데 로버만 안 움직임 | Pi에서 `rover_drive.py` 가 켜져 있는지, GPIO busy 아닌지 |
| `S` 를 눌러도 후진 안 함 | 차량 BP의 Transmission 설정. `SetTargetGear(-1)` 이 자동변속에 덮이면 알려주세요 |
| 빌드가 MQTT 심볼에서 실패 | `.cpp` 상단 주석의 확인 4가지 |

---

## 규약 요약 (로버·웹·Unreal 공통)

```
토픽    veh/cmd     {"th": -1..1, "st": -1..1}     조종 명령
        veh/state   {"l":.., "r":.., "live":bool}  로버 실제 상태

th = W - S      전후
st = D - A      좌우

발행 주기   100ms (하트비트, 키 변화와 무관하게 계속)
타임아웃    300ms  → 로버와 Unreal 모두 정지
```

변환은 웹에서 한 번만 합니다. 그 뒤로는 로버도 Unreal도 같은 숫자 두 개만 읽습니다.
