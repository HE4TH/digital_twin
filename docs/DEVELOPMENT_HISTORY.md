# D-Mix 개발·배포 이력

> 날짜별 설정 변경, 배포, 점검 결과를 보관하는 기록입니다. 프로젝트의 개요와 구조는 [README.md](../README.md)에서 확인합니다.

## 2026-11-07 문서 구조 개편

- 저장소 첫 화면의 README를 프로젝트 개요, 시스템 구성, 주요 기능, 폴더 구조, 실행 순서 중심의 안내 문서로 재구성했다.
- 이전 README에 누적되어 있던 Pi 주소 변경, Quest 설치·배포, 점검 결과는 이 문서에서 계속 보관한다. 실제 구현 여부와 직접 검증 결과는 각각 [CURRENT_STATE.md](CURRENT_STATE.md)와 docs/AI_HANDOFF.md를 함께 확인한다.
- README 제목에서 D-Mix 명칭과 현재 연결 설정·현재 확인 항목을 제거했다. 연결 주소와 미검증 항목은 이력·상태·인계 문서에서 관리한다.

## 2026-10-31 현재 Pi 주소: 192.168.0.6

- 사용자가 안내한 192.168.0.205는 다른 장치였고, Pi에서 직접 확인한 실제 주소는 192.168.0.6이다. 프로젝트의 대시보드 URL, MQTT BrokerHost, Pixel Streaming ConnectionURL, 대시보드 실행 스크립트와 Unreal 보조 도구의 주소를 192.168.0.6으로 변경했다. 대시보드 주소는 http://192.168.0.6:5050 이다.
- Quest 앱 설정을 기기 안에 백업한 뒤 URL과 BrokerHost를 변경하고 재시작했다. Quest에서 Pi ping 2/2 성공, Pi 대시보드 API에서 quest.connected=true, control.broker=true, send_enabled=false를 확인했다. 영상 수신은 아직 확인하지 않았다.
- Pi의 SSH 공개키로 장치를 확인한 뒤 dmix-dashboard와 dmix-pixel 서비스 설정을 .runtime에 백업하고 공개 주소를 변경·재시작했다. 두 서비스는 active이며 대시보드 broker=true를 확인했다. RC 송신과 수신기 시작은 수행하지 않았다. GitHub 게시 전 로컬 변경이다.

## 2026-10-23 현재 Pi 주소: 192.168.1.13

- 사용자가 최초 안내한 192.168.0.13을 192.168.1.13으로 정정했다. 저장소의 Android 대시보드 URL, MQTT BrokerHost, Pixel Streaming ConnectionURL, raspberry-pi/dashboard/start.ps1 및 tools의 브릿지/RC 연결 주소를 최종 주소로 수정했다. 대시보드 http://192.168.1.13:5050, MQTT 1883, Pixel Streaming 8888/8889. 과거 날짜 IP는 당시 기록이다.
- Pi 장치 키와 실제 IP 확인 후 dmix-dashboard/dmix-pixel 서비스 파일을 .runtime에 백업하고 공개 주소 변경·두 웹 서비스 재시작 완료. 둘 다 active 및 새 주소 API 응답을 확인했다.
- Quest Game.ini를 Git 제외 임시 폴더에 백업하고 URL/BrokerHost를 변경·앱 재시작했다. Quest→Pi ping 2/2 성공, quest.connected=true, control.broker=true, send_enabled=false 확인. 조회 당시 video.live=false, receiver=null이며 영상·모터 동작 검증은 하지 않았다.
- APK 재빌드·GitHub 게시·RC 수신기 시작·송신 켜기는 수행하지 않았다.

## 2026-10-22 현재 Pi 주소: 192.168.1.10

- 사용자 요청으로 활성 연결 주소를 192.168.1.10으로 변경했다. 대시보드 http://192.168.1.10:5050, MQTT 192.168.1.10:1883, Pixel Streaming 신호 주소 ws://192.168.1.10:8888 및 웹 연결 8889를 사용한다. 과거 날짜의 IP는 당시 작업 기록이다.
- DefaultGame.ini, AndroidGame.ini, raspberry-pi/dashboard/start.ps1, unreal/tools/connect_local_dashboard.py, unreal/tools/rc_session_start.py를 수정했다. 테스트의 구 IP는 격리된 Host 검사 입력이므로 유지했다. 바깥 인계 사본은 변경하지 않았다.
- Pi 장치 키와 실제 IP 확인 후 dmix-dashboard/dmix-pixel 서비스 설정을 백업·수정하고 두 웹 서비스를 재시작했다. 둘 다 active, 대시보드 HTTP 200 및 broker=true 확인. RC 서비스를 시작하거나 송신을 켜지 않았다.
- 사용자 USB 디버깅 승인 후 Quest Game.ini를 Git 제외 .upload-tools 폴더에 백업하고 기존 설정의 URL/BrokerHost를 192.168.1.10으로 변경했다. 앱 재시작 및 Pi ping 2/2 성공, 서버 quest.connected=true, control.broker=true, send_enabled=false 확인. 초기 조회에서 video.live=false였으므로 영상 재생 검증과 구분한다. RC 수신 상태는 null이며 수신기 시작·송신 켜기는 수행하지 않았다. APK 재빌드 및 GitHub 게시는 아직 하지 않았다.


## 2026-10-21 Pi IP 변경에 따른 대시보드 접속 오류 수정

- 사용자 화면에서 configured host required 오류를 보고했다. 기존 Pi SSH 호스트 키로 장치를 확인했고 실제 IP는 192.168.1.9, dmix-dashboard의 --public-host는 이전 10.95.86.138이었다. 서비스 파일을 .runtime/dmix-dashboard-before-ip-change.service에 백업하고 --public-host를 192.168.1.9로 변경한 뒤 웹 서비스만 재시작했다.
- 새 대시보드 주소: http://192.168.1.9:5050 . 후속 사용자 요청으로 저장소 AndroidGame.ini의 URL을 "http://192.168.1.9:5050", DefaultGame.ini의 BrokerHost를 192.168.1.9로 수정했다. 사용자가 USB 디버깅을 허용한 후 현재 Quest의 Saved/Config/Android/Game.ini를 Git 제외 임시 폴더에 백업하고 URL과 BrokerHost를 변경했다. 앱 재시작 및 Quest→Pi ping 2/2 성공을 확인했다. Pi /api/state에서 quest.connected=true, control.broker=true, send_enabled=false와 video.live=true, video.fps=18, codec=avc1.42801f를 확인했다. 영상 fps는 서버 보고값이며 화면의 시각적 품질을 직접 검증한 것은 아니다. receiver 출력 l/r=0이며 송신 게이트를 켜지 않았다. 현재 Pi/앱은 video 상태 필드를 제공하는 버전이므로 과거 JPEG 전용 상태 기록과 구분한다. APK 재빌드는 하지 않았다. 과거 주소 기록은 당시 네트워크 기준이다. 로컬 문서 기록이며 GitHub 게시 전이다.


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


