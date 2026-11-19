# 로컬 해저 탐사 대시보드

## 2026-10-21 VR 알림을 대시보드에 표시 — 로컬 APK 1.0.3

- 사용자 선택에 따라 VR 위젯의 실제 문구·남은 표시 시간을 기존 Quest 상태 요청에 포함하고, 대시보드 영상 위에 HTML 자막으로 표시한다. 추가 SceneCapture나 영상 합성은 하지 않는다.
- 자동복귀 준비·시작·도착·중단, 고래 관찰, 해파리 주의 및 복귀 중 배지를 전달한다. 동일 알림의 반복 상태 갱신이 표시 시간을 재시작하지 않도록 남은 시간을 사용한다. Pi와 브라우저 양쪽에서 만료·연결 단절 시 자막을 지운다. 상태 전달 주기 때문에 VR과 웹 표시에는 약0.5~1초 차이가 날 수 있다.
- Windows/Android 컴파일·쿠킹·APK 패키징 성공. 서명·버전·ZIP 검사 통과. 서버 검사14개 및 JavaScript 구문 검사 통과. 별도 시험 영상에서 복수 자막 배치 확인.
- Pi 웹서비스에 배포 완료(기존 파일 .runtime/pre-captions 백업). Quest USB 업데이트 및 앱 실행 완료. 기존 RC 송신 꺼짐·양 출력0 확인. 실제 VR 알림과 웹 화면을 동시에 비교한 실기 확인은 미완료다. 사용자 선택은 현재 문구·시간 동기화 방식 유지이며 위젯 영상 합성은 적용하지 않는다.
- APK versionCode=4/versionName=1.0.3, SHA256 `3128d092bc97025fa7732fbe6a33104fe012ce9a390fb38de5c567157b012873`. 로컬 설치 ZIP `outputs/DMix-Quest3-1.0.3-Windows.zip`. 사용자가 현재 웹 자막 방식을 유지하고 이 변경의 커밋·푸시를 승인했다. 설치 ZIP은 GitHub Releases의 1.0.3으로 게시한다.


## 2026-10-21 Quest 1.0.2 영상 배포

- Pi 영상 서버에 H.264/fMP4 중계를 추가하고 서비스를 갱신했습니다. 기존 JPEG 송신도 지원합니다.
- Quest 하드웨어 영상 인코딩과 대시보드 재생 확인. 6초 측정 17.9~18.1fps로 목표30fps에는 미달합니다.
- 배포 파일: server.py, quest_video.py, quest-video.js, app.js, index.html. 서버 검사12개 통과. RC 송신 꺼짐·양 출력0 유지 확인.


## 2026-10-21 Pi 대시보드 버튼·수신 서비스 배포 완료

- 사용자 요청으로 Pi 10.95.86.138(zerotoai)에 원격 main 84917a4의 server.py, app.js, index.html, test_server.py, dmix-rc-session.service, setup_rc_service.sh를 적용했다. 기존 파일/기존 사용자 서비스는 /home/pi/dmix-dashboard/.runtime/pre-controls-20261021에 백업했다. 아래의 Pi 배포 보류 기록은 이번 배포로 대체된다.
- Pi의 임시 배포 폴더에서 HTTP/상태 단위 검사 10개, Node JavaScript 구문 검사, 셸 구문 검사 통과. 설치 스크립트로 사용자 RC 서비스 등록·pi linger 설정·DMIX_RC_SERVICE=1을 적용하고 dmix-dashboard만 재시작했다. dmix-dashboard와 dmix-pixel active, 실제 /api/state의 broker=true 및 receiver_start_available=true, 페이지의 버튼 3개를 확인했다. 배포한 서버/화면 3개 파일의 SHA-256은 저장소와 일치했다.
- RC 수신기는 inactive/dead, 서비스는 static으로 부팅 자동 시작을 등록하지 않았다. 모터 제어 프로세스 없음 확인. 수신 시작/송신 켜기/자동복귀 버튼을 실제로 누르거나 제어 요청을 보내지 않았다. 서비스는 시작 후 비정상 종료 시 재시작하도록 정의돼 있다.
- /home/pi/rover_drive.py는 수정하지 않았고 전후 SHA-256 d69636f746cf62970436778f92288ad43da5c6a06c6b97658d39d89276eaeed7 일치를 확인했다. 기존 Pi 회전 출력 보정을 저장소 수신 코드로 덮어쓰지 않았다.
- 검증 당시 Quest connected=false, streaming=false, control=null, receiver=null이었다. 최신 APK 연결 후 실제 영상·송신 전환·복귀 알림·고정 바퀴 검증은 남아 있다. http://10.95.86.138:5050 을 새로고침해 사용한다. 2026-10-21 사용자가 이번 Pi 배포 결과 문서의 GitHub 게시를 승인했다.


