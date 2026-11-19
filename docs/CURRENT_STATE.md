# 현재 구현 상태

## 2026-10-31 현재 Pi 주소: 192.168.0.6

- 현재 Pi 연결 주소는 `192.168.0.6`이다. Quest와 Pi의 ping, 대시보드의 Quest·MQTT 연결 상태까지 확인했으며, 영상 수신과 실제 RC카 바퀴 반응은 이 주소에서 다시 점검해야 한다.

## 2026-10-23 전시 발표자료 최종 개편

- `output/D-Mix_Presentation.html`: 총 6장 — 작품 소개 → 개발 배경 → 시스템 구성 → 핵심 기술 → MQTT 통신 → 프로젝트의 의미. 발표자: 컴퓨터정보과 이승민.
- 제목은 “VR·IoT 기반 융합 디지털트윈 시스템”. 2번은 탐사의 한계·접근 방식, 4번은 자동복귀 경로 탐색·MQTT 차량 제어 두 카드로 구성한다. 비교용 3-1·4-1은 제거했다.
- MQTT 설명은 비전공자용 정보 전달 흐름으로 정리했다. 기대 효과는 6번으로 옮기고 가상·실물 연결 및 확장 가능성과 함께 제시한다.
- SUIT 내장, 화이트·하늘색·블루, 둥근 카드, 우상단 쪽번호와 16:9 자동 배율을 유지한다. 박스 상단 색 띠는 제거했다. 브라우저로 열고 방향키로 전환하며 F로 전체 화면을 사용할 수 있다.
- 기존 브라우저 렌더 검증 후 최종 6장 구성·비교 슬라이드 제거를 확인했다. 사용자가 이번 발표자료와 관련 문서의 GitHub 게시를 승인했다. 아래 과거 날짜의 슬라이드 수·로컬 보관 표기는 작업 당시 이력이다.

## 2026-10-22 전시 발표자료

- [HTML 발표자료](output/D-Mix_Presentation.html)를 내려받아 브라우저로 연다. GitHub 파일 화면에서는 HTML이 발표 화면으로 실행되지 않으므로 파일을 다운로드한다.
- 구성: 1 작품 소개 / 2 시스템 구성 / 3 핵심 기술과 구현 결과 / 4 MQTT 통신 / 5 프로젝트의 의미. 발표자: 컴퓨터정보과 이승민.
- SUIT Regular/Bold와 OFL 라이선스, 기존 해저 개발 이미지를 HTML에 내장했다. 인터넷 없이 실행되며 16:9 비율로 화면 중앙에 맞춰 확대·축소한다. 방향키·Space로 전환, Home/End로 처음/마지막, F로 전체 화면 전환.
- 화이트·하늘색·블루, 둥근 카드, 한국어 주제, 우상단 쪽번호를 사용한다. 개발 이미지는 최신 APK 실기 캡처가 아니다. 4번의 JSON 예시 및 3번 결과/추가검증 박스는 사용자 요청으로 제외했다.
- 브라우저에서 슬라이드 렌더·글꼴 로드·키보드 전환·여러 화면 크기에서 비율 유지를 확인했다. 발표자료 작업이며 Unreal·Pi·모터 시험은 수행하지 않았다.
- 2026-10-22 사용자가 HTML과 관련 내용의 커밋·푸시를 명시적으로 승인했다. 기존 로컬 전용 지시는 이번 발표자료 게시 승인으로 대체한다. 최신 APK 소스/배포 문서는 보존한다.


## 2026-10-21 VR 알림을 대시보드에 표시 — 로컬 APK 1.0.3

- 사용자 선택에 따라 VR 위젯의 실제 문구·남은 표시 시간을 기존 Quest 상태 요청에 포함하고, 대시보드 영상 위에 HTML 자막으로 표시한다. 추가 SceneCapture나 영상 합성은 하지 않는다.
- 자동복귀 준비·시작·도착·중단, 고래 관찰, 해파리 주의 및 복귀 중 배지를 전달한다. 동일 알림의 반복 상태 갱신이 표시 시간을 재시작하지 않도록 남은 시간을 사용한다. Pi와 브라우저 양쪽에서 만료·연결 단절 시 자막을 지운다. 상태 전달 주기 때문에 VR과 웹 표시에는 약0.5~1초 차이가 날 수 있다.
- Windows/Android 컴파일·쿠킹·APK 패키징 성공. 서명·버전·ZIP 검사 통과. 서버 검사14개 및 JavaScript 구문 검사 통과. 별도 시험 영상에서 복수 자막 배치 확인.
- Pi 웹서비스에 배포 완료(기존 파일 .runtime/pre-captions 백업). Quest USB 업데이트 및 앱 실행 완료. 기존 RC 송신 꺼짐·양 출력0 확인. 실제 VR 알림과 웹 화면을 동시에 비교한 실기 확인은 미완료다. 사용자 선택은 현재 문구·시간 동기화 방식 유지이며 위젯 영상 합성은 적용하지 않는다.
- APK versionCode=4/versionName=1.0.3, SHA256 `3128d092bc97025fa7732fbe6a33104fe012ce9a390fb38de5c567157b012873`. 로컬 설치 ZIP `outputs/DMix-Quest3-1.0.3-Windows.zip`. 사용자가 현재 웹 자막 방식을 유지하고 이 변경의 커밋·푸시를 승인했다. 설치 ZIP은 GitHub Releases의 1.0.3으로 게시한다.


## 2026-10-21 Quest 영상 1.0.2 — 설치·실기 영상 확인 완료 (목표 30fps 미달)

- APK 1.0.2(versionCode 3) 빌드·서명·ZIP 무결성 검사 완료. USB adb install -r 성공 및 기기 버전 확인. 기존 앱 삭제 없이 업데이트 후 앱을 실행해 검증했다. 모터 구동은 하지 않았다.
- 960×540 하드웨어 H.264 인코더와 Pi의 메모리 기반 fMP4 중계, 브라우저 연속 재생 추가. 목표 30fps이며 Quest 실측은 17.9~18.1fps로 목표에 미달한다. VR 렌더 FPS는 별도로 측정하지 않았다. 인코더 실패 시 JPEG 최대10fps로 전환한다. 추가 렌더링·픽셀 변환 비용이 있으므로 Quest VR 프레임도 확인해야 한다.
- 서버 검사12개, 시험 AVC의 FFmpeg 119프레임 디코딩, 브라우저 960×540 재생 및 새로고침 재연결 확인. 시험 소스 수신은 약30fps였으며 실제 Quest 성능 측정과 구분한다.
- Pi 서버 업데이트·서비스 재시작 완료. 기존 JPEG APK도 연결 유지 확인. 변경 전 웹파일은 Pi .runtime/pre-h264에 보관했다. RC 수신기 서비스는 변경하지 않았다.
- 사용자 요청으로 RC 송신 끄기 및 Quest 응답 확인. 1.0.1 실행에서 send_enabled=false, 양 모터 출력0, can_return=true 순간 확인. 실제 자동복귀 전체 경로·고래 재진입 시험 완료를 뜻하지 않는다. 앱 포커스가 없으면 can_return은 false가 된다.
- 출력: outputs/Quest3-1.0.2/MqttVehicle-arm64.apk 및 outputs/DMix-Quest3-1.0.2-Windows.zip. SHA256 895c68d7e7df5994e56fac63be436a6fc10803e416b36b36481c7a3a1c271fb3. 사용자 승인으로 이 변경 묶음과 설치 ZIP을 GitHub에 게시한다.
- 아래 1.0.1 및 미구현 표시는 당시 기록이다. 최신 상태는 이 절을 따른다.


- 최종 실기: Qualcomm c2.qti.avc.encoder, 960×540 AVC → Pi → 브라우저 정상 재생 및 페이지 재연결 확인. GPU 읽기 대기열은 최대3장. Codec2 마지막 UV 픽셀 뒤 패딩 제외 크기 및 정상 출력 버퍼 변경 알림 처리 수정. RC 송신 false·양 출력0을 동시에 확인했다. 자동복귀 요청 응답 기록은 관찰했지만 전체 경로·고래 재진입·물리 모터 시험 완료를 의미하지 않는다. 장시간 안정성은 미검증이다.

## 2026-10-21 자동복귀 진입·반복 체험 수정 APK 1.0.1 (로컬)

- 사용자가 실제 문구를 “복귀 가능한 경로·제어 상태가 없습니다”로 확인했다. 이 문구는 A* 탐색 전 준비 조건 실패이며 경로가 없다는 뜻으로 단정할 수 없다. 정확한 당시 실패 조건은 기존 로그로 확정하지 않았다.
- RC 송신 게이트가 꺼져 있을 때도 MQTT 연결을 요구하던 조건을 수정했다. 송신이 켜져 있으면 연결 요구를 유지한다. 송신이 꺼져 있는 동안의 MQTT 재접속 이벤트는 가상 주행을 중단하지 않는다.
- Pawn/브릿지/복귀 컴포넌트가 OpenXR 세션의 FApp VR 포커스를 공통 사용한다. VR 포커스가 없으면 데스크톱 창 포커스로 우회하지 않으며 PC 모드는 기존 창 포커스를 사용한다. 일시정지·입력 차단은 유지한다.
- 일회성 고래 발견과 구역 재진입 복귀 요청을 분리했다. 구역 이탈 후 재진입 때 다시 요청하며 구역 안의 중복 이벤트와 명시적 정지 후 자동 재시작은 막는다. 거절 원인은 출발점·제어권·포커스·MQTT로 구분하고 RETURN_REJECTED 로그를 추가했다.
- 최신 GitHub 9cb3a33의 주소 따옴표 및 960×540/JPEG80/최대10fps 개선을 반영했다. 30fps 연속 영상은 미구현이며 Pi 웹 갱신 배포는 이번 작업에서 하지 않았다.
- Windows 및 Android 빌드, ASTC 쿠킹·패키징 성공. 복귀·관찰 자동화 9개 및 대시보드 10개 통과, JavaScript 구문 검사 통과. 실제 맵 고래 관찰 지점의 높이 3개에서 A* 경로 확보를 확인했다. 엔진 초기화 DDC/OpenXR/Condition failed 메시지는 자동화 결과와 구분한다. 첫 패키징은 동시 검사 프로세스의 DLL 점유로 실패해 검사 종료 뒤 재실행했다.
- APK versionCode=2/versionName=1.0.1, 263861632 bytes, SHA-256 `7166fc3b185650bda3ac1a012b4a890579294c8f5e8c5bc65c75cbddbc97e612`. 출력 `outputs/Quest3-1.0.1/` 및 `outputs/DMix-Quest3-1.0.1-Windows.zip`. 기존 앱 삭제 없이 업데이트 설치한다.
- 새 APK 실기 설치·VR 시각/프레임·반복 이벤트·RC 검증은 미완료다. Pi 접속·서비스 변경·모터 명령·GitHub 게시는 수행하지 않았다. 아래 오류 보고 및 이전 배포 상태는 과거 기록이다.


## 2026-10-21 자동복귀 수정 진행 — APK 최종 조립 대기

- 사용자 확인 문구는 “복귀 가능한 경로·제어 상태가 없습니다”이며 A* 실행 전 준비 조건 거절이다. 당시 세부 원인은 아직 확정하지 않았다.
- RC 송신을 꺼도 MQTT 연결을 요구하던 조건, OpenXR/데스크톱 포커스 판단, 고래 관찰 구역 재진입 요청을 수정했다. RC 송신 켜짐 상태의 연결 보호와 포커스 상실 정지는 유지한다. 거절 이유를 출발점/입력/포커스/MQTT로 구분한다.
- 최신 main 9cb3a33의 대시보드 URL 따옴표 및 최대10fps 영상 개선을 반영했다. 30fps 전송은 후속 작업이다.
- Windows 및 Android C++ 컴파일·맵 쿠킹 성공. 복귀·관찰 검사9개, 대시보드 검사10개, JS 구문 검사 통과. 실제 맵 고래 관찰 지점의 높이3개에서 경로 확보 확인. Quest 실기 및 Pi 배포·모터 시험은 하지 않았다.
- APK 마지막 Java 조립에서 캐시 JAR의 toRealPath 접근 거부가 발생했다. 경로 권한 추가 후에도 반복되어 사용자에게 일반 PowerShell에서 outputs/Finish-Quest-Build.ps1 실행을 요청했다. 기존 APK는 이전 버전이며 새1.0.1 완성으로 표시하지 않는다. 성공 시 출력 QUEST_1_0_1_READY 및 새 해시로 확인한다.
- 로컬 변경이며 GitHub에 게시하지 않았다.


## 우선 확인할 오류: 자동복귀 진입 실패 및 이벤트 재진입 무반응 (2026-10-21 사용자 보고)

**상태: 사용자 실기 보고, AI 직접 재현·원인 확인·수정은 아직 하지 않음.** 아래 문제를 해결하기 전 자동복귀와 반복 체험이 정상이라고 표시하지 않는다.

1. **자동주행(자동복귀) 진입 실패:** 자동주행 모드로 들어가려 하면 사용자 표현으로 “갈 수 있는 경로가 없다”는 알림이 표시되고 자동주행이 시작되지 않는다. 실제 로그의 정확한 문구, 발생 위치 및 진입 방식(이벤트/버튼)은 추가 확인이 필요하다.
2. **재시도 시 무반응:** 위 실패 이후 사용자가 수동으로 출발점까지 돌아온 다음, 다시 자동주행을 유발하려고 이전 도착지/이벤트 지점으로 이동했으나 아무 반응이 없었다. 사용자는 이벤트가 한 번만 실행되는 것으로 추정한다. 일회성 트리거가 원인이라고 확정하지 않는다.

### 다음 조사·수정 작업

- 같은 실행 세션에서 첫 이벤트 진입 → 경로 탐색 실패 → 수동 출발점 복귀 → 이벤트 지점 재진입 순서로 재현하고, 위치·고도·이벤트 종류·복귀 상태·경로 탐색 결과를 기록한다. 사용한 APK 버전도 확인한다.
- 3D A*의 출발점/현재 위치 유효성, 탐색 범위, 충돌·장애물 판정 및 실패 로그를 확인해 경로가 없다고 판단하는 원인을 조사한다. 이는 조사 항목이며 확인된 원인이 아니다.
- 이벤트 재발동 조건과 일회성 처리 여부, 경로 탐색 실패 후 상태·타이머·트리거 초기화를 점검한다. 실패가 이후 재시도를 막지 않도록 수정하고, 동일 이벤트의 중복 실행 방지와 재도전 허용을 함께 검증한다.
- 경로 탐색 실패 후 수동 조작 유지, 다시 요청했을 때 성공 또는 명확한 실패 안내, 출발점 복귀 후 반복 체험을 검증한다. 장애물 관통이나 강제 순간이동으로 실패를 우회하지 않는다.


## 다음 필수 작업: 전시용 대시보드 영상의 끊김 개선 (2026-10-21 요청)

**상태: 후속 작업으로 확정, 30fps 전송은 미구현·미검증.** 960×540·JPEG 품질 80·최대 10fps 수정은 GitHub 커밋 `89f8a92`에 반영됐지만 APK 재빌드·Quest 설치·Pi 웹 배포 전이다. 10fps 설정만으로 부드러운 전시 영상을 완성했다고 판단하지 않는다.

1. Unreal 빌드 가능한 컴퓨터에서 개선 APK를 패키징하고 Quest에 설치한다. Pi 웹 화면도 함께 업데이트해 해상도와 갱신 주기를 맞춘다.
2. 실제 탐사·빠른 시점 회전 중 대시보드 수신 fps, 영상 지연·끊김, 네트워크 사용량과 Quest VR 프레임 성능을 측정한다. 설정상 최대 fps와 실제 측정값을 구분한다.
3. 전시용 관전자 영상은 **30fps를 목표**로 전송 구조를 개선한다. JPEG 단일 이미지의 해상도·요청 빈도만 높이는 것으로 끝내지 않고, Quest에서 지원 가능한 하드웨어 영상 인코딩과 연속 영상 전송 방식을 조사·선정한다. 코덱·프로토콜·Pi 중계 방식은 아직 확정하지 않았다. 기존 PC Pixel Streaming을 Android APK에서도 그대로 사용할 수 있다고 가정하지 않는다.
4. 선택한 방식으로 Quest → Pi → 대시보드 경로를 구현하고, 지연된 영상이 쌓이지 않도록 처리한다. 관전자 화면의 선명도와 움직임을 개선하면서 헤드셋 VR 체험 성능과 MQTT 제어 응답을 유지한다.
5. 실제 전시 Wi-Fi에서 장시간 실행, 연결 끊김·재연결, 영상 지연·프레임 및 Quest 부하를 검증한다. 결과를 README에 기록한 뒤 APK·Pi 배포본과 설치 안내를 갱신한다. 목표 30fps는 검증 결과가 나오기 전까지 달성으로 표시하지 않는다.


