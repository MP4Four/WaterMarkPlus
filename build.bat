@echo off
rem Build IDWatermark.exe
rem   build.bat          -> MSVC (run from "x64 Native Tools Command Prompt for VS")
rem   build.bat mingw    -> MinGW-w64 gcc in PATH
setlocal
if /i "%1"=="mingw" goto mingw

rc /nologo /fo app.res app.rc || goto fail
cl /nologo /O1 /W4 /MT /utf-8 /DUNICODE /D_UNICODE main.c app.res /Fe:IDWatermark.exe /link /SUBSYSTEM:WINDOWS /MANIFEST:NO user32.lib gdi32.lib comdlg32.lib comctl32.lib shell32.lib ole32.lib gdiplus.lib || goto fail
del /q main.obj app.res 2>nul
echo OK: IDWatermark.exe
goto :eof

:mingw
windres -O coff -i app.rc -o app.res.o || goto fail
gcc -std=gnu11 -Wall -Wextra -Os -municode -DUNICODE -D_UNICODE main.c app.res.o -o IDWatermark.exe -municode -mwindows -s -static -lgdiplus -lcomdlg32 -lcomctl32 -lshell32 -lgdi32 -lole32 || goto fail
del /q app.res.o 2>nul
echo OK: IDWatermark.exe
goto :eof

:fail
echo BUILD FAILED
exit /b 1