## 2026-10-21 Quest 대시보드 버튼 3개

- 사용자 요청에 따라 RC 수신 시작, RC 명령 송신 켜기/끄기, 자동복귀 시작만 표시한다. 수동 전환·수신 중지·복귀 취소·별도 정지 버튼은 표시하지 않는다.
- QuestDashboardComponent가 /api/quest/control에 0.5초 간격으로 상태/적용 응답을 보내고 명령을 받는다. 명령 ID 중복 실행 방지, 2초 서버 대기 만료, 에디터/Quest 응답 분리를 적용한다. 복귀 가능 여부와 송신 게이트/Quest MQTT 연결을 표시한다. 구형 영상 전용 APK는 제어 버튼을 활성화하지 않는다.
- 새 APK는 세션 시작 시 RC 송신 게이트가 꺼진다. 사용자가 송신 켜기를 눌러야 주행 명령을 전달한다. 송신 끄기 또는 대시보드 서버와 3초 통신 단절 시 게이트를 닫고 MQTT 출력값을 0으로 보낸다. 연결 복구만으로 재활성화하지 않는다. 가상 탐사선 움직임과 기존 PC 설정은 유지한다. Pi 수신기의 기존 명령 단절 정지도 유지한다.
- Pi 수신 시작은 고정된 dmix-rc-session.service만 systemctl --user로 시작한다. 셸/임의 명령 실행을 허용하지 않는다. 송신 켜짐·처리 중 요청·구형 Quest 상태일 때 실행을 거부한다. dashboard/dmix-rc-session.service와 setup_rc_service.sh를 준비했다. 설치 스크립트는 수신기를 시작/재시작하지 않고 대시보드만 재시작한다. 수신기 부팅 자동 시작은 추가하지 않는다.
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


## 현재 운영: Raspberry Pi 자동 시작

웹 대시보드와 Pixel Streaming 연결 서버는 Pi `10.95.86.138`의 `/home/pi/dmix-dashboard`로 이전했다. 브라우저에서 **http://10.95.86.138:5050** 을 연다. 노트북은 Unreal 영상 생성·VR 실행에 필요하지만 웹페이지 제공에는 필요하지 않다.

- `/etc/systemd/system/dmix-dashboard.service`: Python 웹서버, 5050 포트, 로컬 MQTT 구독.
- `/etc/systemd/system/dmix-pixel.service`: Node 영상 연결 서버, Unreal 8888 / 브라우저 8889 포트.
- 둘 다 system 서비스로 `enabled`, `active` 확인. 서비스 재시작과 인증 유지 확인. 실제 Pi 전원 재부팅 검사는 수행하지 않았다.
- 상태: `ssh pi@10.95.86.138 "systemctl is-active dmix-dashboard dmix-pixel"`.
- 재시작: `ssh pi@10.95.86.138 "sudo systemctl restart dmix-dashboard dmix-pixel"`.
- 종료: `ssh pi@10.95.86.138 "sudo systemctl stop dmix-dashboard dmix-pixel"`. 다음 부팅에는 다시 실행한다.
- 부팅 자동 시작 해제: `ssh pi@10.95.86.138 "sudo systemctl disable --now dmix-dashboard dmix-pixel"`.
- 소스 갱신은 로컬 dashboard 파일을 Pi 배포 폴더로 복사한다. `pixel/player.js` 변경 후 Pi의 pixel 폴더에서 `npm ci --ignore-scripts` 및 `npm run build`가 필요하다. Python/신호 서버 코드 변경 시 해당 서비스를 재시작한다.

