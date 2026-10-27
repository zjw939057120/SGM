@echo off

set BUILD_DIR=%~dp0\build
set TOOLCHAIN=D:\SDK\native\build\cmake\ohos.toolchain.cmake

if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

cmake -S . -B "%BUILD_DIR%" -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE="%TOOLCHAIN%" -DOHOS_ARCH=armeabi-v7a

cmake --build "%BUILD_DIR%"

adb shell rm -f /data/sgm51242
adb push "%BUILD_DIR%\sgm51242" /data
adb shell chmod 755 /data/sgm51242
adb shell /data/sgm51242

