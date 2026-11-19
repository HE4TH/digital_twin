# D-Mix Quest 3 1.0.3 설치

[APK 포함 설치 ZIP 다운로드](https://github.com/HE4TH/digital_twin/releases/tag/quest3-v1.0.3-preview)

1. ZIP 전체를 압축 해제합니다.
2. Quest를 USB로 연결하고 헤드셋에서 USB 디버깅을 허용합니다.
3. Install-Quest3.cmd를 실행합니다. ADB가 없으면 Google 약관을 확인하고 YES를 입력하거나 공식 platform-tools 폴더를 옆에 준비합니다.
4. Success 후 알 수 없는 출처에서 D-Mix Underwater를 실행합니다.

기존 앱 삭제 없이 업데이트합니다. 앱은 자동 실행하지 않습니다.
Quest와 Pi를 같은 Wi-Fi에 연결하고 http://10.95.86.138:5050 을 엽니다.
RC 송신은 시작 시 꺼져 있습니다.

VR 위젯에 표시 중인 자동복귀·고래·해파리 알림을 대시보드 영상 위에 함께 표시합니다.
같은 문구와 남은 표시 시간을 전달하며, 자막 만료와 연결 단절 시 자동으로 숨깁니다.
960x540, 목표 30fps이며 이전 1.0.2 실측은 약18fps로 목표에 미달했습니다. 이번 버전의 VR 렌더 FPS는 미측정입니다.
하드웨어 인코딩 실패 시 기존 JPEG 최대 10fps로 돌아갑니다.
Pi 서버도 최신 server.py, quest_video.py, quest-video.js, app.js, index.html이 필요합니다.
APK 빌드·서명·ZIP 검사 및 서버 검사 14개 통과. 별도 시험 화면에서 영상 위 자막 배치 확인.
Quest 1.0.3 설치 완료. 실제 VR 알림과 웹 화면의 동시 비교는 미완료입니다. 시험 화면의 자막 표시·만료·연결 단절 처리를 확인했습니다.