## 2026-10-21 Quest 관전자 영상 품질 개선 — 로컬 구현, APK 적용 전

- 기존 APK는 640×360, JPEG 품질 65, 최대 2fps이며 웹도 500ms마다 이미지를 갱신했다. 낮은 원본 해상도와 프레임 제한을 코드에서 확인했다.
- Android 설정에 Width=960, FrameRate=10, JPEGQuality=80을 추가하고 캡처·JPEG 압축·전송 간격이 설정을 읽도록 변경했다. 가로 640~1280(32 단위), 16:9, 1~15fps, 품질 50~90 범위다. 기존 한 프레임씩 처리하는 구조를 유지해 업로드가 느릴 때 프레임이 누적되지 않는다. 실제 10fps 달성을 보장하지 않는다.
- 웹 영상 요청을 상태 조회와 분리해 최대 100ms 간격으로 갱신하고, 이전 이미지 로딩 중 또는 탭이 숨겨진 동안 추가 영상 요청을 하지 않는다. 상태/제어 조회는 기존 500ms다.
- JavaScript 구문 검사를 통과했다. 현재 PC에서 Unreal 빌드 환경을 확인하지 못해 C++ 컴파일·APK 재패키징·실기 성능 검증은 하지 않았다. 기존 설치 APK 및 Pi에는 이 품질 개선을 아직 배포하지 않았다. 새 APK에서 Quest VR 프레임 저하 여부와 Pi 영상 수신률을 측정한 뒤 설정을 조정해야 한다. 2026-10-21 사용자가 이 변경의 GitHub 게시를 승인했다.


## 2026-10-21 사용자 테스트용 Pi 수신기 시작

- 사용자가 모터 제어 수신기 시작을 명시적으로 승인해 dmix-rc-session 서비스를 시작했다. active 및 대시보드 receiver.dry_run=false, reason=waiting_command, l/r/th/st=0을 확인했다. 앞선 수신기 정지 기록은 당시 점검 상태다. 주행 명령 송신이나 모터 동작 시험은 실행하지 않았다.
- 이번 확인 시 Quest connected=false/streaming=false였다. Quest 앱을 다시 활성화하고 Pi 대시보드에서 연결을 확인한 뒤 사용자가 RC 명령 송신을 켜서 테스트한다. Pi 서버/MQTT 연결 및 수신기 준비와 Quest 실시간 연결·실물 동작 검증을 구분한다.


## 2026-10-21 최신 Release APK의 Quest 3 업데이트 설치 확인

- 사용자 USB 연결 및 디버깅 승인 후 Quest 3에 최신 Release APK를 adb install -r로 설치해 Success를 확인했다. 패키지 com.he4th.dmix, versionCode=1/versionName=1.0 및 설치 갱신 기록 확인. APK 263844800 bytes, SHA-256 d68bb2df32b21d4a3ea73a782fca93ec81ba2b6004854912fb8757e445688b37로 배포 manifest와 일치한다. 이전 실기 설치 미확인 기록은 이 설치 결과로 대체하되 앱 동작 검증과 구분한다.
- 최초 설치 시 Quest/노트북은 192.168.0.x로 Pi 접속이 실패했다. 사용자가 Wi-Fi를 변경한 뒤 Quest 10.95.86.57에서 Pi 10.95.86.138 ping 2/2 성공 및 노트북 SSH 접속을 확인했다. 이후 사용자가 앱을 실행했고 MQTT PingResponse 로그를 확인했다. 실제 RC 명령 송수신 검증과 구분한다.
- 앱 로그의 QUEST_DASHBOARD_READY가 http:로 출력되고 POST http:/api/quest/control이 실패했다. AndroidGame.ini의 URL에 큰따옴표가 없어 주소가 잘리는 문제로 판단해 URL="http://10.95.86.138:5050"으로 로컬 수정했다. 배포 APK는 아직 재빌드하지 않았다. 현재 Quest의 Saved/Config/Android/Game.ini에도 같은 주소를 기기별로 적용했다. 사용자 재실행 후 QUEST_DASHBOARD_READY에 전체 주소가 표시됐고 Pi /api/state에서 connected=true, streaming=true, sequence=27, control.broker=true, send_enabled=false, can_return=true를 직접 확인했다. 주소 재정의가 현재 설치 기기에서 동작하고 영상 프레임·제어 상태가 Pi에 수신됨을 확인했다. 다른 기기/기존 Release APK에 해결이 반영됐다고 판단하지 않는다.
- 통합 점검 중 실행 중이던 RC 수신 서비스를 정지했고 inactive/dead를 확인했다. 수신기 시작·송신 켜기·모터 시험은 하지 않았다. 사용자가 앱을 재실행해 연결을 검증했다. VR 화면의 시각 품질·자동복귀 알림·웹 버튼 실제 동작·모터 연동은 미검증이다. 다음 작업은 따옴표를 수정한 설정으로 APK 재패키징 및 새 배포본 검증이다. 현재 기기별 설정은 앱 삭제/데이터 초기화 시 사라질 수 있다. APK와 임시 설정은 Git 제외 폴더에 보관한다. 사용자가 이번 수정·검증 문서와 주소 설정의 GitHub 게시를 승인했다.


## 2026-10-21 Pi 대시보드 버튼·수신 서비스 배포 완료

- 사용자 요청으로 Pi 10.95.86.138(zerotoai)에 원격 main 84917a4의 server.py, app.js, index.html, test_server.py, dmix-rc-session.service, setup_rc_service.sh를 적용했다. 기존 파일/기존 사용자 서비스는 /home/pi/dmix-dashboard/.runtime/pre-controls-20261021에 백업했다. 아래의 Pi 배포 보류 기록은 이번 배포로 대체된다.
- Pi의 임시 배포 폴더에서 HTTP/상태 단위 검사 10개, Node JavaScript 구문 검사, 셸 구문 검사 통과. 설치 스크립트로 사용자 RC 서비스 등록·pi linger 설정·DMIX_RC_SERVICE=1을 적용하고 dmix-dashboard만 재시작했다. dmix-dashboard와 dmix-pixel active, 실제 /api/state의 broker=true 및 receiver_start_available=true, 페이지의 버튼 3개를 확인했다. 배포한 서버/화면 3개 파일의 SHA-256은 저장소와 일치했다.
- RC 수신기는 inactive/dead, 서비스는 static으로 부팅 자동 시작을 등록하지 않았다. 모터 제어 프로세스 없음 확인. 수신 시작/송신 켜기/자동복귀 버튼을 실제로 누르거나 제어 요청을 보내지 않았다. 서비스는 시작 후 비정상 종료 시 재시작하도록 정의돼 있다.
- /home/pi/rover_drive.py는 수정하지 않았고 전후 SHA-256 d69636f746cf62970436778f92288ad43da5c6a06c6b97658d39d89276eaeed7 일치를 확인했다. 기존 Pi 회전 출력 보정을 저장소 수신 코드로 덮어쓰지 않았다.
- 검증 당시 Quest connected=false, streaming=false, control=null, receiver=null이었다. 최신 APK 연결 후 실제 영상·송신 전환·복귀 알림·고정 바퀴 검증은 남아 있다. http://10.95.86.138:5050 을 새로고침해 사용한다. 2026-10-21 사용자가 이번 Pi 배포 결과 문서의 GitHub 게시를 승인했다.


## 2026-10-21 VR 자동복귀 알림 및 휴대용 APK 배포

- 자동복귀 시작/도착/수동 중단/정지 메시지를 약3초 표시하고 복귀 준비/진행 중에는 작은 상태 표시를 유지한다. 반복 상태 전달로 타이머를 초기화하지 않는다. 기존 고래/해파리 알림은 유지한다.
- HMD 실행에서는 1280x720 UMG 렌더 타깃을 얼굴 고정 Stereo Layer로 표시한다. 가상거리2m, 크기153.6x86.4cm, 깊이 가림 없음, 30Hz 위젯 갱신. PC는 기존 화면 위젯을 유지한다. 실제 Quest의 글자 크기/시야 위치/가독성은 아직 미검증이다.
- Windows/Android 컴파일·쿠킹·패키징 성공, APK v2 서명과 ZIP CRC 검증. EncounterNotices 자동화 검사 Success(시작·중복 상태·도착·수동 중단 검증 추가). 최초 헤드리스 실행은 쓰기 가능한 DDC 부재로 초기화 실패했으며 메모리 캐시로 재실행해 검사 성공.
- APK 263,844,800 bytes, SHA-256 d68bb2df32b21d4a3ea73a782fca93ec81ba2b6004854912fb8757e445688b37.
- 사용자 추가 승인으로 APK·설치 코드를 GitHub 비공개 저장소 Release에 게시했다: https://github.com/HE4TH/digital_twin/releases/tag/quest3-2026.10.21-preview . DMix-Quest3-Windows-2026.10.21.zip(248,524,031 bytes)에 APK·Install-Quest3.cmd/ps1·README·release.json 포함. Release 태그는 원격 main(README 설치 안내 커밋06e5fc4)을 가리키며 최신 로컬 C++ 소스와 같다고 주장하지 않는다.
- distribution/quest3에 휴대용 설치 코드/설명/해시를 보관한다. 고정 노트북 경로를 사용하지 않고 APK 해시 확인 후 adb install -r로 갱신한다. ADB 미설치 시 사용자가 Google SDK 약관을 직접 확인·동의한 뒤 공식 Platform Tools를 내려받는다. 실행 정책 변경은 해당 PowerShell 프로세스에 한정한다. 앱·모터 자동 실행 없음.
- 원격 README 최상단에 다른 노트북 설치 절차를 추가해 게시 확인. 이전 소스 커밋f396b44는 원격에 이미 게시된 사실을 브라우저에서 확인했다. 이번 새 C++/대시보드 소스의 원격 게시와 Release APK 게시를 구분한다.
- Pi는 현재 미연결이며 사용자 지시에 따라 배포를 보류한다. 새 APK의 실기 설치/영상/모터/VR 알림 검증도 미완료다.


## 2026-10-21 Quest 대시보드 버튼 3개

- 사용자 요청에 따라 RC 수신 시작, RC 명령 송신 켜기/끄기, 자동복귀 시작만 표시한다. 수동 전환·수신 중지·복귀 취소·별도 정지 버튼은 표시하지 않는다.
- QuestDashboardComponent가 /api/quest/control에 0.5초 간격으로 상태/적용 응답을 보내고 명령을 받는다. 명령 ID 중복 실행 방지, 2초 서버 대기 만료, 에디터/Quest 응답 분리를 적용한다. 복귀 가능 여부와 송신 게이트/Quest MQTT 연결을 표시한다. 구형 영상 전용 APK는 제어 버튼을 활성화하지 않는다.
- 새 APK는 세션 시작 시 RC 송신 게이트가 꺼진다. 사용자가 송신 켜기를 눌러야 주행 명령을 전달한다. 송신 끄기 또는 대시보드 서버와 3초 통신 단절 시 게이트를 닫고 MQTT 출력값을 0으로 보낸다. 연결 복구만으로 재활성화하지 않는다. 가상 탐사선 움직임과 기존 PC 설정은 유지한다. Pi 수신기의 기존 명령 단절 정지도 유지한다.
- Pi 수신 시작은 고정된 dmix-rc-session.service만 systemctl --user로 시작한다. 셸/임의 명령 실행을 허용하지 않는다. 송신 켜짐·처리 중 요청·구형 Quest 상태일 때 실행을 거부한다. raspberry-pi/dashboard/dmix-rc-session.service와 setup_rc_service.sh를 준비했다. 설치 스크립트는 수신기를 시작/재시작하지 않고 대시보드만 재시작한다. 수신기 부팅 자동 시작은 추가하지 않는다.
- 검증: 로컬 HTTP/상태 검사10개 통과, JS 구문 검사 통과, Android C++ 빌드/쿠킹/패키징 ExitCode0, APK ZIP CRC 통과. APK 263,852,052 bytes, SHA-256 14ffc684c9c497c15a380165f91ed4ad193f51acc350ee2ff86237f9aa913f26.
- Pi 접속 시 타임아웃. 사용자가 현재 Pi 미연결이며 Pi 작업은 나중에 하자고 요청했다. Pi 배포·서비스 설치·실기 버튼/모터/영상 검증은 보류다. 새 APK 설치도 아직 확인하지 않았다. GitHub 원격 게시 완료와 구분한다.


## 2026-10-20 Quest APK RC 연결 및 Pi 직접 영상 송신

- 사용자가 Quest APK에서 RC카 미동작과 대시보드 영상 누락을 보고했고, Quest에서 Wi-Fi로 Pi에 직접 송신하는 방식을 선택했다.
- Pi의 mosquitto, dmix-dashboard, dmix-pixel, RC 수신 세션이 active임을 확인했다. 점검 당시 외부 MQTT 연결은 없고 Pi 내부 연결2개만 있었다. 명령을 송신하거나 모터를 시험하지 않았다.
- Android/BSD GetConnectionState는 connect 호출 전에도 쓰기 가능 상태를 Connected로 반환할 수 있다. MQTTConnection이 이를 보고 접속 호출을 건너뛰던 분기를 제거했다. Connect를 먼저 시도하고 GetPeerAddress로 실제 TCP 연결을 확인하며, 과거 errno를 현재 연결 상태로 오해하던 처리도 제거했다.
- 새 QuestDashboardComponent는 Android에서만 동작한다. AndroidGame.ini의 DmixQuestDashboard.URL=http://10.95.86.138:5050으로 전송하며 Enabled=False로 끌 수 있다. HTTP 수요 조회는 초당1회, 대시보드가 열려 있을 때만 640x360/FOV100 관전자 캡처를 최대 초당2회 한다. GPU 비동기 readback, 작업 스레드 JPEG 품질65 압축, 동시 프레임1개와 HTTP 타임아웃2초로 큐 누적을 제한한다. 헤드셋 양안 영상을 자르거나 합성하지 않는다.
- Pi 대시보드에 /api/quest/demand와 /api/quest/frame을 추가했다. 명시한 LAN Host/Origin 검사, X-Quest-Stream 헤더, JPEG envelope/512KiB 제한, 요청 읽기 제한시간3초, 마지막 프레임 메모리1장, 4초 이후 오래된 프레임 숨김을 적용했다. LAN의 기존 공개 대시보드와 같은 접근 범위이며 헤더는 인증 비밀값이 아니다. 인터넷 공개용 인증 기능은 아니다.
- 웹페이지는 최신 Quest 이미지가 있으면 PC Pixel Streaming보다 우선해 16:9 전체 화면으로 표시한다. APK 연결 상태와 에디터 제어 브릿지를 구분한다. 이번 변경은 영상 송신이며 APK에 Python 제어 브릿지를 추가하지 않았으므로 대시보드의 자동복귀/정지 버튼은 에디터 브릿지가 없으면 활성화되지 않는다. 기존 컨트롤러 조작·MQTT 명령 경로를 사용한다.
- 로컬/Pi HTTP 검사7개 통과, JS 구문 검사 통과, Windows 빌드 및 MQTT.Client.ConnectTimeoutLifetime/MQTT.Connection.ShutdownDuringConnect 회귀 검사 통과. Android 빌드/쿠킹/패키징 및 APK 서명/CRC 확인. 새 APK 263,854,736 bytes, SHA-256 977fe923ace45efeb482447d2771294b4684d23c62fb3baed2973be5f1e6f3ff. 실제 Quest 영상 수신·프레임률·RC 바퀴 동작은 업데이트 설치 후 사용자 확인이 필요하다. 영상 캡처는 추가 렌더링 부하가 있으며 앞선 723,420개 모델 예산이 전체 GPU 패스 수를 뜻하지 않는다.
- Pi /home/pi/dmix-dashboard에 server.py/app.js/test_server.py를 배포하고 dmix-dashboard만 재시작해 active를 확인했다. 이전 파일은 .runtime/backup-quest-stream에 보관했다. MQTT/RC 서비스는 재시작하지 않았다.
- 커밋 준비는 원본 .git 쓰기 제한으로 작업 폴더 work/github-publish-repo의 별도 Git 복사본에서 수행한다. GitHub 원격 게시와 Quest 재설치 완료는 별도로 확인한다.


