@echo off
setlocal

cd /d "%~dp0\.."

cl /nologo /TC /W4 /WX /I. /c tests\dll_build.c /Fo"%TEMP%\exri_dll_build_c.obj"
if errorlevel 1 exit /b 1

cl /nologo /TP /W4 /WX /I. /c tests\dll_build.c /Fo"%TEMP%\exri_dll_build_cpp.obj"
if errorlevel 1 exit /b 1

cl /nologo /TC /W4 /WX /I. /c tests\compile_no_stdio.c /Fo"%TEMP%\exri_no_stdio_c.obj"
if errorlevel 1 exit /b 1

echo MSVC checks passed
