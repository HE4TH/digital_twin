param(
 [string]$RepositoryRoot = (Split-Path -Parent $PSScriptRoot),
 [string]$BaselineDate = (Get-Date -Format 'yyyy-MM-dd')
)
$ErrorActionPreference = 'Stop'
$taskRoot = (Resolve-Path -LiteralPath $RepositoryRoot).Path.TrimEnd('\')
$taskExcluded = '\\(\.git|\.github-upload|\.upload-tools|\.panel_build|\.runtime|Binaries|Intermediate|Saved|DerivedDataCache|\.vs|node_modules|__pycache__|venv|\.venv)\\'
$taskSelfOutputs = @('docs/FILE_GUIDE.md','docs/FILE_MANIFEST.json')
$taskSpecific = @{
 'MqttVehicle 5.7/Content/Underwater/Selected/Materials/M_Lanza_Glass.uasset'='Lanzaman 전방 유리: 불투명도 8%·옅은 청록색으로 시야와 창 존재감을 유지'
 'MqttVehicle 5.7/Content/Underwater/Selected/Materials/M_CameraFeed.uasset'='SceneCapture 반대 시점 영상을 UMG 박스에 표시하는 UI 재질'
 'MqttVehicle 5.7/Source/MqttVehicle/Camera/SubmersibleViewWidget.cpp'='좌하단 512×288 반대 시점 영상 박스·24 여백·작은 글씨 제거'
 'MqttVehicle 5.7/Source/MqttVehicle/Camera/SubmersibleViewWidget.h'='좌하단 무자막 반대 시점 영상 위젯 선언'
 'MqttVehicle 5.7/Source/MqttVehicle/Camera/SubmersibleViewComponent.cpp'='Lanzaman 원본 내부·좌석 시점 조명·Iris 별도 조종석과 반대 시점 캡처'
 'MqttVehicle 5.7/Source/MqttVehicle/Camera/SubmersibleViewComponent.h'='런타임 조종석·시점별 숨김·반대 카메라 영상 컴포넌트 선언'
 'MqttVehicle 5.7/Source/MqttVehicle/MqttVehicleUI.cpp'='정확한 MPH 환산·숫자와 단위 문구 동기화·기어 표시'
 'MqttVehicle 5.7/Content/VehicleTemplate/Blueprints/UI/BP_VehicleAdvUI.uasset'='MPH 계산을 사용하는 탐사차 속도·기어 HUD Blueprint'
 'MqttVehicle 5.7/MqttVehicle.uproject'='기준 Unreal 5.7 프로젝트 진입·차량/MQTT 플러그인 선언; EngineAssociation 5.7'
 'docs/ASSET_CREDITS.md'='선택 무료 에셋 제작자·원본 링크·라이선스·변환 및 배치 내역'
 'MqttVehicle 5.7/Source/MqttVehicle/Observation/WhaleSwimmingActor.h'='관찰용 혹등고래 뼈대 메시·반복 애니메이션·원 궤적 설정 선언'
 'MqttVehicle 5.7/Source/MqttVehicle/Observation/WhaleSwimmingActor.cpp'='머리 방향으로 전진하는 시계 방향 고래 유영과 반경 8m·55초 원형 이동'
 'tools/prepare_selected_assets.py'='원본 GLB를 탐사선·해초 FBX/PBR로 변환하고 Lanzaman 위 모니터·지지대 제거'
 'tools/build_selected_underwater.py'='선택 에셋·Lanzaman 8% 청록 유리/높인 시점·Q/E 수중 이동·수동 6/자동복귀 5 MPH·고래 33m 재현'
 'tools/animate_underwater_scene.py'='에디터에서 옛 고래 WPO 재질과 위험 구역의 발광 해파리 3마리 제작'
 'MqttVehicle 5.7/Content/Underwater/Materials/M_DangerJelly.uasset'='위험 구역 해파리의 주황 발광과 절차적 움직임 재질'
 'README.md'='저장소 시작 안내, 최신 요구사항·작업 규칙, 복원·문서 읽기 순서와 보존 정책'
 'docs/DEVELOPMENT_HISTORY.md'='날짜별 설정 변경·배포·점검 결과를 보관하는 개발 이력; 프로젝트 개요는 README 참조'
 'AGENTS.md'='AI 작업 규칙: 백그라운드 처리·사용자 직접 Unreal 콘솔 입력·문서 갱신·업로드 승인·실차 금지'
 'AI_HANDOFF.md'='다른 PC·AI의 필수 인계: 확정 목표, 환경 복원, 현재 코드, 다음 수정·검증 순서'
 'docs/CURRENT_STATE.md'='조종석·두 시점·제자리 회전·수동 6/자동복귀 5 MPH 현재 구현과 검증 한계'
 'MqttVehicle 5.7/Source/MqttVehicle/MqttVehiclePawn.cpp'='수중 조작·충돌·공유 카메라·프로펠러/기포 컴포넌트 연결'
 'MqttVehicle 5.7/Source/MqttVehicle/MqttVehiclePawn.h'='수중 탐사선·공유 카메라·추진 시각 효과 컴포넌트 선언'
 'MqttVehicle 5.7/Source/MqttVehicle/MqttVehiclePlayerController.h'='Lanzaman 조종·메인 시점 숨김·관찰/복귀·정지 API 선언'
 'MqttVehicle 5.7/Source/MqttVehicle/MqttVehiclePlayerController.cpp'='Lanzaman 고정·1/2 선택 키 제거·속도기어 숨김·H/X/M 입력'
 'MqttVehicle 5.7/Source/MqttVehicle/Effects/SubmersiblePropulsionComponent.h'='로컬 추진기 회전·기포 풀과 이동 기반 시각 효과 선언'
 'MqttVehicle 5.7/Source/MqttVehicle/Effects/SubmersiblePropulsionComponent.cpp'='양쪽 회전 날개·월드 공간 상승 기포·정지 감쇠와 128개 풀'
 'MqttVehicle 5.7/Content/Underwater/Materials/M_ThrusterBubble.uasset'='추진기 기포의 투명 표면·밝은 가장자리 재질'
 'tools/build_thruster_effects.py'='Unreal 추진기 기포 재질 재생성 스크립트'
 'MqttVehicle 5.7/Source/MqttVehicle/Exploration/UnderwaterPathPlanner.h'='충돌 콜백·탐색 범위·증분 3D A* 경로 계획 API'
 'MqttVehicle 5.7/Source/MqttVehicle/Exploration/UnderwaterPathPlanner.cpp'='26방향 3D A* 최단 격자 탐색과 충돌 없는 경로 단축'
 'MqttVehicle 5.7/Source/MqttVehicle/Exploration/ExplorationRoute.h'='3차원 경로 기록·제자리 회전·2D/3D 선분 거리와 추종 계산 선언'
 'MqttVehicle 5.7/Source/MqttVehicle/Exploration/ExplorationRoute.cpp'='수직 경로 거리·높이 이탈·선체 회전 공간과 기존 차량 추종 계산'
 'MqttVehicle 5.7/Source/MqttVehicle/Exploration/ExplorationReturnComponent.h'='A* 복귀·도착 회전 상태와 위험 구역 만남 일련번호 선언'
 'MqttVehicle 5.7/Source/MqttVehicle/Exploration/ExplorationReturnComponent.cpp'='A* 복귀·도착 회전·위험 구역 진입/빠른 통과 알림 이벤트'
 'MqttVehicle 5.7/Source/MqttVehicle/Exploration/ExplorationDangerZone.h'='고정 위험 박스의 진입·퇴장 여유·연속 표본과 반복 이벤트 방지 상태 선언'
 'MqttVehicle 5.7/Source/MqttVehicle/Exploration/ExplorationDangerZone.cpp'='로컬 박스 내부·선분 통과·1m 퇴장 여유와 좌표 오류·점프 판정; 차량/네트워크 접근 없음'
 'MqttVehicle 5.7/Source/MqttVehicle/Tests/ExplorationDangerZoneTests.cpp'='위험 구역 경계·재진입·빠른 통과·높이·좌표 오류 자동화 검사'
 'MqttVehicle 5.7/Source/MqttVehicle/Tests/ExplorationReturnTests.cpp'='A* 복귀·실제 맵·도착 반회전 각도와 위치 유지 회귀 검사'
 'MqttVehicle 5.7/Source/MqttVehicle/Observation/WhaleObservationComponent.h'='관찰 영역 설정·최초 발견·출입 이벤트·UI 스냅샷과 세션 발견 상태 선언'
 'MqttVehicle 5.7/Source/MqttVehicle/Observation/WhaleObservationComponent.cpp'='관찰 패드 6m/7.5m·수중 높이 33m·최초 발견/재방문 판정'
 'MqttVehicle 5.7/Source/MqttVehicle/Observation/WhaleObservationWidget.h'='독립적인 고래·해파리 알림 수명과 위젯 상태 선언'
 'MqttVehicle 5.7/Source/MqttVehicle/Observation/WhaleObservationWidget.cpp'='우상단 고래·해파리 5초 알림과 우하단 H/M/Tab 안내'
 'MqttVehicle 5.7/Source/MqttVehicle/Tests/WhaleObservationTests.cpp'='관찰 판정·유영 방향·알림 재진입·중복 방지 검사'
 'MqttVehicle 5.7/Source/MqttVehicle/MqttVehicleBridgeSubsystem.cpp'='송수신 꺼짐 시 브로커 접속 생략·수동 MQTT 발행·재접속·단절 정지'
 'MqttVehicle 5.7/Source/MqttVehicle/MqttVehicleBridgeSubsystem.h'='차량 MQTT 송수신·연결 관리 API와 동작하지 않는 옛 경보 Blueprint 호환 심벌'
 'MqttVehicle 5.7/Source/MqttVehicle/Tests/MqttVehicleControlTests.cpp'='수동 입력·공유 카메라·추진기 회전과 기포 생성/소멸 검사'
 'MqttVehicle 5.7/Config/DefaultGame.ini'='Pi 주소와 veh/cmd th/st·100ms 설정; 송신·레거시 수신 꺼짐, 응급 설정은 미사용 호환 항목'
 'MqttVehicle 5.7/Config/DefaultEngine.ini'='해저 맵 L_UnderwaterExploration을 에디터·게임 시작 맵으로 지정한 엔진 설정; 공유 사본 AFS 인증 토큰은 비움'
 '.gitignore'='Unreal 캐시·빌드 산출물·인증정보·임시파일의 버전 관리 제외 규칙'
 '.gitattributes'='대용량 바이너리 Git LFS 관리와 원본 바이트·줄바꿈 보존 규칙'
 'tools/build_file_catalog.ps1'='모든 업로드 파일의 설명·크기·SHA-256 목록을 재생성하는 PowerShell 도구'
 'tools/build_underwater_scene.py'='UE 에디터 해저 맵 제작·위험 구역/외형·조명·경로 표지만 갱신; 열린 맵 적용과 탐사차 연결, Play 중 제작 거부'
 'tools/build_exploration_rover.py'='스포츠카 물리·바퀴·입력을 보존한 탐사차 외형·카메라·재질·게임모드 제작; 기존 에셋 재사용·명시적 외형 갱신'
 'MqttVehicle 5.7/Content/Underwater/Vehicles/BP_ExplorationRover.uasset'='Lanzaman 기본·Q/E 수중 이동·6/5 MPH·50cm 높인 조종석 카메라와 반대 영상 연결'
 'MqttVehicle 5.7/Content/Underwater/Vehicles/BP_UnderwaterGameMode.uasset'='해저 맵의 기본 Pawn을 BP_ExplorationRover로 지정하고 기존 컨트롤러·UI 유지'
 'MqttVehicle 5.7/Content/Underwater/Materials/M_RoverHullYellow.uasset'='탐사차 압력 선체·측면 탱크·상단 장비의 노란 금속 재질'
 'MqttVehicle 5.7/Content/Underwater/Materials/M_RoverWindow.uasset'='기존 장식 창 및 런타임 조종석 벽·계기판의 짙은 청색 재질'
 'MqttVehicle 5.7/Content/Underwater/Maps/L_UnderwaterExploration.umap'='33m 혹등고래·탐사선·해초/바위·기포와 충돌 없는 동선 표지의 374개 액터 해저 맵'
 'digital_twin/car_subscriber.py'='보존용 전시회 RC카 car/control 수신·telemetry·도착/자율복귀 상태; 현재 탐사 실행 대상 아님'
 'digital_twin/motor_module.py'='gpiozero Motor/Robot 기반 4개 모터 핀·전후진·곡선·회전·정지 저수준 함수'
 'digital_twin/home_sim.py'='보존용 risk_start 낙상 이벤트·autonomous 경보 초기화; 현재 탐사 실행 대상 아님'
 'digital_twin/dashboard.html'='보존용 응급상황 웹 UI: 조작 잠금·도착/복귀·car/control 발행; 현재 탐사 실행 대상 아님'
 'digital_twin/dashboard_server.py'='Flask 대시보드 HTTP 서버(5050), 현재 Pi IP 조회 /ip 경로'
 'materials/pi/rover_drive.py'='veh/cmd 입력 검증·0.3초 워치독·연결/종료 정지·기본 dry-run 수신기; Pi 미배포'
 'materials/pi/tests/test_rover_drive.py'='모터·브로커 없는 입력 오류·워치독·재접속·종료 정리 13개 오프라인 테스트'
 'materials/pi/rover_ambulance.py'='veh/cmd 수동 조종과 승인 출동·시간 기반 도착·복귀를 합친 Pi 코드; 응급 기능 제거 후보'
 'materials/pi/home_sim.py'='home/vitals 주기 발행과 home/alert 사건·home/trigger 처리'
 'materials/pi/index.html'='veh/cmd 수동 웹 조종, 100ms 하트비트, veh/state 표시'
 'materials/pi/home.html'='스마트홈 상태·출동/복귀 승인과 veh/state를 표시하는 기존 관제 UI'
 'materials/pi/mosquitto-veh.conf'='Pi Mosquitto TCP 1883·WebSocket 9001 리스너 설정'
 'materials/web-controller/index.html'='구형 testvi 웹 실습: keys 송신과 Unreal 키 상태 수신; 현재 RC카 규약과 다름'
 'materials/web-controller/mosquitto.windows.conf'='구형 Windows 실습 브로커의 TCP 1883·WebSocket 9001 설정'
 'materials/web-controller/start-broker.bat'='Windows 실습 Mosquitto 시작 스크립트; 실행 전에 설치 경로와 서비스 상태 확인'
 'materials/web-controller/start-web.bat'='Windows 실습 웹 서버 시작·IP 안내 스크립트'
 'materials/web-controller/package.json'='웹 컨트롤러 npm 의존성 선언'
 'materials/corals_fbx.zip'='제3자 Corals FBX·텍스처 원본 ZIP; 협업용 비공개 자료'
 'materials/whale_original/make_whale.py'='Blender에서 직접 제작한 고래 모델을 생성·내보내는 원본 스크립트'
 'materials/whale_original/render_preview.py'='Blender 고래 원본의 미리보기 렌더 스크립트'
}
$taskCodeRoles = @{
 'MqttVehicleBridgeSubsystem'='WorldSubsystem MQTT 연결·구독·JSON 수신·범용 발행과 차량 제어'
 'MqttVehiclePawn'='차량 공통 Enhanced Input·Chaos 입력·전후방 카메라·전복 자세 복구'
 'MqttVehiclePlayerController'='입력 매핑·터치 UI·속도/기어 갱신·차량 재생성'
 'MqttVehicleGameMode'='기본 차량 GameMode와 PlayerController 클래스 설정'
 'MqttVehicleUI'='속도 단위 변환·기어 표시를 Blueprint UI로 전달'
 'MqttVehicleSportsCar'='스포츠카 바퀴·엔진·변속기·조향·차체 물리 설정'
 'MqttVehicleOffroadCar'='오프로드 차량 바퀴·구동·서스펜션·차체 설정'
 'MqttVehicleWheelFront'='공통 전륜 바퀴·조향 설정'
 'MqttVehicleWheelRear'='공통 후륜 바퀴 설정'
 'MqttVehicleSportsWheelFront'='스포츠카 전륜 바퀴 물리 설정'
 'MqttVehicleSportsWheelRear'='스포츠카 후륜 바퀴 물리 설정'
 'MqttVehicleOffroadWheelFront'='오프로드 전륜 바퀴 물리 설정'
 'MqttVehicleOffroadWheelRear'='오프로드 후륜 바퀴 물리 설정'
 'OffroadGameMode'='오프로드 변형 GameMode'
 'TimeTrialGameMode'='타임트라이얼 레이스·결승선·랩 설정'
 'TimeTrialPlayerController'='타임트라이얼 시작·목표 게이트·랩·차량 UI 관리'
 'TimeTrialTrackGate'='트랙 게이트 Overlap·다음 게이트·결승선 랩 처리'
 'TimeTrialUI'='타임트라이얼 랩·시간 UI'
 'TimeTrialStartUI'='타임트라이얼 시작 위젯과 시작 이벤트'
}
function Get-TaskDescription([string]$taskPath) {
 if ($taskSpecific.ContainsKey($taskPath)) { return $taskSpecific[$taskPath] }
 $taskLeaf = [IO.Path]::GetFileName($taskPath)
 $taskStem = [IO.Path]::GetFileNameWithoutExtension($taskPath)
 $taskExt = [IO.Path]::GetExtension($taskPath).ToLowerInvariant()
 $taskContext = if ($taskPath.StartsWith('MqttVehicle/')) {'이전 인계 사본. '} elseif ($taskPath.StartsWith('materials/proj_check/')) {'과거 점검 사본. '} elseif ($taskPath.StartsWith('materials/unreal/')) {'dispatch 확장 후보; 기준 프로젝트 미적용. '} else {''}
 if ($taskLeaf -eq 'Blender_OBJ.zip') { return $taskContext + '장면 제작에 사용한 Blender/3D 원본 ZIP; 내부 자산 내용은 파일 카탈로그에서 별도로 판정하지 않음' }
 if ($taskExt -eq '.service') { return '기존 Pi systemd 서비스 정의: ' + $taskStem + '; 경로·계정·의존환경 확인 후 배포' }
 if ($taskLeaf -eq 'MqttVehicle.Build.cs') { return $taskContext + 'Unreal 차량·입력·UI·JSON·MQTT 모듈 의존성과 하위 include 경로' }
 if ($taskExt -eq '.uproject') { return $taskContext + 'Unreal 프로젝트 진입·모듈·플러그인 설정; EngineAssociation은 PC별 조정 필요' }
 if ($taskLeaf -like '*.Target.cs') { return 'Unreal ' + $taskStem + ' 빌드 타깃 정의 (5.7)' }
 if ($taskCodeRoles.ContainsKey($taskStem)) { return $taskContext + $taskCodeRoles[$taskStem] + $(if($taskExt -eq '.h') {' 선언·Blueprint API'} else {' 구현'}) }
 if ($taskStem -eq 'MqttVehicle' -and $taskExt -in @('.h','.cpp')) { return $taskContext + 'Unreal 게임 모듈 진입·로그 선언/등록' }
 if ($taskPath.StartsWith('materials/mqtt_headers/')) { return '조사 당시 엔진 MQTT 헤더 참고 사본: ' + $taskStem + '; 실제 설치된 5.7 헤더가 우선' }
 if ($taskLeaf -like 'DefaultEngine.ini*') { return $taskContext + '맵·GameMode·렌더링·물리·플랫폼 설정; 공유 사본의 AFS SecurityToken은 비움' }
 if ($taskLeaf -like 'DefaultGame.ini*') { return $taskContext + '프로젝트 설정과 MQTT 주소·토픽·타임아웃; 과거 IP는 현장 값으로 변경' }
 if ($taskLeaf -eq 'DefaultInput.ini') { return $taskContext + '입력 기본값·RawInput·Enhanced Input 설정; 실제 IMC 에셋도 확인' }
 if ($taskExt -eq '.ini') { return $taskContext + 'Unreal 에디터/프로젝트 설정: ' + $taskStem }
 if ($taskPath.StartsWith('materials/selected_free_assets/')) {
  if ($taskPath -match '/Converted/') {
   if ($taskStem -eq 'SM_Lanza') { return 'Lanzaman 변환 FBX: 위 모니터·지지대 제거와 아래 화면·두 좌석 보존' }
   $role = switch ($taskExt) { '.fbx' {'크기·좌표계·원점을 정규화한 UE 임포트 메시'} '.png' {'선택 모델에서 추출한 PBR 텍스처'} '.json' {'정규화 메시·재질·텍스처 연결 매핑'} default {'선택 에셋 변환 자료'} }
   return $role + ': ' + $taskStem
  }
  $maker = $taskPath.Split('/')[2]
  $role = switch ($taskExt) { '.glb' {'공식 원본 모델·재질'} '.fbx' {'원본 메시 또는 유영 애니메이션'} '.zip' {'제작자가 제공한 원본 소스 묶음'} default {'공식 원본 텍스처'} }
  return $maker + ' ' + $role + ': ' + $taskStem + '; 출처·라이선스는 ASSET_CREDITS'
 }
 if ($taskPath.StartsWith('MqttVehicle 5.7/Content/Underwater/Selected/')) {
  if ($taskStem -eq 'SK_Humpback') { return 'Connlan_Immure 혹등고래 관찰용 뼈대 메시·연결 재질' }
  if ($taskStem -eq 'SK_Humpback_Anim') { return '혹등고래 원본 Idle_Swim의 9.125초 반복 유영 애니메이션' }
  if ($taskStem -eq 'SK_Humpback_Skeleton') { return '혹등고래 메시와 원본 유영 애니메이션이 공유하는 뼈대' }
  if ($taskStem -eq 'SM_Iris') { return '보관용 IrisProcess 외형: 길이 4.8m 메시, 현재 플레이 선택에서 제외' }
  if ($taskStem -eq 'SM_Lanza') { return 'Lanzaman 탐사선 메시: 위 모니터·지지대 제거, 아래 화면·두 좌석 유지' }
  if ($taskStem -like 'SM_Seaweed_*') { return 'rkuhlf 해초 군집용 정적 메시: ' + $taskStem + '; 재질 흔들림 적용' }
  if ($taskStem -eq 'SM_Boulder01') { return 'Poly Haven Boulder 01: 충돌과 PBR 재질을 연결한 해저 바위 메시' }
  if ($taskStem -like 'BP_*') { return '해초 3종·바위의 HISM 반복 배치 Blueprint: ' + $taskStem }
  if ($taskStem -like 'M_*') { return '선택 탐사선·해초·바위·혹등고래의 UE 변환 재질: ' + $taskStem }
  return '선택 무료 모델의 UE 변환 텍스처: ' + $taskStem
 }
 if ($taskPath.StartsWith('MqttVehicle 5.7/Content/Vefects/Bubbles_VFX/')) {
  $role = if ($taskStem -like 'NS_*') {'Niagara 기포 시스템'} elseif ($taskExt -eq '.umap') {'제공 데모 맵'} elseif ($taskStem -like 'MI_*') {'재질 인스턴스'} elseif ($taskStem -like 'MF_*') {'재질 함수'} elseif ($taskStem -like 'M_*') {'머티리얼'} elseif ($taskStem -like 'T_*') {'텍스처'} elseif ($taskStem -like '*Skeleton*') {'데모 캐릭터 뼈대'} elseif ($taskStem -like '*PhysicsAsset*') {'데모 물리 에셋'} elseif ($taskStem -like 'AS_*') {'데모 애니메이션'} else {'메시/제공 리소스'}
  return 'Vefects Bubbles VFX ' + $role + ': ' + $taskStem + '; Fab 비공개 협업 자료'
 }
 if ($taskPath.StartsWith('materials/whale_original/')) {
  $taskWhaleDescription = switch ($taskExt) { '.blend' {'직접 제작한 고래 Blender 원본; 정적 모델'} '.fbx' {'고래 UE 임포트용 FBX 정적 모델'} '.glb' {'고래 GLB 교환·미리보기 모델'} '.png' {'직접 제작한 고래 모델 미리보기 이미지'} default {'직접 제작한 고래 모델 사용 안내'} }
  return $taskWhaleDescription
 }
 if ($taskPath.StartsWith('materials/fab_underwater_world_portable/Meshes/')) { return 'Underwater World 협업용 FBX 원본 메시: ' + $taskStem + '; UE 임포트·충돌·머티리얼 연결 필요' }
 if ($taskPath.StartsWith('materials/fab_underwater_world_portable/Textures/')) { return 'Underwater World 협업용 원본 텍스처: ' + $taskStem + '; BC/N/채널 구성은 원본 및 UE 머티리얼에서 확인' }
 if ($taskPath.StartsWith('MqttVehicle 5.7/Content/Underwater/')) {
  if ($taskPath -match '/Whale/') { return '직접 제작한 고래를 임포트한 관찰용 정적 메시: ' + $taskStem + '; 리깅·유영·조종 기능 없음' }
  if ($taskPath -match '/Meshes/') { return 'Underwater World에서 임포트한 해저 장면 정적 메시: ' + $taskStem + '; Fab 협업용 에셋' }
  if ($taskPath -match '/Textures/') { return '해저 메시용 UE 텍스처: ' + $taskStem + '; BC는 색상, N은 노멀, A는 해초 불투명 마스크' }
  if ($taskPath -match '/Materials/') { return '해저 장면에 연결한 머티리얼: ' + $taskStem + '; 바위·산호·해초·고래·바닥·표지·수중 배경' }
 }
 if ($taskExt -eq '.umap') { return $taskContext + 'Unreal 레벨 패키지: ' + $taskStem + '; 실제 액터·레벨 Blueprint는 에디터에서 확인' }
 if ($taskExt -eq '.uasset') {
  $taskRole = if ($taskPath -match '/__ExternalActors__/') {'World Partition 외부 액터 패키지; 맵과 함께 보존'} elseif ($taskPath -match '/__ExternalObjects__/') {'World Partition 외부 오브젝트 패키지; 맵과 함께 보존'} elseif ($taskPath -match '/(Input|Actions)/') {'입력 액션·매핑·터치 관련 에셋'} elseif ($taskPath -match '/Blueprints/') {'Blueprint/설정 에셋; 그래프와 부모 클래스를 에디터에서 확인'} elseif ($taskPath -match '/00_Works/01_Blend/') {'사용자 장면의 임포트 메시·머티리얼·텍스처 에셋; 정확한 타입은 에디터에서 확인'} elseif ($taskPath -match '/Vehicles/') {'차량 메시·물리·애니메이션·머티리얼 관련 에셋'} else {'Unreal 콘텐츠 에셋; 정확한 타입은 에디터에서 확인'}
  return $taskContext + $taskRole + ': ' + $taskStem
 }
 if ($taskExt -eq '.md') { return '기존 설명/인계/교육 문서: ' + $taskStem + '; 현 사용자 요구·코드와 충돌하면 AI_HANDOFF 기준으로 판단' }
 if ($taskExt -eq '.pptx') { return '발표 판넬 또는 수업 슬라이드 원본: ' + $taskStem }
 if ($taskExt -eq '.hwp') { return '기존 작품 기획·설명 한글 문서: ' + $taskStem }
 if ($taskExt -eq '.pdf') { return 'MQTT 교육 참고 PDF: ' + $taskStem }
 if ($taskPath.StartsWith('materials/references/') -and $taskExt -eq '.py') { return 'MQTT 교재 참고 예제: ' + $taskStem + '; 설치 paho 버전과 현재 토픽에 맞춰 검토' }
 if ($taskExt -eq '.js') { return '브라우저 MQTT 클라이언트 라이브러리 또는 JavaScript 자료: ' + $taskLeaf }
 if ($taskExt -eq '.json') { return $taskContext + '설정/참고 JSON: ' + $taskStem }
 return $taskContext + '프로젝트 관련 보존 자료: ' + $taskLeaf
}
$taskFiles = @(Get-ChildItem -LiteralPath $taskRoot -File -Recurse -Force | Where-Object {
 ('\' + $_.FullName.Substring($taskRoot.Length+1)) -notmatch $taskExcluded -and $_.Extension -notin @('.sln','.suo','.log','.pyc','.pem','.key','.pfx') -and $_.Name -notlike '~$*' -and $_.Name -notlike '.env*' -and $_.Name -notin @('Thumbs.db','.DS_Store')
} | Sort-Object FullName)
$taskRecords = [Collections.Generic.List[object]]::new()
foreach ($taskFile in $taskFiles) {
 $taskRelative = $taskFile.FullName.Substring($taskRoot.Length+1).Replace('\','/')
 if ($taskRelative -in $taskSelfOutputs) { continue }
 $taskRecords.Add([pscustomobject]@{path=$taskRelative;bytes=$taskFile.Length;sha256=(Get-FileHash -LiteralPath $taskFile.FullName -Algorithm SHA256).Hash.ToLowerInvariant();description=(Get-TaskDescription $taskRelative)})
}
$taskDocs = Join-Path $taskRoot 'docs'
New-Item -ItemType Directory -Path $taskDocs -Force | Out-Null
$taskManifest = [ordered]@{schema_version=1;baseline_date=$BaselineDate;hash_algorithm='SHA-256 of actual file bytes, not Git/LFS pointer bytes';file_count=$taskRecords.Count;files=$taskRecords}
$taskManifest | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $taskDocs 'FILE_MANIFEST.json') -Encoding utf8NoBOM
$taskGuide = [Collections.Generic.List[string]]::new()
$taskGuide.Add('# 업로드 파일별 설명')
$taskGuide.Add('')
$taskGuide.Add('기준일: '+$BaselineDate+'. 아래 목록은 재생성 파일·로그·인증정보를 제외한 모든 업로드 파일을 설명한다. 바이너리의 타입·동작은 경로와 이름으로 분류했으며 에디터/문서 렌더링으로 모두 검사한 것은 아니다. 원본 사본도 보존하므로 사용 여부와 작업 기준을 구분한다.')
$taskGuide.Add('')
$taskGuide.Add('파일 '+$taskRecords.Count+'개, 실제 파일 합계 '+[math]::Round(($taskRecords | Measure-Object bytes -Sum).Sum/1MB,1)+' MiB. Git LFS는 동일 내용 파일을 공유하므로 전송량은 더 적을 수 있다.')
$taskGuide.Add('')
$taskGuide.Add('이 카탈로그와 FILE_MANIFEST.json은 자기참조 해시를 피하기 위해 manifest의 해시 대상에서 제외하지만 둘 다 업로드한다. 재생성: `pwsh -File tools/build_file_catalog.ps1`.')
$taskGuide.Add('')
$taskGuide.Add('## 카탈로그 파일 자체')
$taskGuide.Add('')
$taskGuide.Add('| 경로 | 역할 |')
$taskGuide.Add('| --- | --- |')
$taskGuide.Add('| [docs/FILE_GUIDE.md](FILE_GUIDE.md) | 각 파일 역할·크기와 보존 범위 |')
$taskGuide.Add('| [docs/FILE_MANIFEST.json](FILE_MANIFEST.json) | 기계 판독 가능한 경로·설명·바이트 크기·SHA-256 |')
$taskGuide.Add('')
$taskGroup = ''
foreach ($taskRecord in $taskRecords) {
 $taskTop = $taskRecord.path.Split('/')[0]
 if ($taskRecord.path -notmatch '/') { $taskTop = '루트' }
 if ($taskTop -ne $taskGroup) {
  $taskGuide.Add('')
  $taskGuide.Add('## ' + $taskTop)
  $taskGuide.Add('')
  $taskGuide.Add('| 파일 | 크기 (KiB) | 역할 |')
  $taskGuide.Add('| --- | ---: | --- |')
  $taskGroup = $taskTop
 }
 $taskLink = ($taskRecord.path.Split('/') | ForEach-Object { [Uri]::EscapeDataString($_) }) -join '/'
 $taskLabel = $taskRecord.path.Replace('|','\|').Replace('[','\[').Replace(']','\]')
 $taskDesc = $taskRecord.description.Replace('|','\|')
 $taskGuide.Add('| [' + $taskLabel + '](../' + $taskLink + ') | ' + [math]::Round($taskRecord.bytes/1KB,1) + ' | ' + $taskDesc + ' |')
}
$taskGuide.Add('')
$taskGuide.Add('## 제외한 재생성·개인 환경 자료')
$taskGuide.Add('')
$taskGuide.Add('- Unreal Binaries / Intermediate / Saved / DerivedDataCache / .vs / 생성 .sln')
$taskGuide.Add('- Office 잠금 파일, 판넬 제작 .panel_build 임시 출력, node_modules와 Python 환경·캐시')
$taskGuide.Add('- 실행 로그와 비밀키·환경 인증 파일. 공유 DefaultEngine.ini의 Android File Server SecurityToken 값은 비움')
$taskGuide.Add('- .github-upload / .upload-tools는 로컬 업로드 작업용이며 저장소 콘텐츠가 아님')
$taskGuide | Set-Content -LiteralPath (Join-Path $taskDocs 'FILE_GUIDE.md') -Encoding utf8NoBOM
[pscustomobject]@{Files=$taskRecords.Count;MiB=[math]::Round(($taskRecords | Measure-Object bytes -Sum).Sum/1MB,1);Catalog='docs/FILE_GUIDE.md'} | ConvertTo-Json