## 2026-10-20 Quest 폴리곤 경량화 및 MQTT 종료 분석

- 사용자 APK 설치 성공 후 심한 렉과 앱 종료 보고. 75만 삼각형을 기준으로 집계·경량화를 요청했다.
- Unreal 엔진 API로 원본 맵 정적 모델 LOD0 인스턴스 합계 23,377,416개, 고래/물고기 197,608개, 활성 Lanzaman 탐사선 92,076개를 확인했다. 기본 합계 23,667,100개. 실제 화면의 GPU 드로 통계가 아닌 모델 집계다.
- 주 원인은 SM_Boulder01 123,976개 × 177개 배치(21,943,752개). 기존 메시 10개에 Quest용 LOD1을 생성하고 Android MinimumLOD=1을 지정했다. 바위 목표500개, 산호300개, 해초120개. PC LOD0 원본은 보존하며 새 LOD의 화면 크기는0으로 지정해 PC 자동 선택을 피한다. 원본 uasset 백업은 작업 폴더 work/quest-mesh-backup에 있다.
- 경량화 결과: 정적 모델 308,552개 + 스켈레탈 모델 197,608개 + 탐사선 92,076개 = 598,236개. 추진기 거품128개와 큐브 부품48개의 추가 예산 125,184개를 포함하면 723,420개로 750,000개 미만. 폰트/UI, 양안/그림자 패스, 시야/가림/LOD 및 다른 런타임 생성 요소에 따른 실제 GPU 처리량과 구분한다. Quest 실기 FPS/외관은 재설치 후 검증해야 한다.
- 충돌 로그 21:43:11은 MQTTCon 스레드 SIGSEGV. APK 심볼로 FMQTTConnection::UpdateConnection의 CONNECT 타임아웃 처리(PendingConnectOperation.Reset)로 해석했다. 메모리 부족 종료나 폴리곤 과다에 의한 종료로 단정하지 않는다.
- MQTTClient::Connect의 완료 continuation이 소켓 스레드에서 마지막 클라이언트 참조를 해제할 수 있어, 참조와 OnConnect 알림을 게임 스레드로 이전했다. 접속 거절/오류를 Connected로 표시하던 분기도 수정했다. 루프백 TCP 연결에 CONNACK를 주지 않고 외부 참조를 해제하는 MQTT.Client.ConnectTimeoutLifetime 회귀 검사를 추가했다. 실물 브로커/모터에는 접속하지 않는다.
- 검증: Windows Editor 빌드 성공, MQTT.Client.ConnectTimeoutLifetime 회귀 검사 Success. 검사 전 헤드리스 엔진 초기화 Condition failed 로그4건은 검사 결과와 구분한다. Android C++ 빌드·ASTC 쿠킹·APK 패키징 UAT ExitCode0, APK v2 서명 및 ZIP CRC 통과. 새 APK 263,856,384 bytes, SHA-256 36e040b99573f97bdf05149876e42037ba51a2c49127cbfea1700a95624d3530. Quest 실기 FPS·외관·충돌 해소는 업데이트 설치 후 확인 필요. 기기에 설치된 APK는 자동 변경되지 않는다. GitHub 미게시.


## 2026-10-20 Quest 3 독립 실행 APK

- 최신 사용자 요청: Wi-Fi/Air Link 끊김을 줄이기 위해 Quest 3에 USB로 설치할 독립 실행 APK를 제작한다. 과거 APK 범위 제외 기록을 대체한다. 사용자가 에디터 저장·종료와 UE 5.7.4 Android 구성요소 설치를 승인했다.
- UE 5.7.4, ARM64, Vulkan ES3.1, ASTC, Min API29/Target API34, Development 서명 빌드. 맵 데이터를 APK 안에 포함한다. 패키지 com.he4th.dmix, 표시 이름 D-Mix Underwater. 별도 OBB 복사는 필요 없다.
- 결과: C:\Users\user\Documents\Codex\2026-10-20\https-github-com-he4th-digital-twin\outputs\Quest3\Android_ASTC\MqttVehicle-arm64.apk (270,426,448 bytes). SHA-256: 2dc27ea58b97e5e59ec09f1ec4a04512b7ed37f0ea426206607c1b384b0d345a. 산출물은 GitHub 게시 대상이 아니다.
- Config/Android/AndroidEngine.ini에서 모바일 Forward·HDR 끔·Multiview·MSAA4 및 레이트레이싱/Lumen/가상 그림자/볼류메트릭 포그 끔을 적용한다. AndroidGame.ini는 VR 시작과 해저 맵/런타임 로드 재질의 쿠킹 범위를 지정한다. PC 렌더링 설정과 맵 원본을 보존한다.
- RawInput/PixelStreaming2/PixelCapture는 Win64만 활성화하고 Build.cs 의존성과 관전자 코드도 같은 조건으로 컴파일한다. Android에서는 관전자 및 반대 시점 캡처를 실행하지 않는다. 기존 조이스틱·트리거·오른손 A 상승/B 하강·그립 시점 전환과 MQTT 기본 설정은 유지한다.
- 독립 APK는 Air Link/노트북 렌더링 없이 실행하도록 구성했다. RC MQTT에는 Quest와 Pi 10.95.86.138의 네트워크 연결 및 Pi 수신기가 필요하다. PC Pixel Streaming 영상과 에디터 Python 상태/운영 제어 브릿지는 APK에 포함되지 않는다. Pi 웹서비스 자체는 그대로 유지한다.
- 검증: Windows Editor와 Android C++ 컴파일·링크 성공. 해저 맵 Android_ASTC 쿠킹 성공(오류0/경고17; 헤드리스 PC OpenXR 및 인증서 저장소 관련). Java 상위 경로 조회(toRealPath)가 자동 실행 환경에서 차단되어 마지막 조립은 사용자가 일반 PowerShell에서 Finish-Quest-Build.ps1을 실행했다. UAT ExitCode0, APK v2 서명·ZIP CRC·ARM64/OpenXR 라이브러리·VR manifest·내장 맵 데이터를 확인했다. Quest에 설치한 실제 화면/프레임률/컨트롤러/MQTT 동작은 아직 미검증이며 모터 시험을 추가 수행하지 않았다.
- 도구: Android NDK 27.2.12479018(r27c), SDK34/build-tools35.0.1, OpenJDK21, Gradle8.7. 공식 배포 다운로드와 해시를 확인했다. 작업 폴더 work/android에 격리했고 전역 JAVA_HOME/SDK 환경변수는 변경하지 않았다. 재빌드: C:\Users\user\Documents\Codex\2026-10-20\https-github-com-he4th-digital-twin\outputs\Finish-Quest-Build.ps1 -FullBuild. 마지막 조립만 재시도할 때는 인수 없이 실행한다. work 폴더에 도구·캐시·빌드 스크립트가 있으므로 보존한다.
- GitHub 미게시. 사용자는 검증 후 변경 목록을 보고 게시 여부를 결정하기로 했다. 새로운 저장소 파일은 AndroidEngine.ini/AndroidGame.ini 두 개다. 인증정보·키·SDK·캐시·로그·APK는 게시하지 않는다.


## 2026-10-20 Pi 웹서버 부팅 자동 실행 및 16:9 관전자 카메라

- 사용자 선택: 웹 대시보드/영상 연결 서버를 Pi로 이전하고 부팅 자동 실행, 별도의 가로형 관전자 카메라 추가. 사용자가 Unreal 종료를 확인한 뒤 빌드했으며 이후 에디터를 다시 열고 브릿지/VR Preview를 실행했다.
- 새 운영 주소는 http://10.95.86.138:5050 이다. Pi /home/pi/dmix-dashboard에 기존 대시보드 소스를 배포하고 Node.js 20.19.2/npm 9.2.0을 설치했다. Python paho-mqtt는 기존 설치를 확인했다. 영상 번들은 Pi에서 npm ci --ignore-scripts와 npm run build로 생성했다.
- /etc/systemd/system/dmix-dashboard.service 및 dmix-pixel.service를 등록했다. User=pi, Restart=on-failure, multi-user.target 부팅 활성화. 각각 웹5050, 신호 연결8888/8889를 사용한다. 두 서비스 enabled/active 및 서비스 재시작 후 정상 응답을 확인했다. Pi 자체 재부팅은 수행하지 않았다. MQTT 브로커는 기존 enabled/active, RC 수신기는 기존 임시 사용자 서비스 그대로이며 부팅 자동 실행은 별도다.
- server.py는 명시한 public-host만 Host/Origin에 추가하고 Pi의 .runtime/bridge-token을 지속 보관한다. 인증 파일은 HTTP 경로로 제공하지 않는다. 재시작 후 인증 동일성 확인. 노트북의 Git 제외 .runtime/remote-bridge.json에 SSH로 연결 정보를 복사했고 connect_local_dashboard.py가 우선 읽는다. 토큰/키/비밀번호는 문서와 저장소에 포함하지 않는다. Pi 주소가 바뀌면 서비스·Unreal ConnectionURL·허용 호스트·연결 파일을 함께 갱신한다.
- Pixel Streaming 연결 URL은 ws://10.95.86.138:8888. 웹 플레이어는 접속한 웹페이지의 호스트에 있는 8889로 연결한다. 웹페이지/영상 연결 서버만 Pi에 있으며 실제 Unreal 렌더링/인코딩/VR에는 노트북이 필요하다. Unreal 에디터 세션마다 사용자가 `py "C:/Users/user/Documents/d-mix/unreal/tools/connect_local_dashboard.py"`로 상태/운영 제어 브릿지를 연결한다.
- 기존 SubmersibleViewComponent에 별도 SceneCapture와 GPU 프레임 producer를 추가했다. 현재 카메라 위치/방향 및 내외부 숨김 규칙을 따르는 1280x720, 수평 FOV100도, 최대30fps 단안 영상이다. 시청자가 있을 때만 추가 렌더링하고 Play 종료 시 이전 producer/FPS로 복원하며 GPU 명령을 비운 뒤 리소스를 해제한다. 기존 반대쪽 카메라 피드/헤드셋 양안 렌더링/차량 제어는 유지한다. GamePreview 자동화·렌더링 불가 실행에서는 관전자 캡처를 시작하지 않는다.
- 대시보드 기본값을 '관전자 전체 화면'으로 바꾸고 기존 한쪽 눈 잘림 선호를 이전하지 않았다. 과거 스테레오 송신용 한쪽 눈 옵션은 별도로 남겼다. 양쪽 눈 영상을 합성한 파노라마가 아니라 독립 가로 투영이다. GPU 부하는 추가되므로 실제 VR 성능은 사용 중 확인해야 한다.
- 검증: UE 5.7.4 빌드/링크 성공, PixelCapture 의존성 경고 정리. UnderwaterMovement Success(검사 오류0/경고40), 엔진 초기화의 별도 Condition failed 로그4건 및 DDC 쓰기 제한은 검사 결과와 구분한다. Python HTTP/상태 검사6개는 노트북·Pi 모두 통과했다. 허용 LAN 호스트/Origin/브릿지 인증과 잘못된 요청 거부 포함. 실제 Pi 웹페이지에서 Play 상태·MQTT·RC 상태 및 1280x720 live 영상(readyState4, currentTime 진행)을 확인했고 한쪽 눈 CSS 잘림은 꺼져 있다. 브라우저 오류 로그 없음. 모터 시험은 추가로 수행하지 않았다.
- 기존 Windows 서버 종료 시 실행기의 프로세스 시작 시각 조회가 null로 실패했다. Pi 서비스/연결에는 영향 없으며 기존 노트북 서버 프로세스 종료까지 확인한 것은 아니다. 운영은 새 Pi 주소를 사용한다. GitHub 미게시.

## 2026-10-20 대시보드 VR 한 화면 표시

- VR Preview의 좌우 눈 영상이 Pixel Streaming으로 나란히 전달되는 것을 확인했다. 대시보드의 기본 영상 보기를 'VR 한 화면'으로 변경해 왼쪽 눈 영역만 원래 비율로 중앙 표시한다. 두 눈을 겹치거나 가로로 늘리지 않으며, 화면 비율에 따라 여백이 생길 수 있다.
- 탐사 화면에 '영상 보기' 선택기를 추가했다. 일반 PC 영상은 'PC / 원본 화면'으로 전환하며 선택은 해당 브라우저에 저장된다. VR 상태를 자동 판정하는 기능은 아니다. 영상 메타데이터/해상도/플레이어 크기 변경 시 표시 크기를 다시 계산한다.
- raspberry-pi/dashboard/index.html과 app.js의 표시 처리만 변경했다. 헤드셋 렌더링·Unreal 설정·Pixel Streaming 라이브러리·MQTT/모터 동작은 변경하지 않았다. 서버 재시작/라이브러리 번들 재빌드 없이 대시보드를 새로고침하면 적용된다.
- 검증: 로컬 대시보드의 실제 3648x1968 VR 스트림에서 한쪽 눈 표시를 스크린샷으로 확인했다. 원본 모드 전환 시 잘림 해제와 contain 비율 표시, VR 모드 복귀를 확인했다. Node 구문 검사 통과, 브라우저 오류 로그 없음. 운영 상태/제어 연결은 영상 연결과 별개이며 기존 상태 그대로다. GitHub 미게시.

## 2026-10-20 자동복귀 RC 바퀴 연동 활성화

- 수동 바퀴 회전 확인 후 사용자가 자동주행에서는 바퀴가 돌지 않는다고 보고했다. DefaultGame.ini의 bPublishAutomaticReturn=False가 원인이 되는 송신 분기를 확인했다. 기존 브릿지는 이 값이 True이고 탐사 제어 중일 때 GetAutomaticVehicleControlInput()을 송신한다. False일 때 자동 제어의 수동 입력은 0이다.
- 최신 요청에 따라 기준 프로젝트의 bPublishAutomaticReturn=True로 영구 변경했다. bPublishVehicleInput=True, bReceiveVehicleCommands=False, 브로커 10.95.86.138:1883 및 기존 시험 토픽은 유지한다. 이전 자동복귀 송신 기본값 False 기록을 대체한다.
- 현재 열린 에디터에는 Play를 중지한 뒤 사용자가 Unreal Cmd에서 `py "C:/Users/user/Documents/d-mix/unreal/tools/rc_session_start.py"`를 실행하면 같은 설정이 적용된다. 기존 스크립트를 그대로 사용하며 C++ 재빌드가 필요 없다. 다음 에디터 시작부터는 저장된 설정을 읽는다.
- 범위는 가상 자동복귀의 수평 전진·회전을 바닥에서 띄운 RC카 바퀴로 재현하는 것이다. 실제 RC카의 위치 측정·경로 복귀를 구현한 것은 아니다. 수직 이동은 실물 모터에 연결하지 않는다. 기존 포커스 상실·정지·도착·입력 유효시간·Pi 통신 단절 정지를 유지한다.
- 설정 파일/기존 실행 스크립트/자동복귀 출력 분기를 정적으로 확인했다. 사용자가 현재 에디터에 세션 설정을 적용한 뒤 자동복귀 시험에서 실제 바퀴가 돈다고 확인했다. 수동 조작과 자동복귀의 고정 RC카 바퀴 연동 모두 사용자 확인 완료이며, 실제 지상 경로 복귀 검증과는 구분한다. Pi 수신기는 앞선 작업에서 20% 상한의 백그라운드 세션으로 실행했다. GitHub 미게시.

## 2026-10-20 Pi RC 수신기 복구 및 실물 바퀴 회전 확인

