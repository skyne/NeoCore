@ECHO OFF
@title NeoRealm
CLS
ECHO Initializing Realm (Logon-Server)...
:1
start "Neo Realm" /B /MIN /WAIT NeoRealm.exe -c NeoRealm.conf
if %errorlevel% == 0 goto end
goto 1
:end