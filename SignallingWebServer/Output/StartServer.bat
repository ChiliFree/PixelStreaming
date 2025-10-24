@echo off
cd /d "%~dp0"

set "WEB_DIR=%~dp0engine\webserver\"
set "WEB_EXE=%WEB_DIR%webServer.exe"

start "WebServer" "%WEB_EXE%"

echo Web Server start successfully.

timeout /t 3 /nobreak >nul

start "PixelStreamingD" "%~dp0PixelStreamingD.exe"
echo PixelStreamingD start successfully.

exit /b 0