- 사용자가 바퀴를 바닥에서 띄운 상태로 모터 시험을 승인했다. SSH 공개키 등록 후 AI가 Pi 10.95.86.138에 직접 접속했다. 자격증명은 저장소에 기록하지 않는다.
- 최초 확인에서는 수신기가 --motors로 실행 중이었으나, 이후 미동작 보고 시 해당 프로세스가 종료되어 있었다. 종료 원인은 확인하지 못했다. 브로커는 active이고 Unreal의 명령은 도착했다.
- Pi에서 임시 사용자 systemd 서비스 dmix-rc-session.service로 /usr/bin/python3 -u /home/pi/rover_drive.py --motors --broker localhost --command-topic codex/ue-bench/20261019/cmd --state-topic codex/ue-bench/20261019/state --max-speed 0.2 를 실행했다. SSH 종료 후 별도 접속에서도 active/running과 상태 수신을 확인했다. 부팅 자동 시작이나 자동 재시작은 설정하지 않았다.
- 45초 관측: 명령 410건, 상태 222건, 전진 최대 th=1, 명령 출력 최대 0.2, dry_run=false. 상태 중 command 213건, timeout 9건이며 기존 0.3초 통신 단절 정지를 유지했다. 상태의 l/r은 측정 속도가 아닌 설정 출력이다.
- 복구 후 사용자가 전진 시험에서 실제 바퀴가 돈다고 확인했다. 지상 주행·모든 바퀴 방향·장시간 안정성 검증까지 완료한 것은 아니다. 현재 수신기 실행을 유지한다.
- 종료: PowerShell에서 `ssh pi@10.95.86.138 "systemctl --user stop dmix-rc-session"`. 실행 확인: `ssh pi@10.95.86.138 "systemctl --user is-active dmix-rc-session"`. 수신기를 중복 실행하지 않는다. 임시 사용자 서비스이므로 Pi 재부팅/사용자 관리자 종료 후에는 다시 시작해야 한다.
- Unreal/Pi 소스 변경 없음. 운영 기록과 파일 카탈로그만 갱신하며 GitHub 미게시.

## 2026-10-20 VR 조종석 기준점·오른손 그립 시점 전환·RC 진단

- 사용자 보고: RC카 미동작, VR 1인칭이 탐사선 외부 왼쪽 위에 위치. 오른손 그립을 Tab과 같은 시점 전환으로 요청했다. Pi 수신기 실행 여부는 사용자가 모르는 상태이며 아직 미확인이다.
- VR 카메라: 최초 유효한 로컬 HMD 추적 시 Local(앉은 시점) 원점으로 전환하고 위치/방향을 재중앙화한다. 기존 조종석 눈높이에 방 좌표의 높이/좌우 위치가 더해지는 문제를 방지한다. 전방 카메라 암 길이/소켓 오프셋을 0으로 정리하고 HMD 추적/late update는 유지한다. R로 다시 중앙을 맞출 수 있다. 실제 Quest 조종석 위치는 사용자가 재실행해 확인해야 한다. XRBase/HeadMountedDisplay 의존성과 XRBase 플러그인을 명시했다.
- 오른손 Grip_Click은 우선순위10의 소비형 Boolean 액션으로 시점을 전환한다. 누르는 순간 한 번만 실행하며 기존 Tab/A/B/E/Q는 유지한다. DefaultInput의 같은 그립 handbrake 항목도 SwitchCamera로 정리했다. 자동화 검사에서 낮은 우선순위의 동일 그립 handbrake를 일부러 추가해도 브레이크가 걸리지 않는 것을 확인했다.
- 최신 사용자 로그(19:03:30 KST)는 10.95.86.138:1883 연결 성공과 vehicle output=1을 보여준다. 실제 Pi 구독/수신/모터 작동 성공까지 의미하지 않는다. 연결 시 목적지 토픽, 입력 변경 시 QoS0 로컬 송신 완료 값을 로그로 남기도록 추가했다. 로그는 수신기 확인과 구분한다. Pi 터미널에서 `pgrep -af '[r]over_drive.py'` 결과를 사용자에게 요청한 상태다.
- 검증: UE 5.7.4 Editor 빌드·링크 성공. 화면/HMD 없는 GamePreview의 UnderwaterMovement 검사 Success, 오류0, DDC 쓰기 제한 경고43건, 프로세스 종료0. A/B·키보드·그립 누름/유지/해제/재누름·브레이크 충돌·기존 수중 이동 검증. 로컬 Pi 수신 코드 오프라인 검사18개 통과. 실제 브로커 접속·Pi 접속·모터 실행 및 실기 HMD 시점 검증은 AI가 수행하지 않았다.
- 에디터 재시작 후 정면을 보고 VR Preview 실행. MQTT 자동 연결 설정은 유지한다. GitHub 미게시.

## 2026-10-20 MQTT 자동 연결·수동 입력 송신 기본 활성화 (최신)

- 사용자가 매번 세션 시작 명령을 입력하지 않도록 요청했다. 기준 DefaultGame.ini의 bPublishVehicleInput=True, BrokerHost=10.95.86.138, BrokerPort=1883, CmdTopic=codex/ue-bench/20261019/cmd로 영구 설정했다. 과거 기본 송신 False 기록은 이 요청으로 대체된다. 수신과 자동복귀 송신 기본값은 False로 유지하며 A/B 수직 이동은 가상 환경에만 적용된다.
- 에디터 재시작 후 Play/VR Preview의 Game/PIE 월드 초기화에서 자동 연결한다. 레벨 편집 상태에서는 연결하지 않는다. 기존 2초 재접속 설정을 유지한다. 별도 rc_session_start.py 실행 없이 수동 조작 송신을 시작하지만 Pi 수신 프로그램은 별도로 실행 중이어야 한다.
- 현재 에디터 세션만 끄려면 Unreal Cmd에서 `py "C:/Users/user/Documents/d-mix/unreal/tools/rc_session_stop.py"` 실행. 실행 중인 Play도 종료 요청한다. 영구 설정을 저장하지 않으므로 다음 에디터 재시작 시 자동 활성화된다. 계속 끄려면 DefaultGame.ini의 bPublishVehicleInput=False로 되돌린다. 이전 rc_session_start.py는 자동복귀 송신도 활성화하는 별도 세션 기능임에 유의한다.
- 설정 값·토픽 일치·초기화/재접속 코드 정적 확인 완료. 설정 변경이므로 C++ 재빌드는 필요 없다. AI가 실제 브로커 접속·Play·Pi/모터 실행을 검증하지 않았다. AGENTS/README/인계/상태/카탈로그 갱신, GitHub 미게시.

## 2026-10-20 Quest 오른손 A/B 상하 이동

- 사용자 확인: 퀄리티를 낮춘 뒤 VR 지연이 크게 개선됐으며 조이스틱 좌우 조작·검지 트리거 전진이 동작한다. 이번에 오른손 A를 상승, B를 하강으로 기존 Enhanced Input 수중 액션에 추가했다. 기존 E 상승/Q 하강은 유지한다. 누르는 동안 이동하고 놓으면 현재 깊이를 유지하며 A+B 동시 입력은 상쇄된다. 기존 정지·수동 전환·충돌·상승 한계 처리를 그대로 사용한다.
- UE 5.7.4 Editor 빌드 성공 및 프로젝트 DLL 반영 완료. `MqttVehicle.Control.UnderwaterMovement` 자동화 검사에서 기존 키보드 동작과 A 상승/B 하강·A 해제·동시 입력·정지 후 B 유지·수직 입력의 MQTT 출력 0을 검증했다. 테스트 Success, 오류 0, DDC 캐시 쓰기 제한 경고 43건, 프로세스 종료 코드 0. 실제 Quest 버튼으로 변경 후 실행은 사용자 확인이 남아 있다.
- 현재 OpenXR 로그는 기본 매핑 컨텍스트가 없어 legacy key-event 경로를 사용한다. 설치된 UE 5.7 OpenXR 소스에서 Touch A/B를 포함한 XR 키 이벤트 등록을 확인했으며 이 이벤트를 기존 런타임 Enhanced Input 컨텍스트가 받도록 연결했다. 향후 OpenXR 기본 IMC로 이전할 때 해당 컨텍스트에도 A/B 등록이 필요하다.
- MQTT 송수신 기본값 변경·Pi 접속·모터 명령 없음. 에디터를 다시 열어 VR Preview에서 확인한다. 기존 소스/자동화 검사/README/인계/상태/파일 카탈로그만 수정했으며 GitHub 미게시다.

## 2026-10-20 대시보드 백그라운드 실행

- `raspberry-pi/dashboard/start.cmd`와 `start.ps1`은 대시보드/영상 서버를 숨김 프로세스로 시작하고 준비 확인 후 반환한다. 실행 완료 후 PowerShell 창을 닫아도 서버는 유지된다. 종료는 `start.cmd -Stop` 또는 `powershell -ExecutionPolicy Bypass -File "C:\Users\user\Documents\d-mix\raspberry-pi\dashboard\start.ps1" -Stop`이다. 구형 전경 서버는 기존 창에서 종료한 뒤 새 실행기를 사용한다. 재부팅 자동 시작은 추가하지 않았다.
- 실행 정보는 Git 제외된 `.runtime/background-processes.json`에 기록하며 PID·시작 시각·실행 파일을 대조해 종료한다. Python 가상환경의 실제 자식 인터프리터 PID도 bridge.json으로 전달해 추적한다. 같은 설정의 중복 실행은 재사용하고, 다른 설정은 먼저 종료하도록 안내한다. 이미 실행 중인 외부 영상 서버를 재사용하면 종료 대상에 포함하지 않는다. 기본 브로커는 사용자 지정 10.95.86.138로 맞췄다.
- 검증: Windows PowerShell 실행기 종료 후 별도 포트5051 HTTP 200, 중복 실행 시 PID 유지, -Stop 후 포트 반환 확인. 번들 Python과 -NoMqtt를 사용했으며 기존 영상 서버를 재사용했다. 기존 서버 단위검사 5개 통과. 사용자 설치 Python 가상환경/새 영상 서버의 전체 실행은 별도 미검증이다. 검사 종료 후 기존 bridge.json 복원. 실제 MQTT/Pi 접속·모터 명령 없음.
- README/인계/상태/파일 카탈로그 갱신. 기존 파일만 로컬 수정, GitHub 미게시. 서버 재시작 뒤 Unreal 운영 상태/버튼 연결에는 `py "C:/Users/user/Documents/d-mix/unreal/tools/connect_local_dashboard.py"` 재실행이 필요하다.

## 2026-10-20 현재 MQTT 브로커 주소 변경

- 사용자가 현재 브로커 주소를 `10.95.86.138`로 확인했다. 기준 프로젝트 `unreal/MqttVehicle/Config/DefaultGame.ini`의 BrokerHost와 `unreal/tools/rc_session_start.py`의 세션 주소를 함께 변경했다. 아래 과거 기록의 192.168.0.8은 이전 주소다.
- MQTT 송수신 기본값 False, 포트 1883 및 기존 토픽은 유지했다. 시작 스크립트는 Play 중지 상태에서 사용자가 실행하면 해당 에디터 세션의 송신을 활성화한다. 이미 열린 에디터에도 스크립트 실행으로 새 주소가 적용된다.
- 파일 값 확인 완료. 실제 브로커 접속·Play·Pi/모터 실행은 수행하지 않았고 GitHub 미게시다.

## 2026-10-20 MQTT OpenSocket 충돌 수정 (최신)

- 사용자 보고: 에디터 실행은 성공했으나 MQTT 시작 후 `FMQTTConnection::OpenSocket`에서 접근 위반으로 종료됐다. 코드에서 연결 작업 스레드 종료 전에 소켓을 삭제하고 대기 작업을 정리하는 수명 경쟁을 확인했다. 보고된 스택과 부합하는 결함이며 해당 사용자 세션의 덤프 분석은 수행하지 않았다.
- `Plugins/MQTT/Source/MQTTCore/Private/MQTTConnection.cpp`: Stop은 종료 상태만 설정하고, 소멸자는 작업 스레드 종료를 기다린 뒤 대기 작업과 소켓을 정리한다. 상태를 명시적으로 초기화하고 Stopping에서 다른 상태로 복귀하지 못하도록 원자적 전이를 사용한다. OpenSocket 취소 판정도 실제 MQTT 종료 상태를 사용한다.
- 검증: UE 5.7.4 MqttVehicleEditor UBT 빌드 성공(종료 코드 0), 프로젝트 MQTTCore DLL 반영 완료. 기존 XmlConfigCache 지정으로 AI 환경의 AppData XML 쓰기 제한을 피했다. `MQTT.Connection.ShutdownDuringConnect` 자동화 검사는 127.0.0.1의 비수신 포트로 연결 중 종료를 8회 반복하며 종료 상태 유지/2초 이내 종료를 확인했다. 테스트 1개 성공, 오류·경고 0, 프로세스 종료 코드 0. 화면 없는 에디터 초기화에는 별도 DDC/웹 브라우저 환경 경고가 있었으며 테스트 결과와 구분한다.
- 이전 로그 카테고리 수정 후 에디터 실행 성공은 사용자 확인이다. 이번 수정 후 실제 맵 Play·실제 브로커 MQTT 통신은 미검증이다. Pi 접속·모터 명령 없음, 기본 MQTT 송수신 False 유지. 에디터를 다시 열면 새 DLL이 적용된다. 시작 스크립트의 브로커 192.168.0.8은 현재 네트워크에서 별도 확인이 필요하다.
- 기존 소스/README/인계/상태/파일 카탈로그만 로컬 수정했다. GitHub 미게시.

## 2026-10-20 노트북 컴파일 오류 수정

- UE 5.7.4 Unity 빌드에서 공통 `LogMqttVehicle`과 브릿지의 정적 동명 정의가 충돌한 C2011을 수정했다. `MqttVehicleBridgeSubsystem.cpp`는 `MqttVehicle.h`를 직접 포함하고 기존 공통 로그 정의를 사용한다.
- 사용자 빌드가 생성한 응답 파일과 같은 컴파일/링크 옵션으로 해당 Unity 모듈과 DLL 생성 성공(각 종료 코드 0). 생성물은 작업용 임시 폴더에 분리했으며 전체 UBT 재빌드·에디터 열기·Play 확인은 남아 있다.
- 프로젝트는 계속 EngineAssociation 5.7이다. 기능·MQTT 설정·맵 변경, Pi 접속·모터 실행, GitHub 게시는 하지 않았다.

## 2026-10-20 GitHub 기준 소스의 H키 복귀 대기 제거

- 원격 기준 81625cc에는 데스크톱 수정 기록만 있고 실제 소스는 Countdown=5초였다. 이번에 UserRequest는 0초, 고래 발견/위험 구역 이벤트는 기존 5초로 분기하도록 기준 소스를 수정했다. H키와 같은 RequestReturn 경로를 사용하는 대시보드 복귀 요청에도 적용된다.
- 고정 대기만 제거한다. 다음 제어 틱에서 차량 정지를 확인한 뒤 경로를 계산하며 충돌 검사·경로 계산 시간과 사용자 정지 보호는 유지한다. Pi 출력 상한·MQTT 설정은 변경하지 않았다.
- 이번 노트북 작업은 소스 분기와 기존 정지 조건의 정적 검토만 수행했다. Unreal 빌드·Play·자동화·Pi/모터 실행은 하지 않았다. 아래 데스크톱 빌드 성공 기록은 별도 작업 결과다. 다른 PC에서는 업로드 후 최신 소스를 내려받아 Editor를 다시 빌드해야 적용된다. 2026-10-20 사용자가 이 소스와 관련 문서의 GitHub 업로드를 승인했다.


## 2026-10-20 Pixel Streaming 자동 영상 연결 (최신)

- 수동 창 선택 화면 공유를 Epic UE5.7 Pixel Streaming 2로 교체했다. raspberry-pi/dashboard/start.cmd가 로컬 서버와 영상 연결 서버를 시작한다. Unreal 에디터 Play 시작 시 자동 송출하며 대시보드는 자동 구독·재연결한다.
- 최초 실행은 Python 3와 Node.js 22 LTS, 인터넷이 필요하다. setup_pixel.ps1이 고정 버전 의존성 설치와 플레이어 번들을 생성한다. 5050/8888/8889는 loopback 전용이다. 원격 키보드·마우스·게임패드 입력은 끈다.
- 프로젝트 DefaultGame.ini의 PixelStreaming2 설정과 플러그인을 활성화했다. EditorStartOnLaunch=False여서 편집 화면은 송출하지 않고 Play 때 시작한다. 독립 실행용 AutoStartStream=True도 지정했다. 패키징 앱 및 VR 헤드셋 미러링은 미검증이다.
- 화면 없는 Unreal Play → Edge WebRTC 실제 영상 디코딩(1014×552)을 확인했다. 서버 검사 5개 통과, 설치/번들 생성 통과. 검증 중 Unreal MQTT 송수신 False; RC 모터 제어 없음.
- 영상은 자동 연결되지만 운영 상태/버튼 bridge는 기존 connect_local_dashboard.py 실행이 별도로 필요하다. 서버 재시작 시 토큰도 갱신된다.
- 사용자 승인 범위: pixel 서버/플레이어/실행기/프로젝트 설정/대시보드/관련 문서 게시. 아래 과거 수동 화면 공유 기록은 이 구현으로 대체된다.


