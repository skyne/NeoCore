@ECHO OFF
@title NeoCore
CLS
ECHO Initializing Core (World-Server)...
:1
start "Neo Core" /B /MIN /WAIT NeoCore.exe -c NeoCore.conf
if %errorlevel% == 0 goto end
goto 1
:end