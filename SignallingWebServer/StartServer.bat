@echo off
cd /d "%~dp0"

echo 启动 webServer.exe ...
start "WebServer" "%~dp0webServer.exe"

timeout /t 3 /nobreak >nul

start "PixelStreamingD" "%~dp0PixelStreamingD.exe"

exit /b 0