## 2026-10-20 대시보드 Unreal 화면 공유

- 영상 영역에 Unreal 화면 연결/연결 해제 버튼과 muted video를 추가했다. getDisplayMedia로 사용자가 선택한 창/화면을 브라우저 안에서만 재생한다. 녹화·서버 업로드·모터 명령은 없다. 화면 선택은 사용자 직접 권한 승인 필요.
- 취소 시 기존 공유 유지, 명시 해제·브라우저 공유 종료·페이지 이탈 시 track 정리, 미지원 브라우저는 Chrome/Edge localhost 안내를 표시한다. 화면은 object-fit:contain으로 비율을 보존한다.
- 서버 미디어 CSP 적용 후 로컬 서버 재시작 및 HTTP200 확인. JS 문법·연결/취소/해제/종료 모의 검사 통과, 서버5개 검사 통과. 실제 사용자 Unreal 창 캡처는 아직 미검증이다. 서버 재시작으로 Unreal 운영 bridge 토큰이 갱신되어 필요 시 connect_local_dashboard.py 재실행이 필요하다. GitHub 미업로드.


## 2026-10-20 대시보드 영상 중간 크기 조정

- 사용자 요청으로 전체 폭을 최대1560px·가운데 정렬, 오른쪽 패널340px로 조정했다. 영상 높이는340~460px 반응형이며 작은 화면은260px다. 최초1200px/310px와 직전 전체너비/16:9 확대 사이의 크기로 조정했다. 로컬 반영, GitHub 미업로드.


## 2026-10-20 대시보드 영상 영역 확대

- raspberry-pi/dashboard/index.html의1200px 최대 폭을 제거하고 좌우 여백24px로 전체 너비를 사용한다. 오른쪽 상태 패널은320px, 나머지는 영상 영역에 배정한다.
- 영상 자리16:9 비율·최소 높이420px,900px 이하 화면에서는 단일 열·최소 높이220px로 전환한다. 영상 스트리밍은 여전히 미연결이다. 로컬 수정이며 GitHub 미업로드.


## 2026-10-20 로컬 해저 탐사 대시보드

- 새 raspberry-pi/dashboard/에 로컬 서버·화면·Windows 실행기·검사를 구현했다. http://127.0.0.1:5050 에서 실행하며 외부 CDN 없이 표시한다. start.cmd 첫 실행은 Python 가상환경과 paho-mqtt 설치가 필요하다.
- MQTT 브로커와 실제 Pi state 토픽을 구독만 한다. 신선한 수신기 출력과 브로커 연결을 구분하고1.5초 무응답은 값을 숨긴다. 서버는 차량 명령을 MQTT로 발행하지 않는다.
- unreal/tools/connect_local_dashboard.py로 에디터를 연결하면 Play·가상 속도·복귀 상태/이벤트를 표시한다. 수동 전환·자동복귀·정지는 기존 Unreal 함수를 호출한다. 2초 만료/응답 표시 및 loopback·Origin·bridge token 검사를 사용한다. 기본 MQTT 송수신은 변경하지 않는다.
- 검증: 서버 검사5개 통과, 실제 Pi MQTT 구독 및0/0 텔레메트리 수신, 브라우저 화면/미연결 버튼 비활성화 확인. 별도 화면 없는 실제 Play에서 MQTT를 끄고 운영3개 함수 응답 및 속도0.0/복귀 상세 상태 전달 확인. 최초 get_pawn API 오류는 GameplayStatics.get_player_pawn으로 수정했다.
- 사용자가 연결 스크립트를 실행한 뒤 에디터를 종료한 로그를 확인했다. 다음 에디터 실행 시 다시 연결 스크립트를 실행해야 한다. 서버 재시작도 토큰 변경으로 재연결이 필요하다. 실제 차량 동작 검사는 하지 않았다.
- 영상은 미연결 안내만 표시한다. 실제 영상/센서RPM·배터리/패키징 앱/VR 연결 감지는 아직 미구현이다. 브라우저로 포커스 이동 시 기존 Unreal 정지 정책을 유지한다. 독립 하드웨어 비상정지 기능이 아니다.
- raspberry-pi/dashboard/.runtime 토큰·.venv는 Git 업로드 및 파일 카탈로그 제외. 새 파일과 관련 문서의 GitHub 업로드 사용자 승인 완료.


## 2026-10-20 H 복귀 준비 대기 제거 (빌드 완료)

- H 사용자 복귀에도 적용되던 Countdown=5초를 UserRequest일 때0초로 변경했다. 고래 발견/위험 구역 이벤트 대기는 유지한다. 다음 제어 틱에서 정지를 확인한 뒤 경로 계산을 시작하므로 경로 계산/충돌 검사 시간은 남는다.
- Editor 빌드 성공. 모터 송신 없는 PlannedReturn 회귀 검사 통과. 실제 H 키 입력 후 체감 지연은 사용자 확인 대기이며 경로 계산 시간은 보존한다. GitHub 미업로드. Pi 및 모터 제어 변경 없음.


## 2026-10-20 제자리 회전 기동 보완 (Pi 적용 완료)

- 사용자가 자동복귀 연동은 작동하나 회전 시 모터 소리만 나고 바퀴가 움직이지 않는다고 보고했다. 회전 상한0.12와 느린 가속이 기동에 부족할 가능성에 따라 PIVOT_MAX_SPEED를 전진과 같은0.2로 변경하고 회전 가속 램프를 제거했다. 실제 기동 원인/RPM은 계측하지 않았다.
- 정지에서 최대 조향 입력 시 즉시 좌우 +/-0.2. 작은 조향 입력은 비례 출력이며 실제180도 각도 제어는 아니다. 반대 부호 전환은0까지 감속, 중립/오류/단절 정지는 유지한다. 오프라인18개 통과(첫 회전 제어주기에서0.2 도달 포함).
- 사용자의 Play 중지 확인 후 기존 수신기를 정상 종료하고 Pi /home/pi/rover_drive.py를 교체했다. 문법 검사 통과, 수정 수신기 PID1890의 MQTT 구독 및 출력0 대기를 확인했다. 별도 모터 명령은 보내지 않았다. 실제 바퀴 기동 재확인 대기, GitHub 미업로드.


## 2026-10-20 자동복귀 송신 미활성 원인 수정

- 사용자 에디터 로그 00:47:46/50(KST)은 이전 `.upload-tools/arm_rc_bench.py` 실행을 기록했다. 해당 임시 파일은 publish_vehicle_input만 켜고 publish_automatic_return을 켜지 않아 자동복귀 입력이 제외됐다. Pi 수신기는 실행 중이며 수동 명령 수신/중립/timeout 로그를 확인했다.
- 기존 임시 진입점을 `unreal/tools/rc_session_start.py` 위임으로 교체했다. 이전 복사 명령도 이제 자동복귀 송신 설정을 함께 켠다. 임시 파일은 업로드 대상이 아니다. 새/옛 진입점 각각 자동송신 활성화·Play 중 실행 차단·종료 해제 모의 검사 통과, Pi 오프라인18개 재통과.
- 사용자에게 Play 정지 후 연결 스크립트 재실행을 요청했다. 실제 Unreal→Pi 자동복귀 출력과 바퀴 동작 확인은 아직 대기 중이다. 별도 모터 명령 발행 없음. 이번 문서 변경은 GitHub 미업로드.


## 2026-10-20 자동복귀 MQTT 동작 동기화

- 사용자 요청으로 수중 자동복귀의 수평 전진과 제자리 회전을 RC 명령에 연결했다. 새 `bPublishAutomaticReturn`은 기본 False이며 `unreal/tools/rc_session_start.py`가 메모리에서만 활성화하고 종료 스크립트가 해제한다. 수동 getter는 기존 동작을 유지한다.
- Pawn은 실제 가상 이동의 수평 속도/복귀 기준 속도를 th(0..1), 적용한 yaw 변화/회전 속도를 st(-1..1)로 제공한다. 최초 경로 정렬·회전 및 도착 180도 회전 방향이 포함된다. 수직 이동 단독은 0이며 지형 충돌 시 전진0, 대기·중단·도착 종료 시0이다. 갱신 후0.2초가 지난 자동 명령은0이다.
- Bridge는 명시적으로 활성화된 자동복귀만 선택하며 기존 포커스·일시정지·통신 실패 정지를 유지한다. Pi 기존 전진0.2·제자리회전0.12 상한/단절정지를 사용한다. 이는 가상 동작을 따라 보내는 명령이며 실물 180도 회전각·거리·출발점 도착을 보장하지 않는다. 실제 공간 지도와 센서 피드백은 미구현이다.
- C++ Editor 빌드 성공. Pi 오프라인18개 통과. Unreal Control 자동화3개 통과(복귀 전진 출력·좌우 회전 방향·대기/중단 정지 검사 포함). 실제 Pi 접속·배포·모터 송신은 이번 작업에서 하지 않았다. 고정 차체 통합 검증은 남아 있다. GitHub 업로드 사용자 승인 완료(2026-10-20).


## 2026-10-20 RC 연결 세션 간소화와 정지 검사

- `unreal/tools/rc_session_start.py`: Play가 멈춘 상태에서 실행. 메모리 기본 객체만 송신 활성화하며 여러 Play에 유지한다. 이전 임시 5분 콜백을 제거한다. 설정 파일의 송수신 False는 유지하고 에디터 재시작 시 다시 명시적으로 켜야 한다. Pi를 시작하거나 연결 성공을 보증하지 않는다.
- `unreal/tools/rc_session_stop.py`: 이후 Play의 송수신을 끄고 진행 중인 Play는 종료 요청한다. 기존 C++ Deinitialize의 0 명령과 수신기 watchdog이 정지를 담당한다. 종료 요청만으로 실물 정지 확인이라고 기록하지 않는다.
- `raspberry-pi/rover/run_rc_session.sh`: Pi에서 같은 폴더의 수신기를 포그라운드로 실행. 기본 `--dry-run`, 고정 차체 실물은 명시적 `--motors`. 10분 제한 없이 Ctrl+C로 종료하며 부팅 자동실행은 하지 않는다. flock은 이 실행기를 통한 중복만 막으므로 이전 임시 수신기는 먼저 종료해야 한다. Pi 배포/실행은 아직 하지 않았다.
- 수신기는 SIGHUP도 정상 종료로 처리하도록 보완했다. 최대 전후진 0.2·제자리 회전 0.12 유지. 모터 없는 기존 검사18개 및 세션 스크립트 모의 검사(시작/Play 중 시작 차단/종료/반복 종료) 통과. 실제 에디터 실행·Pi 셸 실행·SIGHUP 실물 정지는 미검증.
- 정지 코드 검토: 키 해제는 0 명령, 포커스 이탈은 StopVehicleInput 후 0, Play 종료는 최종 0 송신 시도, 명령 단절은 Pi 0.3초 watchdog + 최대 약 0.05초 제어주기 지연. MQTT QoS0이므로 최종 패킷 수신을 보장하지 않는다. 네 항목의 이번 실제 Unreal→바퀴 통합 검사는 사용자 실행 대기다.
- 사용: Pi에서 `sh run_rc_session.sh --motors`(고정 차체 승인 범위만). Unreal Cmd 콘솔에서 `py "C:/Users/HEATH/Desktop/d_mix/STUDENT_HANDOFF_MQTT_2026-09-27/tools/rc_session_start.py"`. 종료는 같은 경로의 `rc_session_stop.py` 실행 후 Pi 터미널 Ctrl+C. Python 입력 모드에서는 `py` 대신 `exec(open("절대경로", encoding="utf-8").read())`를 사용한다.
- 연결상태 UI는 사용자 요청에 따라 Unreal에 추가하지 않는다. 대시보드에서 표시할 계획이다.


## 2026-10-20 전후진 즉시 시작 복원

- 사용자 요청으로 전후진 입력에서 가속 램프를 제거했다. 정지 상태에서 W/S 최대 입력 시 즉시 +/-0.2. 반전 시 0까지 감속 및 제자리 회전 0.12 상한/가속, 중립·오류 즉시 정지는 유지한다. 검사18개 통과, Pi 파일 반영 및 시험 수신기 600초 재시작 완료. 별도 회전 명령은 보내지 않았다.

## 2026-10-20 완만한 가속 실물 출력 시험

- 사용자 요청으로 고정 차체 모터 시험 수행: 전진 3초(최대0.2), 중립0.6초, 좌회전2초(최대0.12), 명령 중단0.7초. 텔레메트리 전진값 0→0.020→0.041→…→0.182→0.200, 좌회전 -0.12/+0.12 및 timeout 시 양쪽0 확인. 수신기/시험 프로세스 정상 종료0, 이전 상시 시험 수신기도 종료했다. 오프라인 검사18개 재통과.
- 로그는 GPIO에 전달한 제어값을 검증하며 실제 RPM/바퀴 기동/체감 부드러움은 사용자 관찰 확인 대기다. 자동화된 모터 없는 검사와 실물 모터 명령 시험을 구분한다.

## 2026-10-20 완만한 가속·저속 회전

- Pi 수신기에 ACCEL_RATE=0.1/s, DECEL_RATE=0.4/s를 적용했다. 직진 상한 0.2에 약 2초로 도달하며 제자리 회전 상한은 PIVOT_MAX_SPEED=0.12다. 반대 방향 입력은 우선 0으로 감속한 뒤 반전한다. 프레임 지연 dt는 0.1초로 제한한다.
- 키 해제와 안전 정지가 동일 0 명령이므로 0 입력은 즉시 정지하며 timeout/오류/연결 해제/종료도 즉시 0으로 초기화한다. 따라서 키 해제 후 완만한 감속은 적용하지 않는다. 무부하 실제 RPM 및 0.12 기동 여부는 미검증이다.
- 오프라인 검사 18개 통과, Pi /home/pi/rover_drive.py 배포 및 문법 검사 완료. 0.2 상한·600초 시험 수신기로 재시작했다. 별도 모터 명령 미발행. GitHub 업로드 전.

## 2026-10-20 수동 제자리 회전 MQTT 적용

- 수중 수동 모드에서 전후진 입력 없이도 ManualSteering을 MQTT st로 전달한다. A/왼쪽 음수 조향은 왼쪽 후진·오른쪽 전진, D/오른쪽은 반대다. 상하 입력 단독·자동복귀·강제 정지·핸드브레이크·입력 해제 대기는 계속 차단한다.
- Editor 빌드 및 Unreal Control 자동화 3개 통과, Pi 오프라인 검사 16개 통과. 최대값 0.2 유지. Pi 시험 수신기 600초 제한으로 갱신했고 에디터를 다시 열었다. 실제 사용자 좌우 회전 확인은 아직 대기. GitHub 업로드 전.

## 2026-10-20 최신 RC카 최대값 0.2

- 사용자 요청으로 로컬/Pi 수신 코드 MAX_SPEED와 실제 실행 인수 --max-speed를 모두 0.2로 변경했다. 오프라인 검사 15개 통과. Pi 수신기 600초 제한 재시작 및 max_speed=0.20 로그 확인. 별도 주행 명령은 보내지 않았다. 실제 회전속도 측정은 미수행. GitHub 업로드 전.

## 2026-10-20 속도/PWM 읽기 점검

- 현재 Pi 실행 인수 --max-speed 0.3, 코드 MAX_SPEED=0.3 및 실제 전진 로그 l/r=0.3 확인. 다른 차량 제어 Python 프로세스는 발견되지 않았다. gpiozero Motor는 기본 pwm=True, PWMOutputDevice 기본 100Hz이며 LGPIO는 value*100을 듀티로 전달한다. 추가 모터 구동 없이 소스/로그만 확인했다.
- 사용자 드라이버 모델 TB6612. 현재 motor_module은 각 모터에 두 GPIO만 설정하며 전용 PWMA/PWMB/STBY GPIO는 지정하지 않는다. 이 두 GPIO가 보드의 어떤 핀에 연결됐는지는 미확인이다. Toshiba truth table에 따라 IN을 PWM 제어하고 PWM핀을 High 고정한 경우 구동/고임피던스 정지가 교차하고, 전용 PWM핀 제어는 구동/short brake가 교차한다. 실제 배선 확인 전 변경 금지. 근거: https://toshiba.semicon-storage.com/info/datasheet_en_20141001.pdf?did=10660
- 정정: 00:00:24.087~26.410 전진 구간에는 timeout 로그가 없고 이후 중립 구간에서 반복됐다. 이것만으로 주행 중 통신 단절이나 속도 원인이라고 단정할 수 없다. RPM/실제 GPIO 파형은 미측정. 보드 PWMA/PWMB/STBY 및 IN 배선 사진/연결 정보가 필요하다.