노트북의 `dashboard/.runtime/remote-bridge.json`은 Pi의 `.runtime/bridge.json`을 SSH로 가져온 비공개 연결 설정이다. `connect_local_dashboard.py`가 이 파일을 우선 사용한다. Pi의 `.runtime/bridge-token`은 서비스 재시작/부팅에도 유지하며, 인증 파일들은 Git에 올리지 않는다. 공개 파일로 HTTP 제공하지 않는다. Pi 주소가 바뀌면 서비스의 public-host/DMIX_PUBLIC_HOST, Unreal ConnectionURL, 브릿지의 허용 주소와 로컬 연결 설정을 함께 갱신한다.

Unreal을 열고 아래 명령을 실행한 뒤 Play/VR Preview를 시작한다. 브릿지는 에디터 세션마다 실행하며, Pi 서비스만 재시작할 때는 인증이 유지되어 재실행하지 않아도 된다.

```text
py "C:/Users/user/Documents/d-mix/tools/connect_local_dashboard.py"
```

대시보드는 '관전자 전체 화면'이 기본이다. Unreal의 별도 SceneCapture가 현재 시점에서 1280x720, 수평 시야각100도, 최대30fps의 단안 영상을 만든다. 시청자가 없으면 추가 장면 렌더링을 하지 않는다. 헤드셋의 양안 영상은 유지하며 과거 '구형 VR 좌우 영상 · 한쪽 눈' 옵션은 이전 송신 방식용이다. 정확히 양쪽 눈 영상을 합성한 결과는 아니며 별도의 가로형 관전자 투영이다.

MQTT 브로커는 기존 부팅 자동 시작 상태다. **RC 수신기는 별도의 임시 사용자 서비스이며 이번 웹서버 이전으로 부팅 자동 시작으로 바꾸지 않았다.**

아래 Windows 실행 방법은 로컬 대체 운영용이다.

Windows에서 Python 3와 Node.js 22 LTS 설치 후 `start.cmd`를 실행하고 http://127.0.0.1:5050 을 연다.
서버는 별도 백그라운드 프로세스로 실행된다. 시작 완료 후 실행 창이 닫혀도 대시보드와 영상 서버는 계속 실행된다. 설치/시작이 완료되기 전에는 창을 닫지 않는다. PC 재부팅 후에는 다시 실행해야 한다.
첫 실행은 가상환경과 paho-mqtt를 설치하므로 인터넷 연결이 필요하다. 이후 화면은 외부 CDN 없이 동작한다.
종료: `powershell -NoProfile -ExecutionPolicy Bypass -File "C:\Users\user\Documents\d-mix\dashboard\start.ps1" -Stop`.
`start.cmd -Stop`도 지원한다. 실행기가 기록한 프로세스의 PID·시작 시각·실행 파일을 확인해 종료한다. 기존 영상 서버를 재사용한 경우 그 서버는 종료하지 않는다. Pi 수신기를 시작/종료하지 않는다.
구형 실행기로 켜둔 서버가 있다면 그 창을 먼저 닫고 새 실행기를 사용한다. 같은 설정으로 다시 실행하면 기존 백그라운드 서버를 재사용한다. 설정 변경은 `-Stop` 후 재실행한다.
로그는 `.runtime/dashboard.log`, `.runtime/dashboard-error.log`, `.runtime/pixel.log`, `.runtime/pixel-error.log`에 저장된다. `.runtime/background-processes.json`은 로컬 실행 정보이며 Git에 올리지 않는다.

기본 브로커 10.95.86.138:1883, 구독 토픽 codex/ue-bench/20261019/state.
다른 설정: `powershell -ExecutionPolicy Bypass -File start.ps1 -Broker 10.95.86.138 -Port 5050`.
MQTT 접속 없는 실행은 `-NoMqtt`, 별도 Python 실행 파일 지정은 `-PythonPath "절대경로/python.exe"`를 사용한다. 영상 플레이어의 허용 Origin은 기본 포트5050이므로 다른 포트는 HTTP 검사 용도로 사용한다.
기본 Windows 실행은 127.0.0.1 전용이며, Pi 배포는 명시한 LAN 주소를 허용한다. MQTT 발행 기능은 없다.

