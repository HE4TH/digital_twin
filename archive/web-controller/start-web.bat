@echo off
REM ---------------------------------------------------------------------------
REM  웹 컨트롤러 정적 서버 (Windows)
REM  PC:   http://127.0.0.1:5173
REM  휴대폰: http://<이 PC 의 Wi-Fi IPv4>:5173   (같은 Wi-Fi 여야 함)
REM ---------------------------------------------------------------------------
setlocal
cd /d "%~dp0"

if not exist "node_modules\mqtt\dist\mqtt.min.js" (
  echo [ERROR] mqtt 라이브러리가 없습니다. 먼저 아래를 실행하세요:
  echo             cd /d "%~dp0"
  echo             npm install
  pause
  exit /b 1
)

echo.
echo  이 PC 의 IPv4 주소:
for /f "tokens=2 delims=:" %%i in ('ipconfig ^| findstr /c:"IPv4"') do echo    %%i
echo.
echo  휴대폰에서는 위 주소 중 Wi-Fi 어댑터 것을 골라 http://주소:5173 으로 접속하세요.
echo  종료하려면 Ctrl+C.
echo.

py -m http.server 5173

endlocal