## 최신 RC카 최대값: 0.3

- 사용자 재요청으로 로컬/Pi MAX_SPEED 및 실제 수신기 실행 인수를 모두 0.3으로 낮췄다. 기존 0.4 수신기를 정지한 뒤 600초 제한으로 재시작했고 max_speed=0.30 로그 확인. 오프라인 검사 15개 통과. 별도 회전 명령은 보내지 않았으며 Unreal의 일회성 송신 설정은 별개다.

## RC카 최대값 0.4 적용

- 사용자 요청으로 로컬 raspberry-pi/rover/rover_drive.py 및 Pi /home/pi/rover_drive.py의 22행 MAX_SPEED를 0.4로 변경했다. 기본값/허용 상한 모두 0.4. 오류 안내는 상수를 참조한다. 검사 15개 및 Pi 문법 검사 통과.
- 실제 프로세스 조회에서 기존 --max-speed 0.25를 확인한 뒤 중립을 검사하고 해당 수신기를 정상 종료했다. --max-speed 0.4로 600초 한정 재시작했으며 로그 max_speed=0.40과 초기 0 출력을 확인했다. 별도 회전 명령은 보내지 않았다. GitHub 업로드 전.

## RC카 최대값 0.3 변경

- 사용자 요청으로 raspberry-pi/rover/rover_drive.py의 MAX_SPEED를 0.6에서 0.3으로 변경했다. 기본값과 허용 상한 모두 0.3이며 이를 초과하는 설정은 거부한다. 검사 15개 통과. Pi /home/pi/rover_drive.py도 반영하고 문법 확인 완료. 모터 구동/재시작은 하지 않았다. 기존 시험 실행 인수 0.25는 유지되며 Unreal MPH 설정과 별개다. GitHub 업로드 전이다.

## 2026-10-19 실제 바퀴 배치 확정·수신기 반영

- 개별 모터 시험 사용자 확인: 1번 GPIO26/19=오른쪽 앞(양수 후진), 2번 GPIO22/27=왼쪽 앞(양수 전진), 3번 GPIO20/21=오른쪽 뒤(양수 후진), 4번 GPIO24/23=왼쪽 뒤(양수 전진). 2/4 양수 왼쪽 전진과 1/3 음수 오른쪽 전진의 25%·2초 시험 모두 정상이라고 사용자 확인했다.
- 수신기 MotorOutput 순서를 왼쪽2/4·오른쪽1/3으로 수정하고 오른쪽 반전을 기본으로 설정했다. --invert-right는 유지하며 다른 차체에서만 --no-invert-right로 해제 가능하다. 기본 실행은 여전히 GPIO 없는 dry-run. 실제 모터 이름별 좌/우/전진/후진/정지 회귀 포함 검사 15개 통과.
- Pi /home/pi/rover_drive.py 원본을 /home/pi/rover_drive.before-wheel-map-20261019.py로 백업한 뒤 수정본을 배포하고 py_compile 통과. 시험 수신기는 25% 상한, codex/ue-bench/20261019/cmd, 600초 자동 종료로 시작했다. 입력이 없으면 정지. Unreal 콘솔 세션 5분 제한은 별개이며 현재 재실행 필요. 배치 보정 후 Unreal 입력에 따른 실제 4륜 통합 검증은 사용자 확인 대기.
- GitHub에는 플러그인 수정본 31bb7ab만 업로드 완료. 수중 입력·Pi 수신기/검사·문서 변경은 아직 업로드 전이다.

## 2026-10-19 오른쪽 모터 방향 보정

- Unreal W 입력으로 바퀴가 회전했지만 사용자 관찰상 오른쪽은 후진 방향이었다. 시험 수신기를 정지했다.
- raspberry-pi/rover/rover_drive.py에 --invert-right 옵션을 추가했다. MotorOutput에서 오른쪽 두 모터 GPIO 부호만 반전하고 논리 th/st 및 l/r 상태값은 유지한다. 기본은 반전 없음이며 이 차체 시험에 옵션을 지정한다. 전후진·좌우 출력·0 정지 회귀를 포함한 오프라인 검사 14개 통과.
- Pi 임시 수신기에 수정본을 반영하고 --invert-right, 30%·2초 한정 시험 후 0 출력/timeout/정상 종료를 확인했다. 양쪽 실제 전진 방향 일치 여부는 사용자 확인 대기. 상시 수신기는 현재 종료 상태다. 변경 소스/검사는 아직 GitHub 업로드 전이다.

## 2026-10-19 23:33 재시험 수신기 만료 확인

- 사용자 Unreal 로그는 23:33:39 KST MQTT 연결 성공을 기록했다. Pi 시험 수신기는 19:17:27 KST에 600초 제한으로 정상 종료되어 당시 수신 프로세스가 없었다. 이번 미동작은 이전 소켓 오류와 다른 원인이다.
- PC에서 실제 시험 토픽을 구독만 하여 Unreal 중립 명령 6개를 확인한 뒤 Pi 수신기를 30% 상한·600초 제한으로 재시작했다. 사용자 W 입력과 실제 회전 확인 대기다. 일회성 시험 수신기는 10분이 지나면 재시작해야 하며 Unreal 콘솔만 실행해서는 Pi 프로그램이 재시작되지 않는다.

## 2026-10-19 Unreal MQTT 연결 실패 수정

- 사용자 콘솔 실행은 성공했지만 UE 로그에서 SE_EISCONN / Connection state: Connected 후 MQTT 연결 실패를 확인했다. 엔진 플러그인의 비동기 소켓 재접속 반복이 이미 연결된 Windows 소켓을 실패로 처리했다.
- 엔진 설치는 수정하지 않고 Plugins/MQTT에 UE 5.7 제공 플러그인 소스를 복사했다. OpenSocket에서 Connect 전후 SCS_Connected를 성공으로 인정하도록 수정했다. 프로젝트 플러그인 빌드 성공.
- 모터 수신기가 구독하지 않는 codex/verify/ue-connection 토픽으로 화면 없는 실제 PIE 검증: MQTT connected 확인, 별도 PC Paho 클라이언트가 Pi 브로커에서 {th:0,st:0} 168개 수신, 프로세스 정상 종료 0. 실제 사용자 키 입력과 바퀴 연결은 아직 확인 대기다.
- 임시 arm_rc_bench.py는 중복 실행 시 이전 콜백을 제거하고 세션별 클로저를 사용하도록 수정했다. 기본 설정 False 유지. 플러그인 소스의 비공개 GitHub 업로드 사용자 승인 완료.

## 2026-10-19 Unreal 수중 수동 MQTT 연결

- 사용자가 배터리를 켠 뒤 30%·2초 시험에서 실제 바퀴 회전을 확인했다. 고정 차체 시험만 승인됐으며 바닥 주행은 금지다.
- MqttVehiclePawn.cpp의 수중 수동 전후진을 th, 전후진 중 조향을 st로 변환했다. Chaos 기어 값에 의존하지 않는다. 제자리 조향만 입력할 때·상하 입력만 있을 때·자동복귀·입력 해제 대기·강제 정지·핸드브레이크·동시 전후진은 0이다. 기본 송수신 False 및 기존 포커스/일시정지 차단을 유지한다.
- Editor 빌드 성공. MqttVehicle.Control 자동화 3개 통과(ManualInput/FiveMphCruise/UnderwaterMovement). 기존 테스트 파일에 수중 전진/후진/조향/동시 페달/정지/상하 제외 검사를 반영했다. PC→Pi 별도 토픽 dry-run에서 명령 수신 및 입력 중단 후 timeout 출력 0을 확인했다. 이 시험은 GPIO 없이 수행했다.
- 임시 .upload-tools/arm_rc_bench.py는 메모리 기본 객체에서만 송신을 켜며 config/에셋을 저장하지 않는다. 한 번의 Play 종료 또는 5분 후 송신 기본 객체를 해제하고 시간 만료 시 Play 종료를 요청한다. 설정 Python 속성 4개는 별도 화면 없는 에디터에서 확인했다. 최종 사용자 Play 실행은 대기 중이다.
- Pi /tmp 임시 수신기를 codex/ue-bench/20261019/cmd, 출력 상한 0.30, 600초 timeout으로 시작했다. Unreal 실제 키 입력→실제 바퀴 연동은 아직 미검증이다. 시험 파일/로그는 업로드하지 않으며 기존 소스/테스트/문서 변경의 GitHub 업로드도 아직 하지 않았다.

## 2026-10-19 고정 RC카 MQTT 시험 재개

