# VR·IoT 해저 탐사 시스템

사용자가 Quest 3에서 가상 탐사선을 조작하며 해저 환경과 고래를 관찰하는 프로젝트입니다. 가상 탐사선의 조작값은 MQTT를 통해 Raspberry Pi로 전달되며, Pi는 고정된 RC카의 바퀴 제어와 웹 대시보드 운영을 담당합니다. 관람객과 운영자는 대시보드에서 Quest 영상, 연결 상태, 탐사 상태를 확인할 수 있습니다.

> 고래는 관찰 대상입니다. 실제 RC카는 가상 탐사선의 이동을 표현하는 장치이며, 실제 공간에서 가상 탐사선의 경로를 정확히 재현하는 기능은 범위에 포함하지 않습니다.

## 프로젝트 구성

```text
Quest 3 / Unreal Engine
  ├─ 가상 탐사선 조작 · 해저 탐사 · 이벤트 기반 자동복귀
  ├─ MQTT 주행 명령 ────────────────┐
  └─ H.264 관전자 영상 · 상태 ──────┼─> Raspberry Pi
                                     │    ├─ Mosquitto MQTT 브로커
웹 대시보드 <────────────────────────┤    ├─ 대시보드 · 영상 중계
                                     └──> └─ RC카 모터 수신기
```

| 구성 요소 | 역할 |
| --- | --- |
| Quest 3 · Unreal Engine | 해저 가상환경 렌더링, 탐사선 조작, 고래 관찰, 자동복귀 경로 계산 |
| MQTT | Quest/Unreal의 주행 명령과 Pi 수신기를 연결하는 메시지 통신 |
| Raspberry Pi | MQTT 브로커, 웹 대시보드, 관전자 영상 중계, RC카 수신기 실행 |
| RC카 | 가상 탐사선 조작에 연동되는 실제 바퀴 움직임 표현 |
| 웹 대시보드 | Quest 영상, Quest·MQTT·수신기 연결 상태, RC 송신 제어 표시 |

## 주요 기능

- 해저 맵에서 탐사선을 수동 조작하고 고래·해양 환경을 관찰합니다.
- 탐사 이벤트 발생 시 출발 지점으로 돌아가는 3D A* 기반 자동복귀를 제공합니다.
- Quest 조작값을 MQTT로 전송해 RC카 바퀴 움직임과 연동합니다.
- Pi 대시보드에서 Quest의 관전자 영상과 통신·수신 상태를 확인합니다.
- RC 명령 송신은 대시보드에서 사용자가 직접 켜야 하며, 연결이 끊기면 정지값을 보내도록 구성했습니다.

## 저장소 구조

| 경로 | 내용 |
| --- | --- |
| `MqttVehicle 5.7/` | 기준 Unreal Engine 5.7 프로젝트. 게임 로직, 맵, 입력, Android 패키징 설정 포함 |
| `MqttVehicle 5.7/Source/MqttVehicle/` | 탐사선, MQTT 브릿지, 자동복귀, Quest 대시보드 연동 C++ 코드 |
| `MqttVehicle 5.7/Config/` | MQTT 브로커, Pixel Streaming, Android/Quest 대시보드 주소 설정 |
| `dashboard/` | Pi 또는 Windows에서 실행하는 웹 대시보드, 영상 중계, RC 수신기 서비스 정의 |
| `tools/` | Unreal 에디터와 대시보드 연결, RC 세션 보조 도구 |
| `distribution/quest3/` | Quest APK 설치 도구와 배포 안내 |
| `materials/` | 초기 MQTT·Pi·Unreal 교육 자료, 에셋, 참고 코드 |
| `docs/` | 현재 상태, 개발 이력, 전체 파일 목록과 인계 문서 |

## 실행 개요

1. Pi와 Quest/PC를 같은 Wi-Fi에 연결합니다.
2. Pi에서 `dmix-dashboard`와 `dmix-pixel` 서비스를 실행합니다.
3. Quest에서 **D-Mix Underwater** 앱을 실행합니다.
4. 브라우저에서 Pi 대시보드를 열어 Quest·MQTT 연결 상태를 확인합니다.
5. RC카를 사용하려면 수신기를 시작한 뒤 대시보드에서 **RC 명령 송신 켜기**를 사용합니다.

실제 모터를 시험할 때는 바퀴를 공중에 띄우고 주변을 비운 상태에서 저속으로 확인합니다.

## 문서 안내

| 문서 | 용도 |
| --- | --- |
| [AI_HANDOFF.md](AI_HANDOFF.md) | 다른 PC 또는 다른 작업자가 이어서 개발하기 위한 현재 환경·작업 인계 |
| [docs/CURRENT_STATE.md](docs/CURRENT_STATE.md) | 구현·검증·미검증 사항과 다음 작업 |
| [docs/DEVELOPMENT_HISTORY.md](docs/DEVELOPMENT_HISTORY.md) | 날짜별 수정 사항, 배포·검증 기록 |
| [docs/FILE_GUIDE.md](docs/FILE_GUIDE.md) | 저장소 전체 파일의 역할과 보존 범위 |
| [distribution/quest3/README.md](distribution/quest3/README.md) | Quest APK 설치 안내 |

개발 과정의 생성형 AI 활용과 향후 확장 계획은 실제 실행 기능과 구분해 문서화합니다. 현재 자동복귀는 학습형 AI가 아니라 3D A* 경로 탐색으로 동작합니다.

## 공개 저장소에서 제외한 파일

이 공개 저장소에는 라이선스상 재배포할 수 없거나 출처·개인정보 문제가 있는 파일을 넣지 않았습니다. 문서에서 아래 경로를 언급하더라도 이 저장소에는 없습니다. Unreal 프로젝트를 그대로 열면 해당 메시·텍스처·기포 효과·물고기가 비어 보일 수 있으므로, 필요한 에셋은 각자 원본에서 취득해 같은 경로에 넣어야 합니다.

| 제외 경로 | 이유 |
| --- | --- |
| `materials/fab_underwater_world_portable/`, `materials/corals_fbx.zip` | Fab Standard 라이선스 에셋 (PackDev, DmitriyDryzhak) |
| `MqttVehicle 5.7/Content/Underwater/Meshes/`, `MqttVehicle 5.7/Content/Underwater/Textures/` | 위 Fab 에셋의 Unreal 변환본 |
| `MqttVehicle 5.7/Content/Vefects/` | Fab Standard 라이선스 에셋 (Vefects Bubbles VFX) |
| `materials/selected_free_assets/Wyreframe/`, `MqttVehicle 5.7/Content/Underwater/Selected/Tailor/` | CGTrader Royalty Free 에셋 (Tailor 물고기)과 변환본 |
| `MqttVehicle 5.7/Plugins/MQTT/`, `materials/mqtt_headers/` | Epic Games 엔진 플러그인 소스 (UE5 엔진의 실험적 MQTT 플러그인을 사용) |
| `MqttVehicle 5.7/Blender_OBJ.zip`, `MqttVehicle/Blender_OBJ.zip`, `MqttVehicle 5.7/Content/00_Works/`, `MqttVehicle/Content/00_Works/` | 출처 미확인 3D 에셋 |
| `materials/수업자료/`, `materials/references/ch12_mqtt.pdf` | 출처 미확인 교육 자료 |
| `output/`, 루트의 작품설명 판넬 양식(`.pptx`, `.hwp`) | 개인정보가 포함된 발표·제출 자료 |

CC BY 4.0·CC0 에셋은 출처 표기와 함께 포함했습니다. 자세한 내용은 [docs/ASSET_CREDITS.md](docs/ASSET_CREDITS.md)를 참고하세요.
