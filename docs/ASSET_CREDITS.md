# 선택한 무료 해저 에셋과 출처

2026-10-16 사용자 선택. 다운로드·임포트·실행 검증은 서로 구분한다.
CC BY 4.0 에셋은 제작자·원본 링크·라이선스와 변경 사항을 함께 표시한다.

| 대상 | 제작자 / 원본 | 라이선스 | 현재 확보 상태 |
| --- | --- | --- | --- |
| 선택 탐사선 Submarine | [IrisProcess](https://sketchfab.com/3d-models/submarine-e55fa485c92949818b1f10adf1fc3c73) | [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/) | 공식 GLB 2K → 길이 4.8m 정적 메시, 1번 선택 외형 |
| 기본 탐사선 Spherical Submersible | [Lanzaman](https://sketchfab.com/3d-models/spherical-submersible-55ed2cc102d241aa8a6d57c6fedfbbdd) | CC BY 4.0 | 공식 GLB 2K → 좌표계·원점·크기 정규화, 위 모니터/지지대 제거(원본 보존), 기본 외형 및 2번 선택 |
| 해초 Sea Weed | [rkuhlf](https://sketchfab.com/3d-models/sea-weed-d97c596b189845b681cc8eef8a3ab429) | CC BY 4.0 | 공식 GLB → 원본 식물 형상 3종의 정적 메시와 UE 흔들림 재질, 871개 인스턴스 |
| 바위 Boulder 01 | [Rico Cilliers / Poly Haven](https://polyhaven.com/a/boulder_01) | [CC0](https://polyhaven.com/license) | 공식 FBX와 2K Diffuse·Normal DX·Roughness, 제공 MD5 확인; UE 변환 후 177개 배치 |
| 기포 Bubbles VFX | [Vefects / Fab](https://www.fab.com/listings/73d1c6cb-d6a5-4c3f-9381-94d4988ab3e6) | Fab Standard, Personal 무료 / Professional 유료 표시 | 사용자가 라이브러리에 취득, Unreal Fab에서 임포트; Area Translucent Niagara 시스템 24곳 배치 |
| 고래 Humpback Whale (Swimming) | [Connlan_Immure](https://sketchfab.com/3d-models/humpback-whale-swimming-f4912be9163a4f45b2480df8ccb8b2c6) | CC BY 4.0 | 공식 FBX·텍스처 → 뼈대 메시, Idle_Swim 9.125초 반복 애니메이션, 길이 약 33m(2026-10-17 체험 후 사용자 요청으로 직전 16.5m의 2배 확대) |

원본은 `assets/selected_free_assets/`, Unreal 변환본은 기준 프로젝트의
`Content/Underwater/Selected/`에서 관리한다. Vefects는 제공 패키지 경로
`Content/Vefects/Bubbles_VFX/`를 유지한다. Blender 변환본은 `Converted/`에 있다.
원본을 보존하고 메시 병합·크기·원점·좌표계·PBR 재질과 유리 표현을 Unreal에 맞춰 변환했다.
해초는 원본 Sway 애니메이션 대신 HISM과 재질의 절차적 흔들림을 사용한다.
고래는 원본 Idle_Swim을 반복하고 별도 C++가 반경 8m 원을 55초에 한 바퀴 시계 방향으로 이동한다. 원본 머리(-X)가 이동 방향을 향하도록 수정했다.
제작자는 이 프로젝트를 보증하거나 제휴한 것이 아니다. 저장·재개방·실행 검증은 현재 상태 문서를 참고한다.
Fab 원본은 독립 에셋으로 재배포하지 않으며 프로젝트 협업자 공유 조건을 따른다.

2026-10-17 최신: Lanzaman을 기본 외형으로 사용한다. 사용자 사진과 일치하는 원본 GLB/변환 SM_Lanza의 좌석 2개·아래 모니터 1개·창틀·조종 장치를 1인칭에도 표시하고 왼쪽 좌석에 카메라를 놓았다. 전방 시야 확보 요청에 따라 변환 FBX와 SM_Lanza에서 위 모니터 및 연결 지지대만 제거했다. 원본 GLB와 텍스처는 보존하며 나머지 내부 디자인은 제작자의 모델을 사용한다. 프로젝트의 SubmersibleViewComponent는 실내 조명을 추가하며 모니터의 기존 화면 그래픽을 그대로 사용한다. 기존 엔진 기본 도형·프로젝트 재질로 구성한 별도 조종석은 IrisProcess 선택에만 사용한다. 반대 시점 표시용 M_CameraFeed는 프로젝트 제작 재질이다. 추가 시야 요청으로 카메라를 50cm 높였다. UE 변환 유리 M_Lanza_Glass는 완전 투명 적용 후 사용자 후속 피드백에 따라 불투명도 0.08·옅은 청록색·비조명으로 조정했다. 원본 GLB/텍스처는 변경하지 않았다.

## 2026-10-18 추진기 시각 효과

Lanzaman 원본 메시를 수정하지 않고 추진기 뒤에 엔진 기본 Cube로 만든 3엽 회전 날개를 각각 추가한다. 위치는 원본 추진기 후면 경계에서 측정했다. 기포는 엔진 기본 Sphere와 프로젝트 생성 재질 `M_ThrusterBubble`을 사용하는 로컬 시각 효과다. 기존 Vefects 환경 기포와 별개이며 새 외부 다운로드/라이선스는 없다. `unreal/tools/build_thruster_effects.py`로 재질을 재생성한다. 원본 제작자/CC BY 표기는 유지한다.


## Tailor 물고기 — 2026-10-19

제작자 Wyreframe, [CGTrader 원본](https://www.cgtrader.com/free-3d-models/animal/fish/tailor-3a24be50-e51e-4016-8da7-61bd552f2485). 상품 표기 Royalty Free License (no AI), 무료 다운로드. 사용자 제공 원본을 프로젝트 협업용 비공개 저장소에 보관하며 독립 에셋 배포/판매용이 아니다. 변환: LOD0 FBX 뼈대 및 Swim_01, PBR 재질, 크기 45~60cm와 4무리 순환 경로. 원본 4개 FBX와 5개 PNG 보존. 실제 렌더/VR 성능 검증은 미완료.

2026-10-19 표시 수정: 원본 객체 변환 키 제거·객체 회전/축척 적용·cm 단위 변환으로 Prepared/TailorNormalized.fbx를 생성. 원본 Swim_01 포즈와 출처/라이선스를 유지하며 MeshV2에 적용했다. 실제 Play의 표시/이동 렌더 2장을 확인했다.