- 사용자가 Pi 192.168.0.8 및 바퀴가 바닥에 닿지 않는 차체 고정을 확인하고 바퀴 회전 시험까지 명시 승인했다. 바닥 주행은 승인 범위가 아니다. Unreal 기본 송수신 False는 유지한다.
- SSH 읽기 확인: 호스트 zerotoai, Mosquitto 1883/9001 수신, 기존 rover_drive/car_subscriber 프로세스 없음, Paho 2.1.0. 실제 motor_module.py의 네 모터 핀과 좌우 매핑을 확인했다.
- 기존 Pi 파일을 덮어쓰지 않고 로컬 수신기를 /tmp/codex_rover_bench.py로 복사했다. 시험별 고유 codex/bench/* 토픽으로 Pi 내부 MQTT dry-run 후 15% 출력 0.6초, 사용자 관찰을 위한 2초 재시험을 실행했다. 각 시험 후 중립 출력·timeout 상태·수신기 정상 종료(0)를 확인했다. 첫 시험은 사용자가 보지 못했고, 두 번째 15% 시험은 사용자가 실제 바퀴가 움직이지 않았다고 확인했다. 출력 텔레메트리는 센서 측정값이 아니다. 움직이는 중 통신 단절 시험은 아직 하지 않았다.
- 모터 배터리/전원 켜짐을 사용자 확인했다. GPIO 백엔드는 LGPIOFactory, Pi get_throttled=0x0. 기존 키보드 코드는 초기 출력 0.7을 사용한다. 기동 출력 부족 가능성을 확인하기 위해 30%·2초 시험을 수행했고 명령 출력 0.3→0 및 정상 종료를 확인했다. 30% 시험에서도 사용자가 바퀴가 움직이지 않았다고 확인했다. 추가 출력 상승은 중단했다. Pi 기존 /home/pi/rover_drive.py도 같은 ch06 모터 모듈과 좌우 매핑을 사용한다. MQTT 수신 성공은 실제 GPIO 전압·모터 회전 검증이 아니다. 드라이버 전원/활성화/실제 배선과 기존 수동 조작 성공 여부 확인이 필요하며 원인은 미확정이다.
- 이후 사용자가 배터리가 OFF였음을 확인하고 재시험을 요청했다(앞선 전원 ON 보고 정정). 30%·2초 재시험에서 출력 0.3→0, timeout 상태 및 수신기 정상 종료를 확인했다. 배터리를 켠 후 사용자가 실제 바퀴 회전을 확인했다.
- 이번 검사는 SSH로 시작한 Pi 내부 MQTT 발행→수신→GPIO 경로다. PC Unreal→Pi MQTT 경로 및 수중 수동 입력 변환은 아직 미완료이며, 수중 모드 GetVehicleControlInput은 여전히 0이다. 임시 시험 파일과 로그는 GitHub에 올리지 않는다.

## 2026-10-19 물고기 50마리 · 크기 1.5배

- 50마리(4무리: 13/13/12/12) 배치는 사용자 실행 로그의 2026-10-19 18:36:54 KST TAILOR_BUILD_OK로 저장 성공을 확인했다. 50마리 전체 Play 외형은 별도 미검증이다.
- 사용자 선택에 따라 제작 스크립트의 길이를 기존 45~60cm에서 67.5~90cm(1.5배)로 변경했다. 액터와 시퀀스의 모든 축척 키에 동일한 절대 크기를 적용하므로 반복 실행해도 계속 커지지 않는다. 개체 수와 유영 경로/속도는 유지한다.
- 크기 변경은 Python 문법 확인 완료, 열린 에디터에서 사용자 콘솔 실행 및 맵 저장 확인 대기다. 이미 대상 맵이 열려 있으면 재로드하지 않고 기존 편집과 함께 저장한다. Play 중 실행과 다른 미저장 맵 전환은 차단한다.
- 이번 50마리/크기 변경 GitHub 업로드는 미완료다. 실제 RC카/Pi 접근 없음.

## 2026-10-19 Tailor 표시 오류 수정 — 실제 Play 렌더 확인

- 초기 물고기 추가본은 액터/참조 Bounds만 검사해 FBX 애니메이션 축척 오류를 놓쳤다. 사용자가 보이지 않는다고 보고했고, 실제 Play 근접 렌더에서 재현했다. 원본 객체 변환 키와 단위를 정리한 파생 FBX로 수정했다. `Prepared/TailorNormalized.fbx`와 `MeshV2/`를 사용하며 원본은 그대로 보존한다. `unreal/tools/build_tailor_fish.py`의 Blender `--prepare-fbx` 모드로 재생성할 수 있다(Blender 4.5 검증).
- 메시/뼈대 객체 회전·축척을 적용하고 중복 객체 변환 애니메이션 키를 제거, cm 단위로 내보내 Swim_01 포즈를 유지했다. SkeletalMesh 재질 사용 플래그도 저장했다. 물고기 수/크기/경로와 MQTT 차단은 유지한다.
- 사용자가 수정 명령 실행 후 종료했다. 로그 2026-10-19 18:13:18 KST에 TAILOR_BUILD_OK 확인. 이후 저장 맵을 새 오프스크린 Play로 재로드해 MeshV2 24개체, 첫 개체 머리-꼬리 약 30.07cm(전체 몸길이 약 45cm), 검사 구간 약 310.68cm 이동을 확인했다. 시간차 렌더 2장을 직접 확인했고 물고기 표시와 위치 변화가 보였다. 진단 Play/프로세스 정상 종료. 전 개체 지형 관통·학교 PC VR 프레임·장시간 시험은 미검증이다.
- 초기 위치 안내 임시 스크립트는 존재하지 않는 editor_set_camera_info API로 실패했다. 선택 액터에 CAMERA ALIGN 명령을 쓰도록 수정했다. 사용자는 이제 콘솔 재실행 없이 저장 프로젝트를 열 수 있다. 임시 진단 파일/렌더는 .upload-tools에만 보관한다.
- 수정 파생 FBX, MeshV2 에셋, 스크립트, 맵/재질/관련 문서의 GitHub 업로드 사용자 승인 완료. 실차·Pi 접근 없음.

## 2026-10-19 Tailor 물고기 무리 추가

- 사용자 제공 CGTrader Wyreframe Tailor 원본(FBX 4개·PBR PNG 5개)을 `assets/selected_free_assets/Wyreframe/Tailor/`에 보존했다. `unreal/tools/build_tailor_fish.py`가 애니메이션 FBX와 BaseColor/Normal/Roughness/Metallic을 임포트하고 기존 맵에 24마리(4무리×6마리)를 추가한다. 원본 Height는 보존만 하며 적용하지 않는다.
- 길이 45~60cm, 4개 높이 구간(약 7.5~17.5m), 반경 3.8~4.9m·60초 순환. 원본 `Swim_01`을 반복하고 Level Sequence가 위치·방향을 갱신한다. 원본 앞방향은 -X. 충돌/그림자/고래 발견 태그 없이 관찰용으로 배치한다. 스크립트 재실행은 TailorFish_v1 소유 액터만 교체한다.
- 현재 LOD0 메시를 사용한다. 제공 LODGroup의 거리별 LOD 연결과 학교 노트북 VR 성능 측정은 미완료다. 경로는 고정 순환이며 장애물 회피·군집 AI가 아니다.
- 검증: Blender에서 뼈대와 Swim_01 확인. UE 5.7.4 화면 없는 에디터 제작/저장 성공, 별도 프로세스에서 24개체·24개 시퀀스 바인딩·각 축 121키·60초 반복·유영 설정·NoCollision 확인. 초기 commandlet Slate 오류 및 NullRHI 오류는 RenderOffscreen 실행으로 회피하고 Python 속성명을 수정했다. Play 실제 유영, 지형 관통/재질 외형/VR 프레임은 미검증이다. C++ 변경·빌드 없음. MQTT False 유지, Pi·모터 미접속.
- 사용자 승인: 물고기 원본/스크립트/생성 에셋과 현재 로컬 해저 맵 전체 업로드 승인. 이전에 업로드에서 제외했던 사용자 저장 맵 변경도 이번 맵에 포함된다. 변경 전 맵은 로컬 `.upload-tools/pre-tailor-map.umap`에 백업(미업로드). GitHub 최신 10월 19일 VR 인수인계 내용을 유지했다.


인수인계 갱신일: 2026-10-19 (Asia/Seoul). 기존 구현·실행 검증 기록은 2026-10-18 기준이다. 현재 사용자 요구와 실제 코드·저장 에셋을 기준으로 정리한다. README의 과거 작업 기록은 현재 검증 상태가 아니다.

## 2026-10-19 VR 전시 인계 반영

| 구분 | 현재 상태 |
| --- | --- |
| 최종 목표 | 학교 노트북 Unreal + Quest 3 Air Link PC VR + 웹 대시보드 + 고정 RC카 바퀴 연동. 전시 예정 2026-10-21. APK 제외 |
| 사용자 보고로 확인 | Quest 3 512GB, Link/Air Link 활성화·PC 검색, Meta 웹 캐스팅에서 앱 목록 표시 |
| 장비 미확인 | 학교 노트북 현재 사용 불가. PC 페어링·VR Preview·VR 성능·네트워크 안정성 미검증. RC카는 집에 있으며 현재 Pi 상태 미확인 |
| VR 소스 | 기존 프로젝트 재사용 예정. 이번 검색에서 전용 XR 연동을 찾지 못했고 .uproject에 OpenXR 명시 항목 없음. HMD/선체 회전 분리·Quest 입력·VR UI 적용 필요. 엔진 기본 활성 상태는 별도 확인 |
| 대시보드 | 기존 archive/exhibit_v1/dashboard.html은 구급차 UI·car/control 발행. 전시용 UI·VR 미러 창 공유·veh/* 상태 표시는 미구현 |
| MQTT 정적 확인 | 송수신 False. GetVehicleControlInput은 수중 모드/자동 제어/입력 해제 대기에서 0. 수평 수동 입력 변환 후 loopback dry-run 필요 |
| 이번 검증 | 문서·프로젝트 설정·출력 조건·대시보드·수신기 CLI 읽기만 수행. 추가 빌드·테스트·VR·브로커·Pi·모터 실행 없음 |

고정된 바퀴 회전도 승인 전에는 실행하지 않는다. Pi/모터 송신 금지와 기본 송수신 False를 유지하며, 독립 브로커·GPIO 없는 dry-run을 우선한다. 기존 파일 수정도 GitHub 게시 전에 확인을 받는다. 자세한 다음 작업·파일 위치·학교 복원/현장 절차는 README의 최신 인수인계를 따른다.

## 실행 제한과 기준 프로젝트

사용자 지정 작업 방식(2026-10-17): 단계마다 화면 없는 처리 가능 여부를 판단해 가능한 작업은 백그라운드에서 수행한다. 최신 지정에 따라 Unreal 콘솔 입력은 사용자가 직접 한다. AI가 스크립트와 `py "절대/경로/스크립트.py"` 형식의 완전한 붙여넣기 명령을 준비하고 목적·Play 중지·저장·에디터 종료 등의 영향을 알린다. 콘솔 입력을 위한 원격 화면 조작은 하지 않으며 실행 결과나 로그 확인 전에는 실행 완료로 기록하지 않는다. 그 밖에 화면 조작이 필요한 단계는 이유와 시작을 알린 뒤 진행한다. 직접 화면 검증을 생략한 항목은 미검증으로 남긴다. 이번 갱신은 Lanzaman 전용·보조 카메라 좌하단 512×288/작은 글씨 제거·우하단 H/M/Tab·우상단 만남 알림·이동 추진 효과 소스 변경을 반영한다. Editor 빌드·ManualInput 1개 통과 및 화면 없는 두 시점 UI 확인을 완료했다. 이전 곡선 자동복귀 정체 수정과 20/30/60Hz 회귀 검증 결과, 유리 8%·50cm 높인 1인칭·Q/E 수중 이동·아래 모니터 1개·좌석 2개·고래 33m는 유지한다.

사용자가 **실제 RC카 주행 금지**를 명시했다. Pi에 접속하거나 차량 프로그램·서비스를 실행하지 않고 모터 제어 명령을 보내지 않는다. MQTT 송수신 기본값은 모두 False이며 브릿지는 이 경우 연결을 시작하지 않는다. 실차 작업은 사용자의 명시적 재개 지시 전까지 보류한다.

기준은 `unreal/MqttVehicle/MqttVehicle.uproject`, 시작 맵은 `/Game/Underwater/Maps/L_UnderwaterExploration`, GameMode는 `BP_UnderwaterGameMode`, 기본 Pawn은 `BP_ExplorationRover`이다. 기존 프로젝트·Rescue 맵은 참고 사본으로 보존한다.

### 2026-10-18 출발점 3D A* 자동복귀

수중 탐사선의 기록 경로 역주행을 **현재 위치에서 출발점까지 새 경로를 계산하는 3D A***로 교체했다. 출발 앵커 근처의 실제 시작 위치만 보관하며 이동 이력은 필요 없다. 직선이 안전하면 곧바로 복귀하고, 막히면 상하를 포함한 26방향 격자 탐색 후 충돌 없는 지름길로 경유점을 줄인다. 모든 방향 회전을 포함하는 보수적 선체 박스로 장애물과 위험 구역을 검사한다. 위험 구역 안에서 시작하면 탈출은 허용하고 재진입은 피한다. 외부 라이브러리 없이 구현했다.

5초 준비 후 경로를 계산하고 다음 목표 방향으로 제자리 회전해 전진한다. 매번 180도 회전을 강제하지 않는다. 순항 5 MPH·수직 1m/s·출발점 25cm 이내 정지와 사용자 중단은 유지한다. 새로운 장애물 또는 8초 정체 시 최대 3회 재탐색하고, 경로가 없으면 정지한다. 일반 차량용 기존 기록 경로 코드는 호환용으로 남긴다.

탐색 간격 150cm, 최대 30,000개 노드·15초·출발점까지 200m 제한이다. 탐색 범위는 두 끝점에서 수평 40m·아래 15m·위 30m 여유를 두고 수중 상한으로 제한한다. 프레임당 최대 48개 작업과 약 2ms의 소프트 시간 예산으로 분할한다. 정적 장애물의 제한된 격자 안에서 최단 비용을 찾으며, 연속 공간 전체의 절대 최단 경로나 매우 좁은 통로 통과를 보장하지 않는다.

**직접 검증:** UE 5.7.4 Editor 빌드 성공, 화면 없는 Unreal 자동화 10개 성공·실패/경고 0. 기록 없는 30m 직선 복귀, 벽 우회, 동적 장애물 재탐색, 위험 구역 회피/탈출, 막힌 시작점 정지, 수동 취소와 알려진 최단 격자 비용을 검사했다. 실제 저장 해저 맵을 로드한 출발점 복귀도 성공했다. 중간 경유점 허용오차를 8cm로 맞춰 이동의 5cm 중립 구간과 충돌하지 않게 했고 정적 벽 사례의 불필요한 재탐색 0회를 확인했다. 이는 GamePreview 자동화이며 실제 H 키·포커스·전체 맵 장시간 통합 주행은 미검증이다. MQTT 송수신 False·출력 0, Pi/실차 미접속 유지. 새 UnderwaterPathPlanner.h/.cpp 업로드는 사용자 승인 완료.

## 구현 상태

### 2026-10-18 추진기 회전·기포와 만남 알림

Lanzaman의 고정 추진기 뒤에 3엽 날개를 양쪽에 추가했다. 원본 GLB/선체 메시를 분해하거나 변경하지 않는다. `Effects/SubmersiblePropulsionComponent`가 실제 Unreal 위치·회전 변화를 읽어 날개를 반대 방향으로 회전시키고, 이동 속도에 따라 양쪽에서 초당 합계 최대 72개의 기포를 만든다. 전후진·상하 이동·제자리 회전에 반응하고 정지 후 감쇠한다. 기포는 월드 공간에 남아 위로 떠오르며 1.6초 내 사라진다. 128개 인스턴스 풀을 재사용하고 충돌/경로 계획/차량 제어에는 관여하지 않는다. 유체/추력 물리 모델이 아닌 시각 효과다. 기존 Vefects 환경 기포는 유지한다.

고래 관찰 구역 진입 시 우상단에 “고래를 만났어요!”, 해파리 위험 구역 진입/빠른 통과 시 “해파리 주의!”를 각각 5초 표시한다. 두 알림은 동시에 표시할 수 있고 머무는 동안 반복하지 않으며 재진입하면 다시 표시한다. 고래·해파리 메시와 직접 접촉하거나 시선으로 인식하는 판정은 아니며 기존 구역 판정을 사용한다. H/M/Tab 단축키 가이드는 우하단 24 여백으로 이동했다. 좌하단 보조 카메라는 유지한다.

**구현/검증:** UE 5.7.4 Editor 빌드 성공, Unreal 자동화 12개 성공·실패/경고 0. 기포 반경 2.5~5cm 보강 후 추진 효과 자동화 1개도 재통과했다. 추진기 2개·이동 중 기포 생성·월드 공간 유지·정지 후 소멸·수직 이동 반응·MQTT 출력 0을 확인했다. 알림의 재진입·동시 표시·반복 억제·빠른 위험 구역 통과 이벤트를 검사했다. 화면 없는 Play에서 실제 고래 관찰 구역·해파리 위험 구역 진입 알림, 우상단 동시 표시·5초 후 숨김, 우하단 단축키를 확인했다. 후방·근접·옆 시점 렌더에서 원본 추진기 뒤 날개와 상승 기포 흐름, 정지 후 소멸을 직접 검토했다. 최종 렌더 검사 성공·종료 코드 0. 초기 임시 검사 스크립트의 Python 속성/호출 접근 오류는 실제 구역 이벤트 방식으로 수정했다. 엔진 초기화의 기존 Condition failed 로그는 원인 미확인으로 자동화 결과와 구분한다. 실제 OS 키보드/마우스 장시간 주행은 미검증이다. MQTT 송수신 False·Pi/실차 미접속. 새 효과 소스 2개·재질 생성 스크립트·기포 재질은 사용자 업로드 승인 완료. 에디터 종료 전 사용자가 저장한 로컬 해저 맵 변경은 보존하고 이번 업로드에서는 제외한다.

### 2026-10-18 도착 후 180도 회전·카메라 마우스 방향 공유

수중 A* 자동복귀는 출발점 25cm 이내에서 정지를 확인한 뒤 제자리에서 180도 회전하고 도착 상태로 전환한다. 도착 당시 방향을 기준으로 반 바퀴이며 최초 출발 방향으로 정렬하는 기능은 아니다. 초당 45도로 약 4초 동안 회전하고 전후진·상하 이동은 하지 않는다. 좌우 회전 공간과 회전 중 선체 충돌을 확인하며 공간 부족/시간 초과/사용자 중단 시 정지한다. 기존 5 MPH 복귀와 MQTT 차단은 유지한다.

탐사선의 1인칭과 3인칭은 같은 yaw/pitch를 실시간 공유한다. 마우스를 움직이면 메인·좌하단 보조 영상이 함께 방향을 바꾸고 Tab 전환 후에도 유지한다. R 정면 복귀도 양쪽에 적용한다. 탐사선 두 카메라의 회전 지연을 꺼 방향 반응을 일치시켰으며 일반 차량의 카메라 동작은 유지한다.

**직접 검증:** UE 5.7.4 Editor 빌드 성공. 화면 없는 Unreal 자동화 10개 성공·실패/경고 0. 기존 실제 맵/장애물/위험 구역 복귀 사례에서 도착 회전 중 이동 없음(0.1cm 허용), 최종 방향 180도(0.2도 허용), 도착 후 제어 해제를 확인했다. 기존 ManualInput 검사에 양쪽 마우스 방향·Tab 유지·R 동기화 검사를 추가해 통과했다. 엔진 초기화의 기존 Condition failed 로그는 자동화 성공 결과와 구분하며 원인 미확인이다. 실제 화면 렌더·OS 마우스 조작은 이번에 검증하지 않았다. MQTT 송수신 False·Pi/실차 미접속 유지. 새 보존 파일은 없고 기존 소스/테스트만 수정했다.

| 항목 | 저장된 구현 | 직접 확인과 한계 |
| --- | --- | --- |
| 탐사선 | Lanzaman 하나로 고정. 1/2 전환 키 제거·Iris 선택 API 거부. 원본 자료 보관 | ManualInput 1개 통과·두 시점 Lanzaman/카메라 UI 확인. 실제 1/2 물리 키 입력/재생성 통합 확인은 별도 |
| 차량 물리 | 선택 탐사선은 E 상승/Q 하강·중립 깊이 유지·W/S 수중 추진·A/D 45도/s 회전. 전체 선체 Bounds 박스로 Sweep | 수직 최고 100cm/s·시작 높이+30m 상한. 바닥/벽/상한·키 해제/정지 자동화 통과. 유체 부력 아닌 로컬 이동, 일반 차량만 기존 Chaos |
| 속도 | 수동 전진 6 MPH·자동복귀 5 MPH(8.04672km/h)·수동 후진 3km/h | 수중 자동화: 수동 6 MPH·자동 5.000 MPH 확인, 수직 1m/s 제한·도착 감속. 기존 Chaos 시험 최고 5.013 MPH는 일반 차량 회귀 결과 |
| 관찰 카메라 | Lanzaman 좌석 2개·아래 모니터 1개·창틀/조종 장치(위 모니터와 지지대 제거), 실내 조명·원본 모니터 그래픽(장식). 1인칭 시작·Tab 전환·좌하단 512×288 무자막 반대 영상 | Lanzaman 눈 위치 (-90,-62,320)cm·FOV90(이전보다 50cm 상승), 유리 불투명도 0.08·옅은 청록색. 높인 위치는 유지. Iris는 플레이 선택에서 제외. 기본/선택 변경/Tab 액션 자동화 통과. 실제 키 화면 입력은 이전 미검증 유지 |
| 고래 | 원본 혹등고래 뼈대 메시 약 33m(직전 16.5m의 2배), 9.125초 반복 애니메이션, 반경 8m·주기 55초 시계 방향 원형 이동 | 확대 후 저장 맵 재로드로 길이 3300cm·균일 축척·반경/주기/애니메이션 연결 확인. 머리 방향 전진 자동화/Play 결과는 확대 전 기록이며 확대 후 Play는 추가 실행하지 않음 |
| 고래 발견 | 관찰 패드 6m 진입·7.5m 퇴장, 수중 높이 33m(일반 차량 3m), 세션 최초 발견 | 경계·재진입·대상 유실 단위 테스트 통과. 시선/영상 인식이 아닌 위치 판정 |
| 환경 | 해초 871개·바위 177개(HISM), Vefects 기포 24곳, 기존 해저 장면·해파리 3마리 | 맵 374개 액터 재개방·4시점 렌더 확인. 기포 재생·최종 프레임 성능은 Play 확인 전 |
| 자동복귀 | 출발점 3D A*·선체/위험 구역 충돌 검사·경로 단축·최대 3회 재탐색 | 자동화 10개 통과. 기록 없는 직선/벽 우회/동적 장애물/실제 저장 맵 복귀 확인. 전체 맵 장시간 물리 키 주행 미검증 |
| 중단·도착 | X/Space/M/새 입력·포커스 정지 유지. 수중 8초 정체 재탐색, 실패 시 정지. 출발점 25cm 도착 후 제자리 180도 회전 | 계획 취소·막힌 시작점 정지·도착 제어 해제 자동화 확인. 포커스/통신 통합 검증은 별도 |
| 위험 구역 | (-600,1800,300)cm, 14×10×6m 고정 TriggerBox, 1m 퇴장 여유, 선분 통과와 반복 방지 | 단위 테스트 통과. 해파리는 시각 장식이며 센서/추적 생물 아님 |
| MQTT | veh/cmd {th,st}, 활성화 시 100ms·QoS0·retain false. 수중 모드/자동복귀/제자리 회전 출력 조회0 | 송수신 기본 False·브로커 접속 생략 유지. Pi 접속/모터 송신 없음. Unreal↔브로커 통합 시험 미실행 |
| 응급 경보 | 브릿지의 경보 구독/파싱/발행 제거, 옛 Blueprint 심벌은 호환용 | 현재 해저 실행 경로에서 옛 Rescue Timeline 사용하지 않음 |
| Pi 수신기 | `raspberry-pi/rover/rover_drive.py`: 입력 검증·0.3초 워치독·재접속/종료 정지·기본 dry-run | 오프라인 단위 테스트 13개 통과. 실제 Pi 미배포·미실행, 실차 결과 없음 |

## 검증 기록

- Lanzaman 전용/좌하단 확대: Editor 빌드 성공. 기존 ManualInput 자동화 1개 성공·실패/경고 0: Iris 선택 거부·Lanzaman 유지와 기존 카메라/입력 회귀를 확인했다. 화면 없는 Play의 내부/외부 메인 시점에서 Lanzaman, 좌하단 512×288/24 여백, 보조 영상의 텍스트 없음, H/M/Tab 세 줄, 속도/기어 숨김을 확인하고 렌더 2개를 직접 검토했다. 검사/종료 코드 0. 렌더 초기화의 기존 Condition failed 4줄은 별도 기록하며 전체 주행/실차 검증으로 해석하지 않는다. 안내는 H/M/Tab만 표시하고 속도/기어는 숨긴다. 실차/Pi 미접속·MQTT False.

- UI 간소화: UE 5.7.4 Development Editor 빌드 성공. 화면을 띄우지 않는 Play에서 내부/외부 메인 시점 각각 우하단 400×225·24 여백·속도/기어 Collapsed·단축키 텍스트와 반대 시점 영상을 확인했고 UI 포함 렌더 2개를 직접 확인했습니다. 검사와 프로세스 종료 코드 0. 기존 엔진 초기화 Condition failed 4줄은 남아 있어 전체 자동화 통과로 표시하지 않습니다. 주행·물리 키 입력·실차 시험은 하지 않았습니다. 우하단 영상 640×360 캡처·최대 20Hz 및 관찰·복귀·정지 동작은 유지한다. MQTT False·Pi/실차 미접속. 임시 검사 스크립트의 Python API 이름/비공개 WidgetTree 접근 문제를 고친 뒤 최종 확인에 성공했다.

- 곡선 복귀: 장애물 없는 곡선·상하 곡선·S자 3개가 기존 코드에서 정체 정지하는 것을 재현했다. 수중 목표점 유지·기록점 편차 20cm 이내의 앞쪽 목표 선택·선체 Sweep을 적용했다. 최신 UE 5.7.4 Editor 빌드 및 자동화 8개 성공(실패/경고 0). 5개 경로×20/30/60Hz 15개 복귀 모두 출발점 25cm 이내, 최고 5.000 MPH. 추가 장애물 사례에서 벽 앞 정지/자동 제어 해제 및 장애물 제거 후 기록 경로 재시도 성공. 실제 Blueprint·수동 이동·복귀 코드를 GamePreview에서 실행했다. 사용자 당시 전체 54개 경로점의 재생이나 전체 해저 맵 통합 검증은 아니다. 실제 RC카/Pi 접근 없음.

- 유리 8% 조정: 열린 에디터 저장, 별도 창 없는 Play에서 불투명도 0.08과 유지된 카메라 Z=320cm 확인, 전방/좌석/외부 렌더 3개 확인. 옅은 청록색과 전방 배경 시야를 확인했다. 렌더 완료/에디터 종료 로그는 있으나 도구 종료 코드 1 및 기존 초기화 Condition failed 4줄은 원인 미확인이다. C++ 빌드·자동화·주행 시험은 추가하지 않았다. MQTT 송수신 False 유지. 아래 0% 완전 투명 설정은 이전 기록이다.

- 조종석 유리/높이: `M_Lanza_Glass` 불투명도 0·비조명으로 유리 색/반사/굴절을 제거하고 Lanzaman 카메라를 50cm 높인 (-90,-62,320)cm로 저장했다. UE 5.7.4 Editor 빌드 및 ManualInput 자동화 1개 성공(실패/경고 0). 저장 재질/Blueprint와 런타임 카메라 위치를 새 프로세스에서 확인하고 창 없는 Play 전방/좌석/외부 영상 3개를 렌더했다. 중앙 배경 시야와 아래 모니터 유지 확인, 종료 코드 0. 기존 엔진 초기화 Condition failed 4줄은 남음. 이번 변경은 주행/실제 키 입력 검증을 추가하지 않았으며 MQTT 송수신 False를 유지한다.

- Q/E 수중 이동: 빌드 성공, 자동화 7개 통과(실패/경고 0). 실제 Q/E 키 이벤트부터 매핑/이동까지 보강한 수중 테스트 재통과. 3초 상승 273.1cm, 중립 호버링·하강·동시 키·Space/정지 후 재입력·6 MPH 수동/5.000 MPH 자동·벽/바닥/30m 상한을 확인했다. 초기 테스트 입력 객체 수명 문제는 보강했다.
- 저장 해저 맵의 화면 Play: 입력 API로 373.50cm 상승 → 호버링 2초 높이 변화 0cm → 124.01cm 하강 → 879.44cm 공중 전진 → 높이 포함 자동복귀(18.5초), 출발점 24.75cm 이내 정지. 화면 없는 Play에서는 포커스 정지로 이동하지 않았고 화면 포커스를 준 실행에서 통과했다. OS 물리 키보드 장시간 주행과 구분한다. MQTT False/출력0 유지, 최신 에디터 재개방 완료.

- 이번 창 없는 카메라 렌더는 이미지 3개 출력·정상 종료 코드 0을 확인했다. 엔진 초기화 전 기존 `LogAutomationTest: Error: Condition failed` 4줄은 다시 출력되었으며 원인은 미확인이다. 이번에는 자동화 테스트를 실행하지 않았고 렌더 확인을 전체 테스트 성공으로 표시하지 않는다.

- 위 모니터 제거 후 기존 FBX/uasset 저장. 원본 GLB 해시 불변, 다른 25개 오브젝트와 전체 Bounds·재질 2개 유지. UE LOD0 정점 90,129→89,269. 별도 창 없는 Play로 저장 메시를 다시 읽어 전방의 위 화면 제거·아래 화면 유지, 좌석 2개·외부 반대 영상을 확인했다. 재임포트 시 FBX 스무딩 그룹 누락 경고가 있었으며 C++ 빌드·주행 시험은 추가하지 않았다. MQTT 송수신 False 유지. 아래 두 모니터 렌더는 이전 기록이다.

- 고래 2배 확대 후 열린 에디터 맵 저장, 별도 화면 없는 프로세스에서 저장 맵 재로드 성공. 길이 3300cm·축척 각 축 119.2359528·직전 길이 대비 2배, 반경 800cm·주기 55초·유영 애니메이션 9.125초·NoCollision·374개 액터 확인. MQTT 송수신 False 유지. C++ 빌드·Play 주행·실차는 이번 크기 변경에서 실행하지 않았다.

- 사진 기준 내부 변경 후 UE 5.7.4 Editor 빌드와 자동화 6개 성공, 실패/경고 0. `FiveMphCruise`는 MQTT 없는 GamePreview 평지에서 실제 저장 탐사차·경로 추종/입력 코드를 사용했다. 10초 동안 20.94m 이동, 최고 5.013 MPH·마지막 5초 평균 4.926 MPH, 자동 입력의 MQTT 출력 0을 확인했다. 최초 시험의 월드 BeginPlay 누락을 고친 뒤 통과했다. 이는 평지 직선 물리 시험이며 해저 맵 전체 경로 또는 실차 결과가 아니다.
- 기존 입력/경로 테스트에 Lanzaman 원본 내부 표시·왼쪽 좌석 카메라·Iris 선택/재선택·수동 6/자동 5 MPH 분리·조향 중 가속/도착 제동 회귀 검사를 보강했다.

- 최종 창 없는 Play 렌더에서 전방 위아래 모니터·조종 장치, 옆을 돌아본 두 좌석, 외부 반대 시점을 확인했다. 처음 두 렌더는 제한된 실행 환경의 Chromium IPC 권한 오류와 함께 초기 종료했고 정상 권한으로 재실행해 출력·종료 코드 0을 확인했다. 엔진 초기화 전 기존 Condition failed 4줄은 별도 자동화 성공 결과와 구분한다.

이전 조종석/회전 단계 기록:

- 최신 조종석/회전 변경 후 UE 5.7.4 Development Editor 빌드 성공. 기존 Unreal 자동화 5개에 실제 저장 Tab 액션 + C++/Blueprint 바인딩, 두 카메라 숨김 목록, 기본 Lanzaman, 공유 6 MPH, 제자리 회전/정지 회귀 검사를 보강했고 모두 통과했다.
- 짧은 화면 Play에서 수동 입력 API로 약 10.9m 이동 후 복귀 API 요청 → 제자리 180도 회전 → 전진 추종 → 출발점 약 98cm 이내 정지 확인. 복귀 최고 약 6.00 MPH, 회전 표본의 XY 위치 변화 0.01cm 미만, 도착 속도0·핸드브레이크와 MQTT용 출력0. 이 표본을 장시간 주행/실차 성공으로 해석하지 않는다.
- 조종석/외부 메인과 좌하단 반대 영상 표시를 직접 화면에서 확인했다. 최종 계기판·조종간·문구 위치 수정 후 창 없는 Play로 SceneCapture 내부·외부 영상을 내보내 확인했다. 임시 검증 실행은 종료했다.
- 창 없는 렌더 실행은 종료 코드0과 내부/외부 파일 출력을 확인했다. 다만 엔진 초기화 전 `LogAutomationTest: Error: Condition failed` 4줄이 있어 원인은 미확인이다. 별도로 실행한 MqttVehicle 자동화 5개의 실패/경고0 결과와 구분한다.
- 실제 Tab 키 화면 검증은 다른 창 입력/포커스 간섭으로 최종 재확인하지 못했다. 저장 매핑 및 액션의 누름/유지/재누름 검사는 성공했으며 OS 입력/포커스 결과와 구분한다.

이전 단계 기록:

- UE 5.7.4 / Development Editor Win64: UHT 포함 최신 C++ 빌드 성공.
- Unreal 자동화: `MqttVehicle.Control.ManualInput`, `MqttVehicle.Exploration.DangerZone`, `MqttVehicle.Exploration.RoutePolicy`, `MqttVehicle.Observation.ZoneAndDiscovery`, `MqttVehicle.Observation.SwimmingAsset` 5개 성공, 실패·경고 0개.
- 자동화는 순수 계산/상태 검사 및 GamePreview 월드를 사용하며 실제 키보드 운전·장시간 Chaos 주행 시험이 아니다. MQTT 브릿지는 GamePreview에서 생성하지 않는다.
- Python 수신기 오프라인 테스트 13개 통과. GPIO·Paho·브로커를 쓰지 않는다.
- 저장 맵 재개방 시 고래 대상/관찰점/출발점 태그 각 1개, HISM 배치 개수와 원본 유영 9.125초·재질 연결 확인. 탐사선 두 외형·고래·전체 환경 렌더 확인. 10월 17일 별도 맵 검사에서는 오류 0개, 기존 해파리 구성의 BoundsScale 확대/동적 그림자 성능 경고 27개가 나왔다. 고래/해초/바위 재질 사용 플래그 저장·재로드는 확인했다.
- 화면 없는 첫 자동화 시도는 복원된 Fab 탭의 RHI 의존으로 실패했다. 시험 명령에서만 `-DisablePlugins=Fab`와 빈 맵을 사용해 5개 모두 통과했다. 프로젝트 Fab 설정은 변경하지 않았다.
- 10월 17일 Play에서 6 MPH 설정에 대해 최고 약 5.98 MPH를 측정했고 단위 문구를 MPH로 맞췄다. H 요청에 의한 유턴 출발 및 고래 앞 방향 유영을 확인했다. 안내등 받침대 충돌을 수정해 영구 저장한 맵에서 수동 입력 API로 약 5.2m 이동→실제 H 키→유턴→기록 경로 전진 복귀→도착 정지를 확인했다. 약 56초(준비 포함), 출발점 약 98cm 이내, 도착 속도 0, 핸드브레이크 적용. 복귀 중 기어 1과 송신용 입력 0을 유지했고 해당 구간 최고 약 4.90km/h를 측정했다.
- 로그/렌더/자동화 보고서는 이 PC의 `.upload-tools/`에만 보관하며 업로드하지 않는다. 실차 제어·Pi 접속·배포는 하지 않았다.

## 코드·에셋 위치

- `unreal/MqttVehicle/Source/MqttVehicle/`: Pawn의 입력·외형·속도·카메라, Controller의 선택/관찰/복귀 연결, BridgeSubsystem의 MQTT 연결 관리.
- 같은 모듈 `Camera/`: 런타임 조종석·반대 시점 SceneCapture·좌하단 512×288 무자막 위젯. `Selected/Materials/M_CameraFeed`가 영상 표시 재질이다.
- 같은 모듈 `Observation/`: 관찰 판정·단축키 전용 UI와 `WhaleSwimmingActor.h/.cpp`의 원형 유영.
- 같은 모듈 `Exploration/`: 수중 3D A* 경로 계산·충돌/위험 회피·재탐색·도착 회전, 일반 차량용 기록 경로 호환 코드. `Tests/`의 최신 기록은 Unreal 자동화 12개 성공이다.
- `Content/Underwater/Selected/`: 선택 탐사선·해초·바위·혹등고래와 변환 재질/텍스처. `Content/Vefects/Bubbles_VFX/`: Fab 원본 Niagara 패키지.
- `assets/selected_free_assets/`: 원본 GLB·FBX·텍스처·원본 압축 파일, `Converted/`에는 Blender 정규화본과 재질 매핑.
- `unreal/tools/prepare_selected_assets.py`, `build_selected_underwater.py`: 변환·임포트·배치. `animate_underwater_scene.py`: 이전 고래 절차 재질·현재 해파리 시각 요소 제작.
- 출처·라이선스는 `docs/ASSET_CREDITS.md`, 파일별 역할·크기·해시는 `docs/FILE_GUIDE.md`, `docs/FILE_MANIFEST.json`.

## 수중 이동 범위

E 상승·Q 하강은 구현/검증 완료다. 탐사선은 기존 바퀴 접지 대신 충돌 검사 기반 이동으로 공중 전후진·정지 호버링을 제공한다. 최대 상승 높이는 시작점+30m, 수직 속도는 1m/s다. 물리 부력·수류·선체별 정밀 충돌·롤/피치 조종은 포함하지 않는다. 자동복귀의 거리/높이/도착은 3D이며 관찰 높이도 33m로 확장했다. 위험 박스는 기존 3D 범위를 유지한다. 수중 모드 MQTT 출력은 0이다. 실차 재개 시 수평 입력 연결을 별도 확인해야 한다.

## 남은 순서

1. 학교 노트북 복원·기존 해저 탐사 비VR 실행을 확인하고 Pawn/카메라/입력의 VR 변경 지점을 조사한다.
2. Quest 3 Air Link PC VR을 위해 HMD·컨트롤러·UI를 구현한다. 기존 키보드·Lanzaman·수중 이동·3D A*·정지 회귀와 실제 VR 성능은 따로 확인한다.
3. 레거시 대시보드를 VR 미러 창 공유·MQTT 상태·명령 표시용으로 개편한다. 웹 캐스팅 성공을 대시보드 통합 성공으로 표시하지 않는다.
4. 수중 수평 수동 입력 변환 후 127.0.0.1 브로커·가짜 모터로 연속 송신·단절·재접속·종료·자동 제어 차단을 검증한다. Pi 주소로 시험하지 않는다.
5. 설치/전시 시작·종료 가이드를 마무리하고 사용자 승인 후에만 고정 RC카 바퀴 시연을 별도 검증한다. 실제 바닥 주행·위치 복귀·음성 구현 완료로 표시하지 않는다.

