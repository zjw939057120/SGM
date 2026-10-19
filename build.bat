@echo off

set BUILD_DIR=%~dp0build\
set TOOLCHAIN=E:\SDK\native-windows-x64-5.1.0.107-Release\native\build\cmake\ohos.toolchain.cmake

if exist "%BUILD_DIR%CMakeCache.txt" del /q "%BUILD_DIR%CMakeCache.txt"
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd /d "%BUILD_DIR%"

cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE="%TOOLCHAIN%" -DOHOS_ARCH=armeabi-v7a ..
cmake --build .

adb shell rm -f /data/sgm51242
adb push ..\build\sgm51242 /data
adb shell chmod 755 /data/sgm51242
adb shell /data/sgm51242