## Unreal 연결

대시보드 서버 실행 후 에디터 Cmd 콘솔에 아래 명령 실행:

```text
py "C:/Users/user/Documents/d-mix/tools/connect_local_dashboard.py"
```

Python 모드라면 `exec(open("위 절대경로", encoding="utf-8").read())` 형태로 실행한다.
현재 서버는 비공개 bridge-token 파일을 유지하므로 재시작해도 인증이 유지된다. 인증 파일을 재발급한 경우 연결 설정도 다시 가져와야 한다.
다른 컴퓨터에서는 프로젝트 위치에 맞춰 절대경로를 변경한다.
이 스크립트는 에디터 세션 전용이며 패키징 앱 자동 연결은 아직 지원하지 않는다.
실행해도 MQTT 송신 기본값은 바뀌지 않는다. RC 연결은 기존 rc_session_start.py 설정을 별도로 사용한다.

## 현재 지원 범위

- MQTT 구독 연결과 실제 수신기 출력 텔레메트리 표시. 1.5초 이상 상태가 없으면 숨김.
- Unreal bridge의 Play/속도/복귀 상태·이벤트 표시. 2초 무응답 시 조작 비활성화.
- 수동 전환·복귀·정지는 로컬 Unreal 함수 호출. 요청/응답 구분 및 2초 만료.
- 화면의 정지는 Unreal 탐사 정지 요청이며 독립 하드웨어 비상정지 장치가 아니다.
- Unreal 포커스 이탈 시 차량 정지 정책을 유지한다. 브라우저 조작 직후 Unreal로 돌아가야 할 수 있다.
- Pixel Streaming 2로 에디터 Play 영상 자동 연결. 화면 선택 없이 재접속한다. VR 좌우 영상 수신·한 화면 표시 확인. 패키징 앱은 아직 미검증. RPM·배터리·실차 위치 센서와 VR 상태 자동 감지는 미구현.
- `.runtime/bridge.json`은 세션 인증 토큰이므로 업로드 금지. 서버 종료 후 남아도 다음 시작 때 교체된다.

검사: `python -m unittest discover -s dashboard -p test_server.py` (프로젝트 루트).

## VR 영상 한 화면 보기

이전 'VR 한 화면' 방식은 구형 VR 좌우 영상 중 왼쪽 눈만 비율을 유지해 표시한다. 현재 기본값은 '관전자 전체 화면'이다. 좌우 여백이 생길 수 있으며 두 눈 영상을 합성하거나 늘리지 않는다. 일반 PC Play 영상은 'PC / 원본 화면'을 선택한다. 선택은 브라우저별로 저장되며 VR 장치 자동 감지는 아니다.
변경 적용은 대시보드 새로고침만 필요하다. 서버/Unreal 재시작과 Pixel Streaming 번들 재빌드는 필요 없다. 실제 VR 스트림의 한 화면 표시와 원본 모드 전환을 브라우저에서 확인했다.

## 자동 영상 실행

1. start.cmd 실행 (처음에는 npm 의존성과 플레이어 번들도 설치).
2. http://127.0.0.1:5050 접속 후 Unreal 프로젝트에서 Play 시작.
3. 영상이 자동 표시된다. 종료하면 대기 화면으로 돌아간다. 자동재생 제한 시 영상 재생 버튼을 누른다.

로컬 영상 서버: 8888(Unreal), 8889(브라우저). 대시보드 기본 포트5050 사용.
의존성 업데이트 후에는 powershell -ExecutionPolicy Bypass -File dashboard/setup_pixel.ps1 재실행.
영상은 관전자용이며 원격 입력을 받지 않는다. 운영 상태/버튼 연결은 위 Unreal 연결 절차가 별도로 필요하다.
VR Preview의 좌우 영상 수신 및 한쪽 눈 표시는 확인했다. 패키징 앱 영상 수신은 아직 검증하지 않았다. 독립 실행용 AutoStartStream=True도 지정했으나 패키징 빌드에서의 실제 수신은 추후 확인한다.

공식 라이브러리: https://github.com/EpicGames/PixelStreamingInfrastructure (UE5.7 패키지, MIT 라이선스)
