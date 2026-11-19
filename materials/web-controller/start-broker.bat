@echo off
REM ---------------------------------------------------------------------------
REM  Mosquitto 브로커 실행 (Windows)
REM  TCP 1883 = Unreal 전용(127.0.0.1) / WebSocket 9001 = 웹, 휴대폰
REM ---------------------------------------------------------------------------
setlocal
cd /d "%~dp0"

set "MOSQ=C:\Program Files\mosquitto\mosquitto.exe"
if not exist "%MOSQ%" set "MOSQ=C:\Program Files (x86)\mosquitto\mosquitto.exe"

if not exist "%MOSQ%" (
  echo [ERROR] mosquitto.exe 를 찾지 못했습니다.
  echo         https://mosquitto.org/download/ 에서 Windows x64 설치본을 먼저 설치하세요.
  echo         다른 경로에 설치했다면 이 파일의 MOSQ 변수를 수정하세요.
  pause
  exit /b 1
)

REM 설치 시 자동 등록되는 Windows 서비스가 1883 을 이미 점유한다.
REM 서비스가 떠 있으면 여기서 "Address already in use" 로 즉시 죽으므로 먼저 내린다.
echo [1/2] mosquitto 서비스가 떠 있으면 중지합니다 (관리자 권한 필요할 수 있음)...
net stop mosquitto >nul 2>&1

echo [2/2] 브로커를 시작합니다. 종료하려면 Ctrl+C.
echo.
"%MOSQ%" -c "%~dp0mosquitto.windows.conf" -v

endlocal