[다운로드 페이지](https://github.com/HE4TH/digital_twin/releases/tag/quest3-2026.10.21-preview)에서 **DMix-Quest3-Windows-2026.10.21.zip**을 받으세요. 저장소가 비공개이면 접근 권한이 있는 GitHub 계정으로 로그인해야 합니다. Source code ZIP에는 APK가 없습니다.

## 다른 노트북에서 설치

1. 위 ZIP 전체를 원하는 폴더에 압축 해제합니다. Unreal Engine, Android Studio, 고성능 GPU는 설치에 필요하지 않습니다.
2. Quest의 기기 소유자/관리자 계정에서 개발자 모드를 켜고 USB 데이터 케이블로 연결합니다.
3. 헤드셋 안에서 **USB 디버깅 허용**을 승인합니다. ‘파일 접근 허용’은 다른 권한입니다. 다른 노트북에서는 다시 승인해야 합니다.
4. 압축을 푼 폴더의 **Install-Quest3.cmd**를 실행합니다. 최초 실행 시 ADB가 없으면 Google의 SDK 약관 링크가 나오며, 직접 확인·동의한 경우 YES를 입력하면 공식 Platform Tools를 내려받습니다. 이미 받은 platform-tools 폴더를 설치 파일 옆에 놓아도 됩니다.
5. `Success`와 `Installation complete`가 나오면 Quest의 앱 목록 → 알 수 없는 출처에서 **D-Mix Underwater**를 실행합니다.

PowerShell에서는 압축을 푼 폴더에서 다음과 같이 실행할 수 있습니다.

```powershell
.\Install-Quest3.cmd
```

여러 기기를 연결했다면:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Install-Quest3.ps1 -Serial "기기번호"
```

기존 앱을 먼저 삭제하지 마세요. `adb install -r`로 업데이트하며 앱을 자동 실행하지 않습니다. APK와 release.json의 SHA-256을 대조한 후 설치합니다. 서명 불일치 오류가 나면 자동 삭제하지 않고 중단합니다.

## 전시 연결

- VR 렌더링은 Quest 자체에서 수행하며 Air Link가 필요 없습니다.
- RC/대시보드는 Quest와 Pi가 서로 통신 가능한 Wi-Fi에 있어야 합니다. 현재 Pi 주소는 10.95.86.138, 대시보드는 http://10.95.86.138:5050 입니다. 주소가 바뀌면 앱과 Pi 설정도 변경해야 합니다.
- 대시보드 버튼: RC 수신 시작 / RC 명령 송신 켜기·끄기 / 자동복귀 시작.
- 앱 시작 시 RC 송신은 꺼져 있습니다. 수신기 준비 후 송신을 켜세요. 서버와 3초 통신 단절 시 송신 게이트가 닫히며 자동으로 다시 켜지지 않습니다.
- 자동복귀 시작·도착·중단 알림과 복귀 중 상태 표시를 VR 시야에 표시하도록 구현했습니다. 실제 RC카의 위치를 측정해 출발점을 찾아가는 기능은 아닙니다.

## 현재 검증 범위

개발용 서명 APK입니다. Android 빌드 및 패키징 검증과 실제 Quest 화면·프레임률·모터 검증은 다릅니다. 이번 알림과 대시보드 제어의 실기 확인은 아직 하지 않았습니다. Pi 버튼 기능 배포는 Pi 연결 후 별도로 진행할 예정이므로 APK 설치만으로 Pi 서비스가 바뀌지는 않습니다.

## 연결 오류

- `unauthorized`: 헤드셋의 기기 소유자 계정에서 USB 디버깅 팝업을 승인한 후 다시 실행합니다.
- 기기 목록이 비어 있음: 데이터 지원 케이블·USB 포트·개발자 모드를 확인합니다.
- APK를 Quest 저장소 폴더에 복사하는 것만으로 설치되지 않습니다.
- 설치 후 USB를 뽑아도 앱은 실행되지만 RC/대시보드에는 Wi-Fi가 필요합니다.

ADB 공식 배포/약관: https://developer.android.com/tools/releases/platform-tools


---

# 해저 탐사 VR 전시 · Unreal / Quest 3 / MQTT / RC카

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


## 2026-10-20 GitHub 게시 요청

- 사용자가 현재 변경·추가 파일의 GitHub 게시를 승인했다. 프로젝트 소스·설정·에셋·문서를 대상으로 하며 APK, 자동 생성 Android Build 파일, 로그, 캐시, raspberry-pi/dashboard/.runtime 인증정보와 SSH 키는 제외한다. 자동 실행 환경의 Git LFS 쓰기 및 Windows 자격증명 접근 제한으로 일반 PowerShell 게시 스크립트를 준비했다. 실제 게시 완료 여부는 원격 main 커밋 확인 결과로 판단한다.


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

## 2026-10-20 노트북 UE 5.7.4 로그 카테고리 중복 빌드 오류 수정

- 사용자 제공 17:36:56(KST) 빌드 로그는 UE 5.7.4 / MSVC 14.44 / SDK 10.0.22621 실행이다. `MqttVehicle.h`의 공통 `LogMqttVehicle` 선언과 `MqttVehicleBridgeSubsystem.cpp`의 같은 이름 정적 정의가 Unity 빌드에서 충돌해 C2011 및 연쇄 로그 형식 오류가 발생했다.
- 브릿지에서 `MqttVehicle.h`를 직접 포함하고 중복 정적 정의를 제거했다. 기존 `MqttVehicle.cpp`의 공통 정의를 사용한다. 차량 동작·MQTT 설정·맵·엔진 연결 버전은 변경하지 않았다.
- 검증: 실패한 Unity 모듈의 기존 응답 파일과 같은 UE 5.7.4 헤더/PCH·컴파일러 옵션으로 재컴파일 및 DLL 링크 성공(각 종료 코드 0). 출력은 작업용 임시 폴더로 분리했다. 전체 UnrealBuildTool 재빌드와 에디터 열기/Play는 미검증이며, 사용자 환경에서 프로젝트를 다시 열어 재빌드해야 한다. AI 실행 환경의 AppData 접근 제한은 사용자가 제공한 원래 C2011 오류와 별개다.
- README·인계·상태 문서와 변경 파일의 카탈로그 크기/해시를 갱신했다. 로컬 수정이며 GitHub 미게시. Pi 접속·모터 명령 없음.

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


Unreal Engine 5.7에서 정상 실행되는 기존 프로젝트를 응급상황·구급차 주제에서 **탐사차를 운전하며 고래를 관찰하는 경험**으로 전환하는 작업 자료입니다. 고래는 관찰 대상이며 조종하지 않습니다.

목표 통신 방향은 **Unreal 탐사차 수동 입력 → MQTT → 실제 RC카**입니다. 기준 C++에 수동 입력 송신과 연결·정지 처리를 추가했습니다. 실제 RC카 연동 검증 전이며 송신은 기본 비활성화 상태입니다.

## 다른 컴퓨터에서 이어가기 — 2026-10-19 최신 인수인계

**전시 예정일은 2026-10-21(한국 시간)**이다. 최신 목표는 학교 노트북에서 기존 Unreal 프로젝트를 실행하고 **Meta Quest 3 512GB와 Air Link로 연결하는 PC VR 체험**이다. Quest 독립 실행 APK 제작은 현재 범위가 아니다. 아래 내용은 10월 18일까지의 저장소 기록과 10월 19일 사용자가 제공한 인수인계를 합친 것이며, 이번 문서 갱신으로 VR·실물 연동이 구현 또는 검증된 것은 아니다. 이 절과 아래 현재 작업 순서가 과거 기록보다 우선한다.

### 최종 구성과 장비 상태

```text
학교 노트북의 Unreal 5.7.4 ↔ Air Link ↔ Quest 3 / VR 컨트롤러
             │ 수동 탐사선 입력 (연결 구현 필요)
             ▼
      MQTT 브로커 → Raspberry Pi 수신기 → 고정된 RC카 바퀴
             │                           (별도 승인 후에만)
             └── 명령·상태 → 웹 대시보드
Unreal VR 미러 창 ── 사용자 화면 공유 선택 ──→ 웹 대시보드 영상
```

- **사용자 보고:** Quest 3는 학교에 있고, Quest Link 및 Air Link를 활성화해 PC 검색 화면까지 진입했다. 학교 실행용 노트북을 현재 사용할 수 없어 페어링은 하지 못했다.
- **사용자 보고:** `horizon.meta.com/casting` 웹 미러링으로 Quest 앱 목록이 PC 브라우저에 표시되는 것을 확인했다. Unreal VR 실행·Air Link 연결·게임 중 지연/프레임/장시간 안정성 검증과는 별개다.
- **학교 노트북:** Git/LFS 복원, UE 5.7.4·C++ 빌드 도구, Meta Horizon Link(Quest Link용 PC 앱), OpenXR 런타임, GPU/무선망 성능을 확인해야 한다. 이 노트북은 기존에 Unreal을 설치하지 못한 개인 노트북과 구분하며 사양을 추정하지 않는다.
- **집 데스크톱:** 기존 해저 프로젝트 개발·검증에 사용했다. **실제 RC카는 집에 있으며**, 현재 설치 파일·서비스·배선·실행 상태는 재확인이 필요하다. 10월 15일 Pi 조사 결과는 현재 상태의 보장이 아니다.
- **실물 시연 목표:** 바닥 주행 없이 차체를 고정하고 바퀴만 회전한다. 고정됐다는 사실만으로 모터 실행이 승인된 것은 아니다.

### 현재 완료와 미완료

| 항목 | 현재 기준 / 근거 | 남은 확인 |
| --- | --- | --- |
| 기존 해저 탐사 | 저장소 기록: Lanzaman 1종, 수중 전후진·회전·Q/E 상하 이동, 약 33m 고래 유영, 해파리 위험 구역, 추진기/기포 | 다른 PC의 빌드·저장 맵 복원과 장시간 체험 |
| 자동복귀 | 10월 18일 기록: **3D A***로 출발점까지 새 경로 계산, 장애물/위험 구역 회피·재탐색, 출발점 25cm 이내 정지 후 180도 회전 | 실제 VR에서 회전 편안함·사용자 중단·성능. 수중 기록 경로 역주행은 이전 방식 |
| 기존 검증 | 저장소 기록: UE 5.7.4 빌드, 최신 Unreal 자동화 12개 성공, Pi 오프라인 단위 검사 13개 성공 | 이번 문서 작업에서 재실행하지 않음. VR·MQTT 통합·실물 통과를 뜻하지 않음 |
| Quest 웹 미러링 | 사용자 보고로 앱 목록 표시 성공 | 체험 중 프레임·지연·안정성 |
| Air Link / VR Preview | 설정 활성화·PC 검색까지만 완료 | 학교 노트북 페어링, HMD 인식, VR Preview |
| VR 기능 | 인수인계 기준 미구현. 소스 검색에서 전용 XR 연동을 찾지 못했고 `.uproject`에 OpenXR 명시 항목 없음 | HMD 추적 카메라, 머리/선체 회전 분리, 컨트롤러 입력, VR UI. 플러그인 기본 활성 상태는 에디터에서 별도 확인 |
| 웹 대시보드 | 현재 `archive/exhibit_v1/dashboard.html`은 구급차용 UI·`car/control` 발행 코드 | 전시 UI, VR 미러 화면 공유, `veh/*` 상태 표시. 기존 화면을 전시 완성본으로 사용하지 않음 |
| MQTT / Pi | 소스 확인: `veh/cmd`, 송수신 False, 수중 모드 출력 0. 보강 수신기는 Pi 배포 전 | 수중 수동 입력 변환, 독립 브로커 dry-run, 연결/중단 검증 |

기존 코드 검사에서 `GetVehicleControlInput()`은 **수중 모드·자동 제어·수동 입력 해제 대기에서 0**을 반환한다. 모든 모드에서 무조건 0이라는 의미는 아니다. 따라서 송신 설정만 켜서는 현재 수중 탐사선 입력을 RC카에 보낼 수 없다. `DefaultInput.ini`의 예전 MotionController 축 설정도 Quest 입력 기능 구현의 증거가 아니다.

### 실행 제한과 연동 설계

- 현재 `bPublishVehicleInput=False`, `bReceiveVehicleCommands=False`를 유지한다. 실제 Pi 접속·배포·차량 프로그램/서비스 실행·하드웨어 제어 명령 전송은 명시적 재개 승인 전까지 하지 않는다. 실제 바닥 주행은 전시에서도 하지 않는다.
- 먼저 모터/GPIO 없이 **독립된 `127.0.0.1` 브로커와 `--dry-run`**으로 검증한다. 시험 시에만 UE 브로커를 loopback으로 바꾸고 별도 시험 설정을 사용한다. `192.168.0.8:1883`은 과거 주소이므로 그대로 활성화하지 않는다. MQTT 데이터만 관찰하는 것도 실제 차량 브로커와 분리해 시작한다.
- 기본 규약은 `veh/cmd {"th":0.5,"st":-0.2}`, 상태는 `veh/state`, 모의 상태는 `veh/state/dry-run`이다. UE는 TCP 1883, 브라우저는 별도 MQTT WebSocket 리스너(기존 예시 9001)가 필요하다. 브라우저는 raw MQTT TCP에 직접 연결하지 않는다.
- 탐사선의 수평 수동 입력을 바퀴 명령에 대응시키는 정책을 설계한다. 상승/하강·HMD 머리 회전은 바퀴 명령에 섞지 않는다. 제자리 회전 매핑은 별도 결정하고 검증한다. 자동복귀·도착 회전·구급차 이벤트의 실물 출력 차단은 유지한다.
- 명령 단절·연결 해제·비정상 입력·종료 시 정지, 재접속 후 정지 상태를 유지한다. 수신기 0.3초 워치독은 모터 출력 해제 기준이고 실제 바퀴 정지 시간과는 다르다. 상태의 좌우 출력값을 측정 속도·위치로 표시하지 않는다.

### 다음 작업 우선순위와 파일

| 순서 | 작업 및 주요 위치 | 완료 판단 |
| --- | --- | --- |
| 1 | `docs/AI_HANDOFF.md`, `docs/CURRENT_STATE.md`와 기준 Pawn·카메라·입력·Build.cs·플러그인 조사. 학교 노트북 사용 가능 시 사양/복원을 병행 | 기존 기능 보존 계획과 수정 파일 식별. 현재 PC/장비 접근 가능 여부 확인 |
| 2 | 기존 `BP_ExplorationRover`와 `MqttVehiclePawn.*`, `Camera/SubmersibleViewComponent.*`, PlayerController에 VR 지원 설계·구현 | OpenXR 기반 HMD 카메라·머리/선체 회전 분리, Quest 스틱 조작, 키보드 유지. 자동복귀 회전과 UI를 VR에서 확인 |
| 3 | `archive/exhibit_v1/dashboard.html`, `dashboard_server.py`를 전시용으로 개편 | 메인 VR 영상, 오른쪽 연결/RC 상태, 하단 명령/텔레메트리. 화면 공유 시작·거부·중지·재선택, 연결 상태 표시 |
| 4 | `MqttVehicleBridgeSubsystem.*`, Pawn 입력, `raspberry-pi/rover/rover_drive.py`, `raspberry-pi/rover/tests/test_rover_drive.py` | 수평 수동 입력 변환과 독립 브로커 dry-run. 중립·단절·재접속·종료·자동 제어 차단 검증. 하드웨어는 별도 승인 |
| 5 | 이 README 및 AI_HANDOFF에 학교 설치·시작/종료 가이드 정리 | 실행 PC 경로·버전·실제 브로커·검증 결과를 기록. 장비 없는 검사와 실제 장비 검사를 구분 |

소스 위치는 `unreal/MqttVehicle/Source/MqttVehicle/` 기준이다. 프로젝트·맵·탐사선·3D A*를 새로 만들지 않고 기존 기능을 재사용한다. `archive/exhibit_v1/car_subscriber.py`와 `car/control` 계열은 레거시이며 현 `veh/cmd` 수신기에 그대로 연결하지 않는다. VR 버튼별 최종 배치와 상승/하강 조작은 아직 결정되지 않았다.

### VR 화면을 웹에 표시하는 방법 — 구현 예정

같은 학교 노트북에 뜨는 Unreal VR 미러 창을 사용자가 선택하고, `getDisplayMedia()`가 돌려준 스트림을 대시보드의 영상 요소에 표시하는 방식을 우선한다. 공유 버튼 클릭·창 선택·허용이 필요하며 허용 거부와 영상 트랙 종료를 처리해야 한다. 이 기능에는 secure context가 필요하므로 같은 PC의 `http://localhost` 또는 HTTPS를 사용한다. 기존 Pi의 `http://사설IP:5050` 페이지를 그대로 열어 화면 공유가 된다고 가정하지 않는다. [화면 공유 API](https://developer.mozilla.org/en-US/docs/Web/API/MediaDevices/getDisplayMedia), [localhost 보안 컨텍스트](https://developer.mozilla.org/en-US/docs/Web/Security/Defenses/Secure_Contexts).

Meta 공식 캐스팅 페이지의 iframe 삽입을 기본으로 삼지 않는다. 현재 성공한 공식 웹 미러링은 별도 확인 경로다. `getDisplayMedia()`만으로 다른 PC에 영상이 전송되지는 않으므로 대시보드는 우선 Unreal 실행 노트북에서 표시한다. 별도 기기에 영상을 보낼 때는 추가 전송 방식이 필요하다. 화면 선택 중 Unreal 포커스 정지와 VR 포커스 복원도 검사하며 기존 정지 보호를 임의로 끄지 않는다.

### 학교 노트북에서 시작할 순서

아래는 **복원 및 개발/현장 검증 체크리스트**다. 미구현 VR·대시보드 기능이 이미 실행 가능한 것으로 읽지 않는다.

1. 비공개 저장소 권한·Git/LFS로 아래 내려받기 명령을 실행한다. 기존 checkout이면 `git status --short`로 로컬 수정을 확인한 뒤 깨끗한 상태에서 `git pull --ff-only`, `git lfs pull`, `git lfs fsck`를 실행한다. 수정 중인 데스크톱 맵을 덮어쓰거나 강제 초기화하지 않는다.
2. UE 5.7.4와 Visual Studio C++/SDK를 확인하고 `unreal/MqttVehicle/MqttVehicle.uproject`를 Development Editor / Win64로 빌드한다. 저장된 맵 `/Game/Underwater/Maps/L_UnderwaterExploration`, GameMode `BP_UnderwaterGameMode`, Pawn `BP_ExplorationRover`를 사용한다.
3. MQTT를 끈 상태에서 비VR 실행과 기존 탐사·카메라·자동복귀를 확인한다. VR 미적용 상태를 Air Link 오류로 판단하지 않는다.
4. 학교 노트북에 Meta Horizon Link PC 앱을 준비하고 OpenXR 플러그인·활성 런타임을 확인한다. 여러 런타임이 설치돼 있으면 사용할 런타임을 명확히 한다. [Epic OpenXR 안내](https://dev.epicgames.com/documentation/en-us/unreal-engine/openxr-prerequisites-in-unreal-engine?application_version=5.7).
5. 학교의 무선망에서 Quest 3와 노트북을 Air Link 페어링하고 연결·인식을 확인한다. 학교 네트워크에서 기기 간 통신이 가능한지 확인한다. 웹 캐스팅 성공으로 이 단계를 대체하지 않는다.
6. VR 지원 코드 적용·빌드 후 VR Preview에서 HMD 추적·컨트롤러·키보드·수동 정지·자동복귀·사용자 편안함과 프레임을 확인한다. VR 성능/지원 사양은 실제 학교 노트북에서 측정한다.
7. 전시용 개편 후 대시보드 서버를 실행하고 같은 노트북의 localhost 주소를 연다. 공유 버튼으로 Unreal 미러 창을 선택한다. 기존 Flask 서버 포트는 5050이며 최종 실행 명령·의존성은 개편 결과와 함께 갱신한다.
8. 독립 loopback MQTT TCP/WS 브로커를 준비하고 가짜 수신기와 명령·상태를 확인한다. 기존 수신기 CLI 예시는 `python raspberry-pi/rover/rover_drive.py --dry-run --broker 127.0.0.1 --port 1883`이며 Python/Paho 2.x 설치와 독립 브로커가 선행돼야 한다. 이번 문서 작업에서 실행하지 않았다.
9. VR 입력→MQTT→모의 출력, 입력 해제·비정상 입력·연결 단절/복구·화면 공유 중지·VR 종료를 검사한다. 하드웨어 연결 없이 완료 가능한 단계다.
10. 고정 RC카 바퀴 시연은 사용자 승인 후 별도 진행한다. 전시 시작 시 모의/실물 모드와 연결 대상을 확인하고 정지 상태에서 체험을 시작한다. 종료 시 먼저 정지·출력 0을 확인하고 제어 송신을 끈 다음 화면 공유·VR·수신기를 종료한다. 실제 바퀴 정지 여부는 현장에서 확인한다.

### 다음 컴퓨터의 AI에 전달할 시작 요청

> README의 2026-10-19 인수인계, AGENTS.md, docs/AI_HANDOFF.md, docs/CURRENT_STATE.md를 먼저 읽어줘. 기존 Unreal 5.7.4 해저 탐사 맵과 Lanzaman·수중 이동·3D A*를 유지하고 Quest 3 Air Link PC VR → 전시 대시보드 → 모터 없는 MQTT dry-run 순서로 작업해줘. 시작 시 현재 checkout/커밋·변경 파일·UE 설치·장비 사용 가능 여부를 확인해줘. HMD 머리 회전과 선체 회전을 분리하고 수중 입력의 현재 MQTT 0 반환 정책을 검토해줘. Pi 접속·모터 명령·고정 RC카 바퀴 회전도 명시적 승인 전에는 하지 마. 학교 노트북/Quest가 없으면 코드·설정·대시보드 작업과 장비 검증을 구분해줘. 새 파일은 생성 즉시 알리고, GitHub 업로드는 변경 목록 확인을 받은 뒤 진행해줘. Unreal 콘솔은 사용자가 실행할 완전한 명령을 제공해줘.

### 이번 문서 갱신의 검증 범위

2026-10-19에 원격 `main`의 기준 커밋 `321db9f`를 받아 기존 문서와 사용자가 붙여준 인수인계 전체를 읽었다. `.uproject`, Pawn 출력 조건, MQTT 설정, Pi 수신기 CLI, 기존 대시보드/서버 소스를 정적으로 대조했다. 이번 변경은 문서와 파일 카탈로그만이며, UE 빌드·Play·VR Preview·화면 공유·브로커 접속·Pi·모터·실물 검증을 새로 수행하지 않았다. 10월 18일 기록상 데스크톱에만 저장하고 업로드에서 제외한 해저 맵 변경도 있으므로, 새 PC는 GitHub 버전과 데스크톱 미게시 변경을 구분해야 한다.

## 작업 기록과 GitHub 업로드 규칙

사용자 요청일: 2026-10-14.

- 이 프로젝트의 추가 작업을 시작하거나 진행할 때 README를 읽고 실제 변경 내용과 대조합니다. 새로 구현·추가·확인한 내용이 빠져 있으면 작업 중 바로 보완합니다. 변경 목적, 확인한 결과, 아직 확인하지 못한 항목과 다음 작업을 구분해 기록합니다.
- 2026-10-19 인수인계 기준으로 GitHub 업로드 전에는 변경 파일과 내용을 사용자에게 확인받습니다. 이전의 기존 파일 수정 자동 푸시 규칙은 이 기준으로 대체합니다. README·인계·파일 목록 갱신은 별도 요청 없이 수행합니다.
- 소스·에셋·문서 등 보존할 새 프로젝트 파일을 만들면 **즉시 사용자에게 파일 경로와 역할을 알리고 “GitHub에 올려도 될까요?”라고 묻습니다.** 해당 파일을 포함한 업로드는 승인을 받은 뒤 진행합니다. 기존 보존 정책에서 제외한 빌드 산출물·캐시·임시 로그는 업로드 대상으로 취급하지 않습니다.
- 데스크톱의 원본 작업 폴더 `C:/Users/HEATH/Desktop/d_mix/STUDENT_HANDOFF_MQTT_2026-09-27`와 그 안의 Git 저장소 `.github-upload/`는 별도 사본입니다. 업로드할 때 변경분을 동기화하고 커밋·푸시합니다.
- 다른 AI도 같은 규칙을 적용하도록 `docs/AI_HANDOFF.md`에 함께 기록합니다.

2026-10-17 사용자 작업 방식 지정: 단계마다 백그라운드 처리 가능 여부를 판단합니다. 코드·문서 수정, 명령줄 빌드·자동화 검사·Git 작업과 지원되는 에셋 제작은 화면·마우스·키보드를 사용하지 않고 처리합니다. **열린 Unreal 에디터의 콘솔 입력은 사용자가 직접 합니다.** AI는 필요한 스크립트 파일을 준비하고 `py "절대/경로/스크립트.py"` 형태의 완전한 붙여넣기 명령과 목적을 제공합니다. Play 중지·저장·에디터 종료 같은 영향이 있으면 먼저 알립니다. 콘솔 입력을 위해 원격 화면 조작을 하지 않으며, 사용자 실행 결과나 로그를 확인한 뒤 후속 작업을 진행합니다. 그 밖에 직접 화면 조작이 필요한 단계는 이유와 시작을 짧게 알린 뒤 진행합니다. 창을 최소화한 채 화면 조작을 계속하는 방식은 사용하지 않습니다. 명령 제공만으로 실행 완료로 기록하지 않으며 직접 확인하지 않은 외형·키 입력·주행은 미검증으로 남깁니다. 이번 갱신은 작업 방식 지정이며 새 기능 구현이나 추가 테스트 통과를 의미하지 않습니다. 실차 금지와 새 파일 업로드 확인 규칙은 계속 적용합니다.

파일 목록 재생성 도구는 실행일을 기준일로 기록합니다. 특정 점검일로 생성하려면 `pwsh -File unreal/tools/build_file_catalog.ps1 -BaselineDate 2026-10-14`처럼 날짜를 지정할 수 있습니다.

## 최근 확인 상태

### 2026-10-18 추진기 회전·기포와 만남 알림

Lanzaman의 고정 추진기 뒤에 3엽 날개를 양쪽에 추가했다. 원본 GLB/선체 메시를 분해하거나 변경하지 않는다. `Effects/SubmersiblePropulsionComponent`가 실제 Unreal 위치·회전 변화를 읽어 날개를 반대 방향으로 회전시키고, 이동 속도에 따라 양쪽에서 초당 합계 최대 72개의 기포를 만든다. 전후진·상하 이동·제자리 회전에 반응하고 정지 후 감쇠한다. 기포는 월드 공간에 남아 위로 떠오르며 1.6초 내 사라진다. 128개 인스턴스 풀을 재사용하고 충돌/경로 계획/차량 제어에는 관여하지 않는다. 유체/추력 물리 모델이 아닌 시각 효과다. 기존 Vefects 환경 기포는 유지한다.

고래 관찰 구역 진입 시 우상단에 “고래를 만났어요!”, 해파리 위험 구역 진입/빠른 통과 시 “해파리 주의!”를 각각 5초 표시한다. 두 알림은 동시에 표시할 수 있고 머무는 동안 반복하지 않으며 재진입하면 다시 표시한다. 고래·해파리 메시와 직접 접촉하거나 시선으로 인식하는 판정은 아니며 기존 구역 판정을 사용한다. H/M/Tab 단축키 가이드는 우하단 24 여백으로 이동했다. 좌하단 보조 카메라는 유지한다.

**구현/검증:** UE 5.7.4 Editor 빌드 성공, Unreal 자동화 12개 성공·실패/경고 0. 기포 반경 2.5~5cm 보강 후 추진 효과 자동화 1개도 재통과했다. 추진기 2개·이동 중 기포 생성·월드 공간 유지·정지 후 소멸·수직 이동 반응·MQTT 출력 0을 확인했다. 알림의 재진입·동시 표시·반복 억제·빠른 위험 구역 통과 이벤트를 검사했다. 화면 없는 Play에서 실제 고래 관찰 구역·해파리 위험 구역 진입 알림, 우상단 동시 표시·5초 후 숨김, 우하단 단축키를 확인했다. 후방·근접·옆 시점 렌더에서 원본 추진기 뒤 날개와 상승 기포 흐름, 정지 후 소멸을 직접 검토했다. 최종 렌더 검사 성공·종료 코드 0. 초기 임시 검사 스크립트의 Python 속성/호출 접근 오류는 실제 구역 이벤트 방식으로 수정했다. 엔진 초기화의 기존 Condition failed 로그는 원인 미확인으로 자동화 결과와 구분한다. 실제 OS 키보드/마우스 장시간 주행은 미검증이다. MQTT 송수신 False·Pi/실차 미접속. 새 효과 소스 2개·재질 생성 스크립트·기포 재질은 사용자 업로드 승인 완료. 에디터 종료 전 사용자가 저장한 로컬 해저 맵 변경은 보존하고 이번 업로드에서는 제외한다.

### 2026-10-18 도착 후 180도 회전·카메라 마우스 방향 공유

수중 A* 자동복귀는 출발점 25cm 이내에서 정지를 확인한 뒤 제자리에서 180도 회전하고 도착 상태로 전환한다. 도착 당시 방향을 기준으로 반 바퀴이며 최초 출발 방향으로 정렬하는 기능은 아니다. 초당 45도로 약 4초 동안 회전하고 전후진·상하 이동은 하지 않는다. 좌우 회전 공간과 회전 중 선체 충돌을 확인하며 공간 부족/시간 초과/사용자 중단 시 정지한다. 기존 5 MPH 복귀와 MQTT 차단은 유지한다.

탐사선의 1인칭과 3인칭은 같은 yaw/pitch를 실시간 공유한다. 마우스를 움직이면 메인·좌하단 보조 영상이 함께 방향을 바꾸고 Tab 전환 후에도 유지한다. R 정면 복귀도 양쪽에 적용한다. 탐사선 두 카메라의 회전 지연을 꺼 방향 반응을 일치시켰으며 일반 차량의 카메라 동작은 유지한다.

**직접 검증:** UE 5.7.4 Editor 빌드 성공. 화면 없는 Unreal 자동화 10개 성공·실패/경고 0. 기존 실제 맵/장애물/위험 구역 복귀 사례에서 도착 회전 중 이동 없음(0.1cm 허용), 최종 방향 180도(0.2도 허용), 도착 후 제어 해제를 확인했다. 기존 ManualInput 검사에 양쪽 마우스 방향·Tab 유지·R 동기화 검사를 추가해 통과했다. 엔진 초기화의 기존 Condition failed 로그는 자동화 성공 결과와 구분하며 원인 미확인이다. 실제 화면 렌더·OS 마우스 조작은 이번에 검증하지 않았다. MQTT 송수신 False·Pi/실차 미접속 유지. 새 보존 파일은 없고 기존 소스/테스트만 수정했다.

### 2026-10-18 출발점 3D A* 자동복귀

수중 탐사선의 기록 경로 역주행을 **현재 위치에서 출발점까지 새 경로를 계산하는 3D A***로 교체했다. 출발 앵커 근처의 실제 시작 위치만 보관하며 이동 이력은 필요 없다. 직선이 안전하면 곧바로 복귀하고, 막히면 상하를 포함한 26방향 격자 탐색 후 충돌 없는 지름길로 경유점을 줄인다. 모든 방향 회전을 포함하는 보수적 선체 박스로 장애물과 위험 구역을 검사한다. 위험 구역 안에서 시작하면 탈출은 허용하고 재진입은 피한다. 외부 라이브러리 없이 구현했다.

5초 준비 후 경로를 계산하고 다음 목표 방향으로 제자리 회전해 전진한다. 매번 180도 회전을 강제하지 않는다. 순항 5 MPH·수직 1m/s·출발점 25cm 이내 정지와 사용자 중단은 유지한다. 새로운 장애물 또는 8초 정체 시 최대 3회 재탐색하고, 경로가 없으면 정지한다. 일반 차량용 기존 기록 경로 코드는 호환용으로 남긴다.

탐색 간격 150cm, 최대 30,000개 노드·15초·출발점까지 200m 제한이다. 탐색 범위는 두 끝점에서 수평 40m·아래 15m·위 30m 여유를 두고 수중 상한으로 제한한다. 프레임당 최대 48개 작업과 약 2ms의 소프트 시간 예산으로 분할한다. 정적 장애물의 제한된 격자 안에서 최단 비용을 찾으며, 연속 공간 전체의 절대 최단 경로나 매우 좁은 통로 통과를 보장하지 않는다.

**직접 검증:** UE 5.7.4 Editor 빌드 성공, 화면 없는 Unreal 자동화 10개 성공·실패/경고 0. 기록 없는 30m 직선 복귀, 벽 우회, 동적 장애물 재탐색, 위험 구역 회피/탈출, 막힌 시작점 정지, 수동 취소와 알려진 최단 격자 비용을 검사했다. 실제 저장 해저 맵을 로드한 출발점 복귀도 성공했다. 중간 경유점 허용오차를 8cm로 맞춰 이동의 5cm 중립 구간과 충돌하지 않게 했고 정적 벽 사례의 불필요한 재탐색 0회를 확인했다. 이는 GamePreview 자동화이며 실제 H 키·포커스·전체 맵 장시간 통합 주행은 미검증이다. MQTT 송수신 False·출력 0, Pi/실차 미접속 유지. 새 UnderwaterPathPlanner.h/.cpp 업로드는 사용자 승인 완료.

### 2026-10-17 Lanzaman 전용·좌하단 영상 확대·안내 최소화

플레이 가능한 탐사선은 **Lanzaman 하나로 고정**했습니다. 1번 IrisProcess 선택 및 1/2 전환 키를 제거하고, 기존 선택 API에서도 Iris 요청을 거부합니다. 시작/재생성 시 Lanzaman을 적용합니다. 원본 Iris 자료와 이전 에셋은 보관하며 실행 중 선택 대상에서는 제외합니다.

우상단은 **H 자동복귀·M 수동 전환·Tab 시점 전환**만 표시합니다. 다른 조작키의 안내만 제거했으며 W/S·A/D·Q/E·정지·카메라 조작 기능은 유지합니다. 보조 카메라는 **좌하단 512×288**(직전 400×225 대비 가로·세로 28% 확대), 왼쪽/아래 여백 24로 바꾸고 작은 설명 글씨와 제목 줄을 제거했습니다. 16:9 영상·640×360 캡처·최대 20Hz·Tab 교환과 속도/기어 숨김은 유지합니다.

**검증:** Editor 빌드 성공. 기존 ManualInput 자동화 1개 성공·실패/경고 0: Iris 선택 거부·Lanzaman 유지와 기존 카메라/입력 회귀를 확인했다. 화면 없는 Play의 내부/외부 메인 시점에서 Lanzaman, 좌하단 512×288/24 여백, 보조 영상의 텍스트 없음, H/M/Tab 세 줄, 속도/기어 숨김을 확인하고 렌더 2개를 직접 검토했다. 검사/종료 코드 0. 렌더 초기화의 기존 Condition failed 4줄은 별도 기록하며 전체 주행/실차 검증으로 해석하지 않는다. MQTT 송수신 False·Pi/실차 미접속을 유지합니다.

### 2026-10-17 보조 화면 확대·오른쪽 배치와 단축키 UI

보조 카메라를 **우하단**으로 옮기고 영상 영역을 **320×180 → 400×225** 논리 크기로 확대했습니다(가로·세로 각각 25%). 오른쪽·아래 여백은 24이며 16:9 비율, 640×360 캡처·최대 20Hz와 Tab 시점 교환은 유지합니다. 탐사선 2종에서는 기존 속도·기어 HUD를 숨기고 일반 차량 템플릿은 유지합니다.

우상단은 W/S·A/D·E/Q·Space/X·H/M·마우스/R·Tab·1/2의 **단축키와 기능 이름만** 표시합니다. 관찰 설명·거리·발견/복귀/도착/위험 상태 문장은 표시하지 않습니다. 탐사선 선택이 불가능한 동안 1/2 안내는 흐리게 표시합니다. 기존 관찰·자동복귀·정지 판정은 계속 동작합니다.

**직접 검증:** UE 5.7.4 Development Editor 빌드 성공. 화면을 띄우지 않는 Play에서 내부/외부 메인 시점 각각 우하단 400×225·24 여백·속도/기어 Collapsed·단축키 텍스트와 반대 시점 영상을 확인했고 UI 포함 렌더 2개를 직접 확인했습니다. 검사와 프로세스 종료 코드 0. 기존 엔진 초기화 Condition failed 4줄은 남아 있어 전체 자동화 통과로 표시하지 않습니다. 주행·물리 키 입력·실차 시험은 하지 않았습니다. MQTT 송수신 False·Pi 미접속 유지. 사용자 직접 콘솔 명령으로 에디터 종료 후 빌드했으며, 다시 프로젝트를 열면 적용됩니다. 임시 검사·렌더·로그는 업로드하지 않습니다.

### 2026-10-17 곡선 자동복귀의 잘못된 정체 판정 수정

사용자가 보고한 “차량이 진행하지 못해 정지했습니다”는 남은 거리가 8초 동안 충분히 줄지 않을 때의 정체 감지 메시지입니다. 기존 수중 추종은 매 프레임 앞쪽 목표 경로점을 다시 선택해, 곡선의 선택 경계에서 목표 방향이 번갈아 바뀌고 방향 정렬을 위한 제동이 반복됐습니다. 장애물 없는 재현 검사에서도 곡선·상하 혼합·S자 경로 3개가 같은 메시지로 실패했습니다.

수중 복귀는 **선택한 목표점을 통과할 때까지 유지**하고, 건너뛸 기록점이 직선 진행 구간에서 20cm 이내인 경우에만 앞쪽 목표를 선택합니다. 선체 전체 크기로 해당 구간의 충돌도 확인합니다. 경로 순서·높이를 유지하며 실제 장애물이 막으면 기존 정체 정지를 유지하고 장애물 안내를 표시합니다. 정체 시 경로점·위치·목표·속도 진단 로그를 남깁니다. 자동 순항 5 MPH·제자리 회전·사용자 중단·포커스 정지·MQTT False는 유지합니다.

**검증:** UE 5.7.4 Editor 빌드 성공, Unreal 자동화 **8개 성공·실패/경고 0**. 새 `UnderwaterReturn` 검사는 기존 테스트 파일에 추가했으며 저장된 탐사선 Blueprint의 실제 수동 이동을 기록해 실제 복귀 컴포넌트·이동 코드로 추종합니다. 직선·곡선·상하 곡선·수직 구간·S자 5개 경로 × 20/30/60Hz = **15개 복귀 성공**, 모두 출발점 25cm 이내 도착·최고 5.000 MPH. 추가 장애물 사례는 벽 앞 정지·자동 제어 해제·장애물 제거 후 기록 경로 재시도 성공을 확인했습니다. GamePreview에서 포커스·MQTT와 분리한 검사이며 사용자의 당시 54개 경로점 전체를 재생한 것은 아닙니다. 실제 맵 전체 동선·물리 키 장시간 조작·실차 검증은 이 결과에 포함하지 않습니다. MQTT 송수신 False·Pi 미접속을 유지했습니다.

### 2026-10-17 유리 존재감을 위한 약한 불투명도

완전 투명하면 창이 없는 것 같다는 사용자 피드백에 따라 Lanzaman 유리 불투명도를 **0 → 0.08(8%)**로 조정하고 옅은 청록색을 넣었습니다. 처음의 28%보다 약한 표현이며 내부·외부·보조 카메라에 동일하게 적용합니다. 50cm 높인 1인칭 위치 `(-90,-62,320)`cm는 유지합니다. 기존 재질과 재현 스크립트를 갱신했으며 C++·주행 코드는 변경하지 않았습니다.

**직접 확인:** 열린 에디터에서 기존 재질 저장, 별도 창 없는 Play에서 저장 불투명도 0.08·기존 카메라 Z=320cm 재확인 및 전방/좌석/외부 렌더 3개 확인. 배경을 가리지 않는 약한 유리 색을 확인했습니다. 렌더와 에디터 종료 로그는 완료되었지만 실행 도구가 종료 코드 1을 반환했고 초기화의 기존 `Condition failed` 4줄도 남았습니다. 원인은 미확인으로 기록하며 전체 테스트 통과로 표시하지 않습니다. C++ 빌드·자동화·주행 시험은 추가 실행하지 않았고 MQTT 송수신 False를 유지했습니다.

### 2026-10-17 조종석 유리 완전 투명화와 시점 상승 (이전 설정)

Lanzaman의 `M_Lanza_Glass` 불투명도를 **0.28 → 0**으로 바꾸고 반사·굴절·유리 색이 시야를 덮지 않도록 비조명 투명 재질로 저장했습니다. 내부/외부/보조 카메라는 같은 투명 유리를 사용하며, 창틀과 좌석 2개·아래 모니터 1개는 유지합니다. 1인칭 위치를 `(-90,-62,270)` → **`(-90,-62,320)`cm로 50cm 높여** 모니터 위로 정면 배경이 보이도록 했습니다. 시작·Lanzaman 재선택·Blueprint 기본값·재현 스크립트에 같은 위치를 적용했습니다. Iris 시점은 기존 값을 유지합니다.

**직접 검증:** UE 5.7.4 Editor 빌드 성공. `MqttVehicle.Control.ManualInput` 1개 성공(테스트 실패/경고 0), 카메라 위치·탐사선 재선택·Tab 액션 검사 포함. 저장 에셋을 새 프로세스에서 읽어 불투명도 0·카메라 Z=320cm를 확인했고, 창 없는 Play의 전방·좌석·외부 영상 3개를 렌더해 중앙 시야와 아래 모니터를 확인했습니다. 렌더 프로세스 종료 코드 0. 엔진 초기화의 기존 `Condition failed` 4줄은 남아 있으며 테스트 결과와 구분합니다. 이번에는 추가 주행/물리 키 입력 시험을 하지 않았고 MQTT 송수신 False·Pi/실차 미접속을 유지했습니다.

### 2026-10-17 Q/E 상승·하강 및 수중 이동 구현

Q/E는 앞서 제안만 했고 입력/이동 코드가 없었습니다. 이번에 **E 상승·Q 하강**, 키를 놓으면 해당 방향 정지와 깊이 유지, 물속 W/S 전후진·A/D 회전을 실제 연결했습니다. Play 화면을 한 번 클릭해 입력 포커스를 준 뒤 조작합니다. 다른 창으로 전환하면 기존 포커스 상실 정지가 작동합니다.

- 탐사선 2종은 바퀴 접지 대신 충돌 검사를 동반한 수중 이동을 사용합니다. 유체 부력 시뮬레이션이 아닌 체험용 이동이며 기존 일반 차량은 Chaos 주행을 유지합니다. 상승/하강 최고 1m/s, 시작 높이보다 최대 30m 상승, 수동 전진 최대 6 MPH·후진 3km/h입니다. HUD는 수중 전체 이동 속도를 표시합니다.
- 선체 전체 Bounds 박스로 이동/회전을 검사해 바닥·벽 관통을 막습니다. 두 수직 키 동시 입력은 상쇄하고 Space/X·포커스 상실 후에는 누르던 키를 모두 놓아야 다시 출발합니다. 충돌은 보수적인 박스 근사이며 정밀 선체 형상·물리 부력은 아닙니다.
- 자동복귀는 높이를 포함한 실제 경로를 역순으로 전진 추종합니다. 제자리 회전·중단/정체 정지는 유지하고, 도착은 기록 출발점의 3D 거리 25cm 이내입니다. 자동 순항 목표 5 MPH, 수직 속도 1m/s 제한과 도착 감속을 적용합니다. 관찰 패드의 수평 6m/7.5m 판정은 유지하고 수중 모드의 높이 허용 범위는 33m로 넓혔습니다. 위험 구역은 기존 3차원 박스를 유지합니다.
- 수중 모드의 MQTT용 출력은 현재 0이며 송수신 기본 False입니다. 실제 RC카 재개 시 수중 조작과 실차 수평 입력의 연결을 별도로 확인해야 합니다. Pi 접속·모터 송신·실차 주행은 하지 않았습니다.

**검증:** UE 5.7.4 Editor 빌드 성공. 자동화 7개 성공, 실패/경고 0. 이어 새 수중 테스트를 실제 Q/E 키 이벤트→런타임 Enhanced Input 매핑→이동 경로로 보강해 다시 통과했습니다. 3초 상승 273.1cm, 키 해제 호버링·Q 하강·동시 키·Space/정지 유지·재입력·공중 전진 6 MPH·자동 5 MPH·벽/바닥 충돌·30m 상한·수동 전환을 검사했습니다. 초기 테스트의 임시 입력 객체 수명 문제를 보강한 뒤 통과한 결과입니다.

**실제 저장 맵 Play:** 입력 API로 373.50cm 상승 → 2초 높이 변화 0cm → 124.01cm 하강 → 공중 전진 879.44cm → 제자리 회전 및 높이를 따라 복귀 → 출발 위치 24.75cm 이내 도착 정지(복귀 약 18.5초)를 확인했습니다. 창 없는 Play는 포커스가 없어 정지했고 화면에 포커스를 준 실행에서 성공했습니다. 이 Play의 이동은 임시 입력 API로 수행했으며 OS 물리 키보드 장시간 주행과 구분합니다. 기존 긴 경로·동적 장애물·전체 이벤트/성능 검증은 남아 있습니다. 최신 에디터를 다시 열어 반영했고 문서/파일 목록을 갱신했습니다.

### 2026-10-17 전방 시야를 위한 위쪽 모니터 제거

Lanzaman 조종석의 **위쪽 모니터와 연결 지지대를 제거**하고 아래 모니터 1개·좌석 2개·창틀·조종 장치를 유지했습니다. 하나의 메시를 쓰므로 1인칭과 외부 시점 모두 같은 수정본을 표시합니다. 원본 GLB는 보존하고 기존 `Converted/SM_Lanza.fbx`와 `Selected/Meshes/SM_Lanza.uasset`를 갱신했습니다. Blender 재현 스크립트에도 동일한 제거를 반영했습니다.

**직접 확인:** 원본의 모니터 오브젝트만 수정하고 나머지 25개 오브젝트·탐사선 전체 크기·재질 슬롯을 유지했습니다. 열린 에디터에서 재임포트·저장 후 별도 창 없는 Play로 저장 메시를 다시 읽고 전방·좌석·외부 반대 영상을 렌더해 확인했습니다. 재임포트에는 FBX 스무딩 그룹 누락 경고가 있었으며 기존 재질 2개를 그대로 연결했습니다. C++ 변경·빌드·주행 시험은 추가하지 않았습니다. MQTT 송수신 False와 실차 금지를 유지했습니다. 아래 모니터 2개 설명은 이전 기록입니다.

**수중 상하 이동 문의:** 구현 가능하며 아직 추가하지 않았습니다. 현재 Chaos 바퀴 차량과 수평 경로 추종을 수중 추진·상승/하강·정지 호버링·충돌 처리로 바꾸고, 높이를 포함한 자동복귀 및 관찰/위험 구역 판정도 함께 조정해야 합니다. E 상승/Q 하강은 제안이며 현재 매핑이 아닙니다. 실제 RC카는 계속 비활성입니다.

### 2026-10-17 고래를 현재 크기의 2배로 확대

사용자가 직접 체험한 뒤 고래가 작게 느껴진다고 요청하여 **16.5m → 약 33m**로 확대했습니다. 가로·세로·높이를 동일하게 2배로 적용했으며 기본 Lanzaman 탐사선(앞뒤 길이 3.8m)의 약 **8.7배 길이**입니다. 저장 맵의 고래 크기와 재현 스크립트 `WHALE_LENGTH_CM=3300`을 함께 반영했습니다. 반경 8m·55초 원형 이동, 기존 유영 애니메이션, 관찰 판정과 차량 설정은 유지합니다.

**직접 확인:** 열린 에디터에서 확대·맵 저장 완료. 별도 화면 없는 Unreal 프로세스로 저장 맵을 다시 읽어 길이 3300cm·이전 대비 2배·균일 축척, 반경/주기와 애니메이션 연결, 액터 374개·MQTT 송수신 False를 확인했습니다. 이번 변경은 크기 설정과 맵 수정이며 C++ 빌드·Play 주행 시험은 추가 실행하지 않았습니다. 아래 16.5m 설명은 이전 기록입니다.

### 2026-10-17 사진 기준 Lanzaman 내부와 자동복귀 5 MPH

사용자가 제공한 정면/후면 사진을 기준으로 1인칭을 수정했습니다. 기존 `SM_Lanza`에 이미 포함된 **좌석 2개와 앞쪽 위아래 모니터 2개**, 구형 창틀·조종 장치를 그대로 사용합니다. 이전 임시 조종석이 원본 내부를 숨기던 처리를 고치고 왼쪽 좌석 눈높이 `(-90,-62,270)`cm에 카메라를 배치했습니다. 은은한 실내 조명을 추가하고 모니터의 원본 그래픽을 유지했습니다. 모니터 그림은 장식이며 실제 주행 속도는 기존 HUD가 표시합니다. IrisProcess는 기존 별도 조종석을 사용하며, Tab 메인 전환과 좌하단 반대 영상은 유지합니다. 원본 모델/텍스처는 변경하지 않았습니다.

자동복귀 목표는 **5 MPH(8.04672km/h)**, 수동 전진 제한은 **6 MPH**, 수동 후진은 3km/h입니다. `AutomaticCruiseSpeedMph`를 별도 설정으로 추가했고 작은 조향 보정 때문에 목표가 크게 낮아지던 계산을 완화했습니다. 강한 조향에서는 최대 20% 감속하며 출발점 접근 시 정지를 위한 감속은 유지합니다. 제자리 180도 회전 중에는 이동하지 않습니다.

**직접 검증:** UE 5.7.4 Editor 빌드 성공. Unreal 자동화 6개 성공, 실패/경고 0개. 별도 GamePreview 평지 물리 시험에서 저장된 탐사차와 실제 추종/입력 코드를 사용해 10초 동안 약 20.94m 이동, 최고 **5.013 MPH**, 마지막 5초 평균 **4.926 MPH**, MQTT용 출력 0을 확인했습니다. 이는 가상 평지 직선 시험이며 긴 곡선·경사·실차 주행 결과는 아닙니다. 카메라 위치·Lanzaman/Iris 시야 분리·Tab 액션·조향 감속·도착 제동 회귀 검사도 통과했습니다.

최종 창 없는 Play 렌더에서 전방의 위아래 모니터·조종 장치, 옆을 돌아봤을 때 두 좌석, 외부 보조 시점을 확인했습니다. 실제 RC카 주행·Pi 접속·MQTT 송수신은 하지 않았으며 기본 False를 유지했습니다. 기존 파일만 수정했고 임시 렌더·로그·시험 도구는 업로드하지 않습니다.

### 2026-10-17 Lanzaman 조종석·두 시점·제자리 회전 적용 (이전 기록)

최신 사용자 요청에 따라 **Lanzaman 기본 선택 + 조종석 1인칭 시작**, **좌하단 반대 시점 영상**, **제자리 회전**, **수동/자동복귀 공통 6 MPH**를 적용했습니다. 아래 속도·고래 절의 전진 원호 유턴·자동 6km/h와 이전 IrisProcess 기본 선택은 과거 기록입니다.

- **속도:** 수동 전진 제한과 자동복귀 직선 목표를 같은 6 MPH(9.656064km/h) 설정에서 읽습니다. 곡선과 도착 직전에는 감속하고 후진 제한은 3km/h를 유지합니다.
- **제자리 회전:** 전후진 키를 놓고 A/D 또는 좌우 방향키만 누르면 정지한 뒤 몸체가 초당 45도 회전합니다. 자동복귀는 좌우 공간을 검사하고 제자리 180도 회전 후 기록 경로를 전진 추종합니다. 회전 도중 장애물·기울기·위치 이동·시간 초과가 감지되면 정지합니다. Unreal 내부 동작이며 자동복귀와 제자리 회전 입력은 MQTT 출력에서 0입니다.
- **조종석과 시점:** 기본은 Lanzaman의 내부 시점이며 Tab으로 외부 3인칭과 바꿉니다. 좌하단에는 반대 시점의 640×360 영상이 최대 초당 20회 갱신됩니다. 관찰/복귀 안내는 우상단으로 옮겼습니다. 기존 1=IrisProcess, 2=Lanzaman 선택 조건은 유지합니다.
- **조종석 구성:** 기존 외형과 별도로 창틀·벽·계기판·조종간을 코드로 구성했습니다. 내부 시점에서 외형이 시야를 가리지 않게 하고 외부 시점에서는 내부 전용 부품을 숨깁니다. Lanzaman 원본의 완성된 내부 모델을 새로 받은 것은 아닙니다. 수중 상승·하강 이동은 추가하지 않았습니다.
- **저장과 재현:** 승인된 Camera 소스 4개와 `M_CameraFeed.uasset`, 기존 탐사차 Blueprint·제작 스크립트를 반영했습니다. 다른 PC에서는 최신 C++를 빌드한 뒤 저장 에셋을 사용합니다. `unreal/tools/build_selected_underwater.py --settings-only`로 설정을 재적용할 수 있습니다.

**직접 검증:** UE 5.7.4 Development Editor 빌드와 Unreal 자동화 5개 성공, 실패/경고 0개. 저장된 Tab 매핑의 액션을 실제 C++/Blueprint 입력 바인딩에 전달해 누름 1회 전환·누른 상태 유지·다시 눌러 복귀를 확인했습니다. 화면을 사용한 짧은 Play에서는 약 10.9m 이동 후 제자리 180도 회전→전진 자동복귀→출발점 약 98cm 이내 정지, 자동복귀 최고 약 6.00 MPH와 MQTT용 입력 0을 확인했습니다. 이 구간의 이동·복귀 요청은 임시 검증 스크립트 API를 사용했습니다. 두 메인 시점과 좌하단 반대 영상 표시를 화면에서 확인했고, 마지막 조종간/문구 위치 조정 후에는 창을 띄우지 않는 Play 렌더로 내부·외부 영상을 다시 확인했습니다.

**남은 검증:** 실제 Tab 키의 화면 입력은 포커스 간섭으로 최종 재확인하지 못해 자동화 결과와 구분합니다. 긴 곡선·경사·발견/위험 이벤트·회전 중 장애물 진입·장시간 성능은 추가 확인이 필요합니다. 실제 RC카 주행·Pi 접속·MQTT 송수신은 하지 않았고 두 기본 설정은 False입니다. 임시 검사·렌더·로그·빌드 파일은 업로드하지 않습니다.

### 2026-10-17 속도·고래·자동복귀 수정 및 오프라인 Play 확인

사용자가 승인한 수동 최고속도 **6 MPH(9.656064km/h)**와 고래 **1.5배·약 16.5m**를 기존 코드와 저장 에셋에 적용했습니다. 아래 10월 16일 절은 이전 상태 기록이며, 최신 상태는 이 절과 CURRENT_STATE를 따릅니다.

- **속도:** 기존 km/h 계산과 MPH 문구의 불일치를 수정했습니다. MPH 환산은 `cm/s ÷ 44.704`이며 숫자와 단위 문구를 같은 설정으로 동기화합니다. 수동 후진 제한은 3km/h를 유지합니다.
- **고래:** 원본 머리가 로컬 -X인 것을 뼈대에서 확인하고 원형 이동을 시계 방향으로 뒤집었습니다. 반경 8m·55초 주기와 원본 유영 애니메이션은 유지하며, 저장된 길이는 16.5m입니다.
- **자동복귀:** 조향 중 약 15% 가속으로 출발하지 못하던 현상을 재현해 출발 가속을 보강했습니다. 직선 목표를 5→6km/h로 높이고 회전 중에는 감속합니다. 유턴 중 8초 동안 30cm 이상 진행하지 못하면 정지합니다.
- **주행 공간:** 낮은 장애물까지 Vehicle 충돌 채널로 검사하도록 수정했습니다. 바퀴를 걸리게 하던 안내등 받침대 12개는 충돌 없는 동선 표지로 정리했습니다. 바위 등 실제 장애물은 공간 검사 대상입니다.
- **저장·재현:** `unreal/tools/build_selected_underwater.py --settings-only`로 재임포트·재배치 없이 이번 설정을 적용할 수 있습니다. 고래·해초·바위 재질 사용 플래그도 저장해 다른 PC에서의 재로드에 반영했습니다.

**직접 검증:** UE 5.7.4 Development Editor 빌드 성공. 기존 자동화 5개에 실제 HUD 단위·머리/꼬리 기준 전진 유영·조향 출발·낮은 장애물 회귀 검사를 추가했고 5개 모두 성공, 테스트 실패/경고 0개입니다. 최종 저장 맵 Play에서 임시 측정 스크립트의 수동 입력 API로 약 5.2m를 주행하고, 실제 H 키로 복귀를 요청했습니다. 전진 유턴→기록 경로 복귀→도착 정지까지 약 56초(준비 시간 포함), 출발 앵커 약 98cm 이내에서 속도 0과 핸드브레이크를 확인했습니다. 복귀 중 전진 기어와 송신용 입력 조회 0을 유지했습니다. 평지 수동 최고 약 5.99 MPH, 이 짧은 복귀 구간 최고 약 4.90km/h를 측정했습니다. 고래는 Play에서도 머리 방향으로 이동했습니다.

**확인 범위와 남은 작업:** 짧은 출발 구간의 복귀 완주 결과이며, 긴 곡선 경로·경사·후진 속도·위험/발견 자동 이벤트·8초 정체 중단의 통합 시험과 최종 성능 검증은 남아 있습니다. 별도 저장 맵 검사에서는 오류 0개와 기존 해파리 구성의 확대 BoundsScale/동적 그림자 성능 경고 27개가 나왔습니다. 재질 사용 플래그 3개·맵 374개 액터·고래 크기·안내등 충돌 저장은 재로드로 확인했습니다.

**수중 자유 이동:** 구현 가능하지만 현재는 Chaos 바퀴 차량입니다. 상승·하강/수중 추진 이동과 높이를 포함한 복귀 경로를 별도 설계해야 합니다. 사용자는 가능 여부를 물었으며 이번에는 이 이동 방식을 추가하지 않았습니다.

실제 RC카 주행·Pi 접속·모터 송신은 하지 않았습니다. MQTT 송수신 False와 브로커 접속 생략을 유지합니다. 기존 파일만 변경했으며 임시 측정 코드·로그·빌드 산출물은 업로드하지 않습니다.

### 2026-10-16 선택 에셋 적용·저장 검증 완료, Play 통합 주행 확인 남음

사용자가 지정한 무료 에셋과 이번 구현 파일 묶음은 업로드 승인되었습니다. 원본은 `assets/selected_free_assets/`, 제작자·라이선스·변경 사항은 [ASSET_CREDITS](docs/ASSET_CREDITS.md)에 기록했습니다. Vefects는 사용자가 무료 Personal 에셋을 취득한 뒤 Unreal Fab에서 가져왔습니다.

- **탐사선 2종:** 기본 IrisProcess Submarine, 선택 Lanzaman Spherical Submersible. 출발점 5m 안에서 정지·중립 상태일 때 `1` / `2`로 외형을 고릅니다. 기존 Chaos 차량 물리와 입력을 유지합니다. 수중 부력·상하 자유 이동은 구현하지 않았습니다.
- **속도 설정:** 가상 차량 수동 전진 5km/h·후진 3km/h 제한. 제한 속도 부근에서 입력을 줄이고 초과 시 제동합니다. 실제 주행 최고속도는 Play 확인 전이며 RC카 출력 제한값도 아닙니다.
- **고래:** Connlan_Immure의 Humpback Whale (Swimming), 길이 약 11m, 원본 9.125초 애니메이션 반복. 관찰 구역 위 반경 8m 원을 55초마다 한 바퀴 이동하는 C++를 연결했습니다. 고래는 관찰 대상입니다.
- **환경:** rkuhlf 해초 3종 합계 871개·Poly Haven 바위 177개를 HISM으로 배치하고 실제 Vefects Niagara 기포 24곳을 추가했습니다. 기존 산호·해초·바위와 해파리 3마리도 유지합니다. 경로·출발/관찰 패드 주변은 비워 둡니다. 저장 맵은 총 374개 액터입니다.
- **자동복귀:** 전진 유턴으로 방향을 바꾼 뒤 기록한 위치를 역순으로 전진 추종합니다. 좌우 회전 공간을 검사하며 양쪽 모두 막히면 정지합니다. 사용자 중단·수동 전환·도착 정지와 자동 입력 MQTT 차단을 유지했습니다. 실제 Chaos 물리로 유턴·복귀하는 통합 시험은 남아 있습니다.
- **다른 PC 재현:** 저장된 `.uasset/.umap`를 사용하면 재임포트할 필요가 없습니다. `unreal/tools/prepare_selected_assets.py`는 Blender 변환, `unreal/tools/build_selected_underwater.py`는 UE 임포트·차량 설정·반복 배치, `WhaleSwimmingActor`는 원형 유영을 담당합니다.

**직접 검증:** UHT 포함 UE 5.7.4 Development Editor C++ 빌드 성공. Unreal 자동화 테스트 5개(입력·위험 구역·복귀 경로 계산·고래 발견·유영 에셋) 성공, 실패/경고 0개. Pi 코드의 모터·네트워크 없는 Python 단위 테스트 13개 통과. 맵 재개방 후 태그 유일성, 배치 개수, 고래 재질·뼈대·애니메이션 저장을 확인했습니다. 두 탐사선·고래·전체 환경을 렌더했고 맵 검사는 오류·경고 0개였습니다. 창 활성화가 두 차례 `failed to activate captured window`로 실패해 이번 Play 화면·키보드 주행 검증은 하지 못했습니다.

**현재 실행 제한:** 사용자가 “실제 RC카를 주행하면 안돼”라고 다시 명시했습니다. Pi 접속·차량 프로그램 실행·모터 제어 송신을 하지 않습니다. `bPublishVehicleInput=False`, `bReceiveVehicleCommands=False`를 유지하며, 두 설정이 꺼져 있으면 브릿지가 브로커에 접속하지 않도록 수정했습니다. 이번 작업에서 실차는 주행하지 않았습니다.

**남은 순서:** 통신을 끈 Unreal Play에서 외형 선택·5/3km/h 실제 속도·카메라/HUD·발견/위험 이벤트·전진 유턴/복귀/정지·프레임 성능 확인 → 차량과 완전히 분리한 loopback MQTT 통합 검증 → 사용자가 실차 작업을 명시적으로 재개한 이후에만 Pi 배포·실차 검증. 단위 테스트와 정적 렌더를 실차 또는 통합 주행 성공으로 해석하지 않습니다.

## 과거 단계별 작업 기록

아래의 미실행·정적 고래·이전 노란 탐사차·후진 복귀 설명은 당시 기록입니다. 현재 상태는 위 최신 절과 [CURRENT_STATE](docs/CURRENT_STATE.md), [AI_HANDOFF](docs/AI_HANDOFF.md)를 기준으로 합니다.

### 2026-10-16 탐사차 외형 · 조명 · 관찰 동선 정리

해저 맵에 **노란 선체·관찰창·카메라 장비를 갖춘 탐사차**를 지정했습니다. 사용자가 `unreal/tools/build_exploration_rover.py`와 새 Blueprint 2개·재질 2개의 업로드를 승인했습니다. **에디터 제작·저장·정적 외형 확인 단계이며 실제 주행 검증은 아닙니다.**

- `/Game/Underwater/Vehicles/BP_ExplorationRover`는 기존 스포츠카 Blueprint의 복제본입니다. 차체 외형과 카메라 위치·전조등 방향을 수정하고 장식 부품 15개를 추가했습니다. 바퀴·스켈레톤·물리·엔진·변속기·조향·입력 그래프를 유지하며 원본 차량과 게임모드 에셋은 보존했습니다. 새 외형 부품은 NoCollision입니다.
- `BP_UnderwaterGameMode`가 탐사차를 기본 Pawn으로 지정합니다. 전방 카메라 기준점은 차체 로컬 `(205,0,150)`cm, 후방 SpringArm 기준점은 `(0,0,100)`cm입니다. 관찰창은 불투명한 장식 재질이며 전방 카메라는 창 바깥에 있습니다. 부력·수중 이동 물리는 추가하지 않았습니다.
- 주 조명에 우선순위 1을 지정해 여러 DirectionalLight의 경고를 해소했습니다. 청록색 경로 표지 6쌍은 이동 방향에 수직으로 배치하고, 바닥 화살표 10개와 출발·관찰 안내판을 추가했습니다. 관찰 패드 표지는 청록색, 위험 구역은 주황색입니다. 출발·관찰·고래·위험 구역 좌표와 판정 태그는 유지했습니다.
- 기존 장면 스크립트의 `--polish-only --current-editor`로 열린 해저 맵의 해당 제작 액터만 갱신할 수 있습니다. 전체 재생성도 탐사차와 새 표지를 반영합니다. 차량 제작 스크립트는 완료된 에셋을 기본 재사용하며, 외형을 다시 적용할 때만 `--refresh-appearance`를 사용합니다.

**직접 확인:** 맵 322개 액터·해저 에셋 47개, 새 GameMode/Pawn 연결, 원본과 같은 엔진·변속기·조향·구동·바퀴 설정을 확인했습니다. 탐사차·동선·고래 시점 이미지를 Play 없이 렌더했으며 에디터 맵 검사는 오류·경고 0개입니다. Python AST 문법 분석도 통과했습니다. 경로 중심과 바위 경계상자 사이 간격은 표본 점검에서 최소 약 4.18m였으나 실제 충돌·회전 여유·카메라 시야·성능은 주행으로 확인해야 합니다. C++ 빌드·Play·자동화 테스트·Pi 실행·MQTT 송신은 하지 않았고 `bPublishVehicleInput=False`를 유지합니다.

### 2026-10-16 위험 구역 배치 · 복귀 이벤트 연결 · 응급 경보 경로 정리

해저 맵에 **지정 위험 구역 1개와 주황색 경계·경고 표지**를 추가했습니다. 차량이 구역에 들어오면 이유 안내와 5초 정지 준비 후 기존 기록 경로로 복귀하도록 C++를 연결했습니다. 새 판정 소스 2개와 테스트 1개는 사용자가 업로드를 승인했습니다. **맵 제작·저장은 확인했으며 새 C++는 빌드·Play·테스트 전입니다.**

- `DangerZone_Reef`는 중심 `(-600,1800,300)`cm, 전체 크기 14×10×6m의 고정 TriggerBox입니다. `ExplorationDangerZone` 태그와 차량 원점으로 판정하며 충돌·Overlap에 의존하지 않습니다. 출발점·청록색 안내 경로·고래 관찰 패드와 분리했습니다. 현재는 지정 구역이며 위험 생물 모델·행동·실물 센서 감지는 구현하지 않았습니다.
- 구역별 진입은 한 번 처리하고 경계에서 1m를 벗어난 뒤 재진입해야 새 이벤트가 발생합니다. 연속 표본 사이 짧은 통과도 판정합니다. X/Space 정지·M/새 주행 입력 수동 전환 후 같은 구역 안에서 복귀를 반복 시작하지 않습니다. 정지 상태에는 이벤트가 재출발을 명령하지 않습니다.
- 위험 구역과 고래 발견이 겹치면 위험 안내를 우선합니다. 이미 복귀 중이면 안내만 갱신하고 대기 시간·경로 진행을 초기화하지 않습니다. 복귀할 기록이나 제어 조건이 없으면 경고와 함께 정지합니다. UI에 구역 내 주의 표시를 추가했습니다.
- 기준 MQTT 브릿지에서 응급 경보 구독·파싱·이벤트 발행과 `PublishHomeTrigger()`의 송신을 제거했습니다. 기존 Blueprint가 참조하는 이름은 호환용으로 남겼으며 `bEnableHomeEvents=True`도 무시합니다. Rescue/VehicleBasic 그래프·옛 웹/Pi 자료는 보존하지만 현재 탐사 실행 경로에서 사용하지 않습니다. 차량 통신 단절 정지와 수동 MQTT 출력 규약은 유지합니다.
- 제작 스크립트에 `--update-danger-only`와 `--current-editor`를 추가했습니다. 위험 구역만 갱신하거나 열린 맵에 적용할 수 있고 Play 중 실행은 거부합니다. 새 머티리얼·메시는 만들지 않고 기존 에셋을 사용했습니다.

**확인 결과:** 열린 UE 5.7 에디터에서 배치·저장·표지 시야를 확인했습니다. 기존 288개 액터의 위치·태그와 존재는 유지되며 현재 총 298개입니다. 맵 검사는 오류·경고 0개입니다. 별도로 뷰포트에 기존 여러 DirectionalLight의 우선순위 경고가 표시되어 다음 장면 정리 항목에 남겼습니다. 설치된 UE API 대조·코드 검토·Python AST 문법 분석을 진행했지만 빌드·Play·자동화 테스트·Pi 실행·MQTT 차량 송신은 하지 않았습니다. 송신 설정은 `bPublishVehicleInput=False`입니다.

### 2026-10-16 주행 경로 기록 · 발견 후 Unreal 자동복귀 코드 추가

고래 관찰 다음 단계로 **실제 주행 좌표 기록 → 발견 후 5초 안내·정지 → 역순 경로 추종 → 출발점 정지**를 C++로 연결했습니다. 새 경로 계산·복귀 컴포넌트·테스트 소스 5개는 업로드 승인을 받았습니다. **빌드·Play·자동화 테스트·실물 주행은 실행하지 않았으며 동작 검증 완료를 뜻하지 않습니다.**

- 출발점 태그 근처에서 기록을 시작합니다. 실제 차량 위치와 방향을 저장하며 제작용 `RouteGuide`는 사용하지 않습니다. 좌표가 크게 뛰거나 기록 한도를 넘으면 경로를 무효화합니다. 차량 재생성·자세 복구 시에도 이전 경로를 이어 붙이지 않습니다.
- 최초 고래 발견 또는 `H` 키 요청으로 복귀를 준비합니다. 5초 안내와 차량 정지 확인 후 저속 Chaos 차량 입력으로 기록 지점을 역순 추종합니다. 전진으로 탐사한 구간은 주로 후진으로 되짚으며, 차량을 좌표로 순간 이동시키지 않습니다. 카메라는 계속 조작할 수 있습니다.
- `X`는 정지, `M`은 수동 전환입니다. 복귀 중 `Space`도 중단·정지하며, 주행 조작을 모두 놓은 뒤 다시 입력하면 수동 전환합니다. 발견 순간 이미 누르고 있던 주행 키는 즉시 복귀를 취소하지 않습니다. 중단 후 자동 재출발은 하지 않으며 `H`로 재요청할 수 있습니다.
- 출발 기록점 1m·출발 앵커 2m 이내에서 속도 10cm/s 미만이 0.5초 유지되어야 도착으로 표시합니다. 경로 이탈·진행 정체·포커스 상실·일시정지·차량 교체 등은 복귀 중단으로 처리합니다. 송신 모드가 켜져 있을 때 브로커 단절도 중단 조건입니다.
- Unreal 자동복귀 중 MQTT용 `GetVehicleControlInput()`은 항상 0을 반환합니다. 실물의 정밀 복귀는 구현 범위로 확정되지 않았으므로 자동복귀 입력을 RC카에 보내지 않습니다. 수동 입력 송신 규약은 유지하며 `bPublishVehicleInput=False`입니다.
- 기존 관찰 UI에 복귀 이유·대기 시간·남은 경로 거리·도착/중단 안내를 통합했습니다. `ExplorationReturnTests.cpp`를 추가하고 기존 차량 입력 테스트에 자동 출력 차단·수동 전환·정지 후 입력 해제 검사를 보강했습니다.

**검증:** 설치 UE 5.7의 Chaos 기어·입력 API와 입력 바인딩·컴포넌트 tick API를 읽어 대조하고 코드·파일 변경을 정적으로 검토했습니다. 경로 추종 수치(차축 거리·조향각·속도·경계)는 초기값이며, 실제 곡선·후진/전진 전환·경사·충돌·도착 제동은 미검증입니다. 장애물 회피·임의 경로 탐색·위험 구역·실물 위치 복귀는 이번 구현에 포함되지 않습니다. 기존 DLL에는 이 코드가 없으므로 실행 재개 후 UHT 포함 빌드가 필요합니다.

### 2026-10-16 고래 발견 판정 · 관찰 안내 UI 코드 추가

해저 맵 다음 단계로 관찰 구역과 안내 UI를 연결했습니다. 새 관찰 컴포넌트·위젯·테스트 소스 5개는 사용자가 업로드를 승인했습니다. **소스 구현 단계이며 빌드·Play·테스트는 실행하지 않았습니다.** 기존 DLL에는 이 기능이 없으므로 실행 재개 후 새로 빌드해야 표시됩니다.

- 차량이 관찰 패드 중심에서 수평 6m 이내, 높이 차이 3m 이내에 들어오면 최초 발견을 알립니다. 퇴장은 수평 7.5m 초과 또는 높이 범위 이탈이며 경계 근처 반복 진입을 줄입니다. 카메라 시선이나 영상 인식이 아닌 **관찰 구역 진입 판정**입니다.
- 화면 왼쪽 위에 관찰 패드까지의 수평 거리, 최초 5초 발견 알림, 관찰·재방문 안내와 카메라 조작법을 표시하도록 작성했습니다. 기존 속도·기어 UI를 유지하고 위젯은 조작 입력을 가로채지 않습니다. 이후 추가한 복귀 컴포넌트가 발견 후 정지·복귀를 담당하며 해당 상태에서는 복귀 안내를 우선 표시합니다.
- 발견 상태는 PlayerController에 보관해 차량 재생성·재진입 후 중복 발견을 막습니다. 새 플레이 세션에서 초기화되며 파일에 저장하지 않습니다. `OnObservationZoneChanged`와 최초 1회의 `OnWhaleDiscovered`를 분리했습니다. 이후 위 복귀 컴포넌트를 최초 발견 이벤트에 연결했습니다.
- 기존 맵의 `WhaleObservationAnchor`·`WhaleObservationSubject` 태그를 각각 하나씩 찾습니다. 대상·차량이 없거나 태그가 중복되면 안내를 숨깁니다. `.umap`·기존 Blueprint·MQTT·Pi 코드는 이번에 변경하지 않았습니다.
- 새 `Tests/WhaleObservationTests.cpp`는 진입/퇴장 경계, 상하 범위, 재진입·대상 유실과 비유한 값 처리를 월드·모터·네트워크 없이 검사하도록 작성했습니다. **테스트 통과를 확인한 상태는 아닙니다.**

**직접 확인:** 설치된 UE 5.7의 UMG·델리게이트 API와 대조하고, 기존 에디터 점검 기록의 관찰 패드 반경 600cm와 두 태그를 확인했습니다. 런타임 발견 이벤트·화면 배치·한글 표시·입력 유지·Pawn 재생성은 실행 재개 후 검증해야 합니다. 경로 기록·자동복귀는 이후 위 소스 작업으로 추가했으며 위험 구역은 후속 작업입니다. MQTT 송신은 계속 `bPublishVehicleInput=False`입니다.

### 2026-10-15 해저 탐사 맵 제작

사용자가 **에디터 제작 작업**을 승인해 `/Game/Underwater/Maps/L_UnderwaterExploration`을 새로 만들었습니다. `unreal/tools/build_underwater_scene.py`와 `unreal/MqttVehicle/Content/Underwater/`의 생성 에셋 업로드도 승인했습니다. Play·테스트·C++ 빌드·Pi 프로그램 실행과 MQTT 차량 송신은 계속 보류합니다.

- 140×100m의 충돌 바닥, 외곽 암벽, 출발 패드·PlayerStart, 경로 표지 6쌍, 관찰 구역을 배치했습니다. 기존 `BP_VehicleAdvGameMode` / `BP_SportsCar_Pawn`을 사용합니다. 차량 외형은 아직 템플릿 스포츠카입니다.
- 보유 Underwater World 원본에서 바위 4종·산호 3종·해초 1종과 대응 텍스처를 임포트했습니다. 색상·노멀·해초 마스크를 연결했고, 채널 구성을 확인하지 않은 AO_R_MT 텍스처는 연결하지 않았습니다.
- 직접 제작한 약 11m 고래를 정적 관찰 대상으로 배치했습니다. 유영 애니메이션·조종은 없습니다. 당시 없던 발견 이벤트는 10월 16일 C++로 추가했으며 실행 검증 전입니다.
- 출발점·관찰점·경로 표지는 향후 기능 연결용 액터입니다. 현재 자동복귀·위험 구역·발견 판정은 실행하지 않습니다. 기존 차량 트랙과 Rescue 맵은 보존합니다.
- 에디터 시작 맵과 게임 기본 맵을 새 해저 맵으로 지정했습니다. `.uproject`를 열면 이 장면을 작업할 수 있습니다.
- 제작 스크립트는 기존 맵을 기본적으로 덮어쓰지 않습니다. `--refresh-generated`를 명시하면 제작 태그가 붙은 액터만 재배치하며, 손으로 추가한 액터와 기존 임포트 에셋은 보존합니다. 태그가 붙은 액터를 수동 수정했다면 재배치 전에 태그를 제거하거나 맵을 복제하세요.

**검증:** UE 5.7.4 에디터에서 맵 저장·다시 열기, 차량 게임모드/Pawn 연결, 288개 액터 배치, 바닥의 박스 충돌·바위의 볼록 충돌 도형과 머티리얼 연결을 확인했습니다. 에디터 맵 검사에서 오류·경고가 없었고, Play 없이 전체 배치와 관찰 시점 이미지를 렌더해 회전·누락 머티리얼·조명을 조정했습니다. 새 UE 에셋은 43개, 약 90.1MiB입니다. **장면 초안**이며 실제 주행·카메라·충돌 물리·성능은 Play로 검증하지 않았습니다. 최신 카메라 C++와 자동화 테스트를 포함한 빌드도 여전히 미실행입니다.

### 2026-10-15 관찰 카메라 정리 · 실제 연동 검증 보류

사용자가 실제 테스트를 아직 할 수 없어 해당 단계를 건너뛰도록 요청했습니다. 실물·MQTT 검증은 미완료로 남기고 관찰 카메라 C++ 작업을 진행했습니다. 기존 실행 보류와 `bPublishVehicleInput=False`는 유지합니다.

- 매 프레임 시야를 정면·수평으로 되돌리던 코드를 변경해 전방·후방 카메라가 각자 둘러본 방향을 유지하도록 했습니다. 현재 활성 카메라만 조작합니다.
- 마우스 Y와 게임패드 오른쪽 스틱 Y로 상하 관찰, `R` 또는 오른쪽 스틱 클릭으로 현재 카메라의 시작 방향 복귀를 추가했습니다. 기본 상하 범위는 아래 45°~위 70°이며 카메라 각도·감도·상하 반전은 Pawn의 `Camera|Observation`에서 조절합니다.
- `Tab` 전환은 입력을 누른 순간 한 번 처리합니다. 차량 자세 복구용 Backspace와 카메라 정면 복귀는 별개입니다. 카메라 조작은 차량 주행 입력이나 MQTT 조작값을 변경하지 않습니다.
- 자동 정면 복귀는 기본 꺼짐입니다. 선택적으로 켜면 마지막 시점 조작 후 기본 2초 대기한 뒤 복귀합니다. 후방 카메라 충돌 검사는 기본 켜짐으로 적용합니다.
- 새 입력은 실행 시 임시 Enhanced Input 매핑으로 등록하고 차량 소유 해제·종료 시 제거합니다. 기존 `.uasset`와 맵은 수정하지 않았으며 새 보존 파일도 만들지 않았습니다.

**검증:** 설치된 UE 5.7 헤더와 소스를 대조하고 변경 내용을 정적으로 검토했습니다. 이번 카메라 변경은 빌드·실행하지 않았습니다. 상하 방향·감도·충돌·전환·재소유 시 매핑 정리는 실행 재개 후 확인해야 합니다. 이후 해저 맵·고래 장면을 제작했으며 위 제작 기록을 참고합니다.

### 2026-10-15 후속 작업: Pi 수신 코드 보강

저장소의 `raspberry-pi/rover/rover_drive.py`를 수정했습니다. Pi의 `~/rover_drive.py`는 배포하지 않아 이전 버전이며, 두 파일의 해시는 이제 다릅니다.

- JSON 객체의 `th`, `st`에 유한한 숫자만 허용하고 범위 초과는 -1~1로 제한합니다. 누락·문자열·boolean·NaN/Infinity·잘못된 JSON·과대 payload와 브로커가 재전달하는 retained 명령은 입력을 지우고 다음 제어 루프에서 정지하도록 변경했습니다.
- 구독 승인 전에는 입력을 받지 않으며 연결 단절·재접속 시 이전 명령을 지웁니다. 마지막 정상 명령으로부터 0.3초가 지나면 다음 50ms 제어 루프에서 0을 적용합니다. 실제 시간은 OS 스케줄링 영향을 받으며 실차의 완전 정지 시간을 뜻하지 않습니다.
- MQTT 콜백은 상태만 저장합니다. 모터 출력과 SIGINT/SIGTERM·예외 종료 정리는 메인 루프가 담당하고, 한 모터의 정리 실패가 나머지 모터의 정리를 건너뛰지 않도록 했습니다.
- 기본 모드는 GPIO를 불러오지 않는 `--dry-run`입니다. 명령은 `veh/cmd`, 시험 상태는 `veh/state/dry-run`입니다. 실제 모터 제어에는 `--motors`가 필요하며 상태 토픽은 `veh/state`입니다. 수신 프로그램은 차량 명령을 발행하지 않습니다.
- 기존 좌우 믹싱식과 최대 출력 0.6을 유지했습니다. `--max-speed`로 상한을 낮출 수 있습니다. 실제 회전 방향, 후진 조향과 저속 회전 특성은 아직 검증하지 않았습니다.
- 새 `raspberry-pi/rover/tests/test_rover_drive.py`는 입력 오류·워치독·재접속·종료를 가짜 출력으로 검사하며, 사용자가 업로드를 승인했습니다.

**검증:** 두 Python 파일의 AST 문법 분석과 코드 검토만 수행했습니다. 파일을 import하거나 테스트·수신 프로그램을 실행하지 않았습니다. Unreal 빌드·실행, MQTT 접속·송신, Pi 배포·모터 실행도 이번 후속 작업에서 하지 않았습니다. 실행 보류와 Unreal 송신 비활성화는 유지합니다. 실행 재개 후 사용할 명령과 확인 항목은 `docs/AI_HANDOFF.md` 5절에 있습니다.

### 2026-10-15 차량 입력·MQTT 코드와 Pi 확인

- `MqttVehiclePawn`에 공통 입력 상태와 `GetVehicleControlInput()` / `StopVehicleInput()`을 추가했습니다. 전진·후진 전환 중 제동, 양쪽 페달 동시 입력, 핸드브레이크, 입력 취소·자세 복구·EndPlay 정지를 처리합니다. 정지 출력은 `th=0, st=0`입니다.
- 브릿지에 약 100ms 주기의 `veh/cmd {"th":…, "st":…}` 송신(QoS 0, retain false), 연결·구독 결과 검사, 재접속, 포커스 상실·일시정지·연결 상실·종료 시 정지 처리를 추가했습니다. 기존 명령 수신과 응급 이벤트는 기본 비활성화했고 Blueprint 참조는 보존했습니다. 실제 네트워크 동작은 후속 검증 대상입니다.
- 입력 에셋에서 W 전진, S 제동/후진, A/D 조향, Space 핸드브레이크, Tab 카메라 전환, Backspace 자세 복구를 확인했습니다. 카메라 구현과 맵은 이번에 변경하지 않았습니다.
- Pi `192.168.0.8`에 SSH로 접속해 Mosquitto 실행과 MQTT 접속을 확인했습니다. 차량 제어 프로그램은 실행 중이 아니었습니다. 당시 Pi의 `~/rover_drive.py`는 수정 전 저장소 사본과 SHA-256이 같았으며 `veh/cmd`를 사용합니다. 이후 저장소 수신 코드만 보강했으므로 현재 Pi와는 다릅니다. 실제 모터 모듈은 전시회 사본과 핀·극성이 다릅니다.
- `DefaultGame.ini`의 브로커 주소를 `192.168.0.8`로 변경하고 `bPublishVehicleInput=False`를 유지했습니다. Pi 파일·서비스·모터는 변경하거나 실행하지 않았습니다.
- 새 자동화 테스트 `unreal/MqttVehicle/Source/MqttVehicle/Tests/MqttVehicleControlTests.cpp`의 업로드는 사용자가 승인했습니다. **사용자의 “실행을 하지마” 요청에 따라 추가 빌드·Unreal 실행·자동화 테스트·Pi 차량 실행을 보류합니다.** 이후 다음 단계 요청에 따라 수신 코드 수정·문서화·업로드를 진행하며 실행 보류는 유지합니다.

검증 구분: 실행 중단 요청 이전에 입력·브릿지 C++의 UE 5.7.4 Development Editor 빌드는 성공했습니다. 에디터 월드의 임시 검사에서 전진·입력 해제·동시 페달·핸드브레이크를 확인했으나, 물리 차량이 없는 에디터 월드에서는 후진 기어를 설정할 수 없어 후진 검사가 실패했습니다. 이를 게임 물리 월드에서 확인할 새 자동화 테스트를 작성했으며 **새 테스트를 포함한 최종 빌드와 테스트 실행은 하지 않았습니다.** Unreal↔MQTT↔Pi 연속 송신·재접속·실물 주행도 미검증입니다.

### 이전 에디터 점검 (2026-10-11)

2026-10-11 에디터 점검 결과를 2026-10-14 문서에 반영했습니다. 점검 중 프로젝트 기능은 수정하지 않았습니다.

| 항목 | 확인 결과 |
| --- | --- |
| 시작 맵 / Pawn | `VehicleBasic` 차량 트랙 / `BP_SportsCar_Pawn` |
| 해저 장면 | 사용자가 아직 없다고 확인. 해저 맵과 고래 관찰 장면은 새로 구성할 대상 |
| 차량 Blueprint | 이벤트 그래프 32개 노드에서 조작·카메라·조명 연결 확인. 도착·자동복귀·MQTT 발행 연결은 발견하지 못함 |
| `VehicleBasic` 레벨 Blueprint | `OnHomeAlert` 수신·문자열 출력 구성. 자동 이동 Timeline 없음 |
| `Lv_RescueMap` 레벨 Blueprint | 경보 레벨에 따른 `DispatchTimeline` 재생·역재생으로 차량 모형 이동·복귀, `FallTimeline`으로 사람 모델 회전. `F`/`C` 키로 낙상/해제 경보 발행 |
| 당시 MQTT 연결 | 연결 실패 코드 `8` 뒤 구독 완료처럼 기록하는 문제 확인. 10월 15일 반환 코드 검사·재접속 코드를 추가했으며 실제 재접속은 미검증 |
| 실행 검증 범위 | Play에서 차량 생성·후방 카메라·속도 UI 확인. 수동 주행·카메라 전환 정상 동작과 실제 RC카 연동은 아직 검증 미완료 |

## 다른 PC / 다른 AI의 시작점

1. [docs/AI_HANDOFF.md](docs/AI_HANDOFF.md)를 먼저 읽으세요. 최신 요구사항, 설치·실행 절차, 다음 작업과 검증 기준이 있습니다.
2. [docs/CURRENT_STATE.md](docs/CURRENT_STATE.md)에서 코드로 확인한 구현 상태와 문서 충돌을 확인하세요.
3. [docs/FILE_GUIDE.md](docs/FILE_GUIDE.md)에서 업로드한 **각 파일**의 경로·역할·크기를 확인하세요.
4. 실제 작업 대상은 **`unreal/MqttVehicle/MqttVehicle.uproject`**입니다. `archive/MqttVehicle_old/`는 보존한 인계 사본입니다.

## 내려받기

이 저장소는 사용자의 승인으로 비공개로 전환했습니다. 저장소 접근 권한이 있는 계정으로 Git 인증을 먼저 완료하세요. Git LFS가 필요합니다.

```powershell
git lfs install
git clone https://github.com/HE4TH/digital_twin.git
Set-Location digital_twin
git lfs pull
git lfs fsck
```

ZIP 다운로드 대신 Git clone + Git LFS를 사용하세요. Unreal 에셋과 원본 파일이 LFS 포인터 텍스트로만 남으면 프로젝트를 열 수 없습니다.

## 폴더

| 위치 | 역할 |
| --- | --- |
| `unreal/MqttVehicle/` | 기준 Unreal 프로젝트. Source, Config, Content, Blender 원본 ZIP |
| `archive/MqttVehicle_old/` | 이전 인계 프로젝트 사본. 비교·복구 참고용 |
| `archive/exhibit_v1/` | 기존 전시회 RC카 수신 코드, 모터 모듈, 웹 대시보드와 systemd 서비스 |
| `raspberry-pi/rover/` | 현재 Pi의 `veh/cmd` 코드를 바탕으로 보강한 수신기·오프라인 테스트와 기존 웹·브로커 자료. Pi 배포·실행 전 |
| `archive/unreal_snippets/` | 출동 지령 기능이 있는 별도 C++ 후보. 기준 프로젝트에 적용된 코드가 아님 |
| `archive/proj_check/` | 과거 프로젝트 점검용 소스·설정 사본 |
| `archive/web-controller/` | 구형 `testvi/*` 웹 실습. 실물 제어 기준으로 사용하지 않음 |
| `assets/whale_original/` | 직접 제작한 고래 원본과 생성·렌더 스크립트 |
| `assets/selected_free_assets/` | 사용자 선택 탐사선·해초·바위·혹등고래 원본과 Blender 변환본 |
| `unreal/MqttVehicle/Content/Underwater/Selected/`, `Content/Vefects/Bubbles_VFX/` | 현재 장면의 선택 모델·재질·애니메이션과 Fab 기포 패키지 |
| `materials/fab_underwater_world_portable/`, `materials/corals_fbx.zip` | 프로젝트 협업용 해저 메시·텍스처 원본 |
| `materials/수업자료/`, `archive/references/` | 강의 자료와 MQTT 참고 예제 |
| `output/`, 루트 PPTX·HWP | 작품 설명 판넬·기획 자료 |
| `unreal/tools/` | 해저 맵·탐사선·선택 에셋 변환/배치와 파일 설명·SHA-256 목록 도구 |

## 보존 범위

두 Unreal 사본, 프로젝트 콘텐츠, 원본 모델·압축 자료, 코드, 기존 문서와 발표 자료를 보존합니다. `Binaries`, `Intermediate`, `Saved`, `DerivedDataCache`, `.vs`, 생성된 솔루션, 판넬 제작 임시 산출물은 제외합니다. 다른 PC에서 이 파일들은 재생성합니다. 실행 로그에는 세션 인증정보가 포함될 수 있어 업로드하지 않습니다.

공유 사본의 `DefaultEngine.ini`에 들어 있던 Android File Server `SecurityToken` 값은 비웠습니다. 기존 로컬 Unreal 프로젝트 파일은 변경하지 않았으며, 필요한 경우 각 PC에서 별도 설정합니다.

## 현재 다음 작업

1. 최신 문서·소스와 학교 노트북 환경을 확인하고 기존 맵을 복원한다. 최신 기존 검증 기록은 Unreal 자동화 12개·Pi 오프라인 13개이며 VR 통과 기록은 아니다.
2. Lanzaman의 OpenXR HMD 카메라·Quest 입력을 기존 키보드/수중 이동/3D A*와 함께 구현한다. Air Link 페어링·VR Preview는 학교 장비에서 검증한다.
3. 레거시 대시보드를 전시 UI·Unreal VR 미러 화면 공유·veh/cmd/veh/state 표시로 개편한다.
4. 수중 수동 입력 변환 후 127.0.0.1 브로커와 dry-run 수신기로 MQTT 통합 검증한다. Pi·실제 모터 제어는 별도 승인 전까지 금지한다.
5. 시작/종료 절차와 장시간 성능을 확인하고, 승인 후에만 고정 RC카의 바퀴 회전 시연을 별도 검증한다. 바닥 주행은 하지 않는다.

초기 업로드와 10월 11일 점검은 인계·읽기 작업이었고, 10월 15일에는 위 차량·MQTT 소스를 수정했습니다. Pi 배포·실물 주행은 수행하지 않았습니다. 전복 자세 복구와 통신 단절 정지는 목적지 자동복귀와 구분해 유지합니다.

## 2026-10-14 요구사항 갱신 (과거 결정 기록)

이 절은 2026-10-11 인계 이후 사용자와 합의한 변경사항이다. 기능 구현 완료 보고가 아니며, 기존 코드 상태는 `docs/CURRENT_STATE.md`를 참고한다.

아래의 기록 경로 역추종·토픽 미확정·정적 고래 설명은 당시 기준이다. 현재 수중 복귀는 3D A*, 명령 토픽은 veh/cmd이며 장면에는 유영 고래가 있다. 최신 목표·실행 제한·검증 상태는 이 문서 상단의 2026-10-19 인수인계와 10월 18일 구현 기록을 따른다.

| 이전 기준 | 최신 기준 |
| --- | --- |
| 자동복귀·도착 처리 전체 제거 | 기존 응급상황 의존은 제거하되, 해저 탐사 이벤트 기반 자동복귀와 출발점 도착 처리는 새로 설계한다. 재사용 가능한 기능은 조사 후 판단한다. |
| 수동 주행·고래 관찰 중심 | 수동 탐사 → 목표물 발견 또는 위험 생물 구역 진입 → 복귀 이유 안내 → 기록한 경로를 역순 추종 → 출발점 정지 흐름을 추가한다. |
| 음성 명령으로 자율주행차 제어라는 과거 기획 표현 | 현재 조작 기준은 가상 탐사차 수동 입력이다. 음성 제어는 현재 구현 범위로 확정되지 않았다. |
| 자율주행이라는 포괄적 표현 | 우선 Unreal 내 이벤트 기반 경로 추종 자동복귀를 구현한다. 임의 목적지 탐색·장애물 회피 전체를 구현한다고 주장하지 않는다. |

고래는 계속 관찰 대상이며 조종하지 않는다. 수동 탐사 중 이동 지점을 기록하고 복귀 시 차량을 조향·가속·제동해 기록 경로를 따라가도록 설계한다. 단순 위치 순간 이동과 구분한다. 자동복귀 중 사용자 정지·수동 전환을 제공하며, 통신 단절 정지 기능은 유지한다.

실제 RC카의 정확한 경로 복귀는 미확정이다. Unreal 복귀 조작값의 MQTT 전송과 실제 차량 위치 일치는 별개이며, Pi 실행 코드·위치 센서·제어 방식·주행 오차를 확인한 후 범위를 정한다. 단순 명령 역재생만으로 실물의 정확한 복귀를 보장하지 않는다. MQTT 토픽도 실제 Pi 수신 프로그램 확인 후 한 계열로 통일한다.

작품 설명 문구는 **AI·IoT 융합 디지털트윈 시스템**을 유지한다. 생성형 AI는 실제 활용한 에셋·도구·제작 단계를 확인해 설명하고, 제공된 정적 고래 모델을 AI 제작 또는 유영 구현 완료로 표시하지 않는다.

### 작업 환경과 GitHub 연결

- 데스크톱의 Unreal 5.7 정상 실행은 사용자가 확인했다. 노트북에서는 Chrome 원격 데스크톱을 통해 데스크톱의 Unreal·Codex를 사용한다.
- 팀 워크스페이스에서 Codex 원격 제어 활성화 오류가 발생했다. 관리자 제한인지 여부는 확정되지 않았다. 화면 원격접속은 두 PC의 로컬 Codex 대화나 프로젝트 파일 자동 동기화를 의미하지 않는다.
- 2026-10-14 노트북 작업에서 저장소를 별도로 복제했다: `D:/STUDENT_HANDOFF_MQTT_2026-09-27/github/digital_twin`. Git 원격은 `https://github.com/HE4TH/digital_twin.git`이다. 이 경로는 노트북 전용이며 데스크톱 작업 경로와 혼동하지 않는다.
- 기존 바깥 폴더의 인계 사본은 덮어쓰지 않았다. 이번 변경은 문서화이며 UE 코드 수정·빌드·Pi 배포·실물 주행 검증을 수행하지 않았다.

### 앞으로의 작업 규칙

1. 이 저장소에서 작업하면 별도 요청이 없어도 README를 실제 변경 내용에 맞춰 갱신한다. 영향받는 인계·상태 문서도 함께 정리한다.
2. 구현 완료, 계획, 사용자 확인, 직접 검증 결과를 구분한다. 실행하지 않은 빌드·실물 테스트를 완료로 쓰지 않는다.
3. 새 소스·문서·에셋 파일을 만들면 즉시 경로와 목적을 알리고 **“GitHub에 올려도 될까요?”**라고 묻는다. 임시 파일·빌드 캐시·로그는 업로드 대상에서 제외한다.
4. 새 파일 업로드 승인 전에는 push, GitHub 파일 생성 또는 PR 생성으로 게시하지 않는다. 로컬 수정과 검증은 계속할 수 있다.
5. 문서·코드의 로컬 수정과 GitHub 게시를 구분해 보고하며, 새 파일 게시 전에는 사용자 승인을 확인한다.
6. 2026-10-19 인수인계 기준으로 기존 파일 수정도 GitHub 업로드 전 변경 목록 확인을 받는다. README·인계·파일 목록 갱신은 계속 자동으로 수행한다.

## 에셋 공유

제3자 원본 에셋이 포함되어 있으므로 저장소를 비공개로 유지하고, 프로젝트 협업자에게만 접근을 부여하세요. 원본의 별도 공개 배포와 일괄 오픈소스 라이선스 적용은 하지 않습니다. 상세 출처와 기존 공유 조건은 [docs/에셋_사용범위.md](docs/에셋_사용범위.md), [보유_에셋_검토.md](보유_에셋_검토.md)를 참고하세요.
