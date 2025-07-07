@echo off
setlocal

:: Check for device argument
if "%~1"=="" (
    echo Usage: LGTG_build_update.bat [device_name]
    exit /b 1
)

set DEVICE=%~1
set SKETCH_NAME=LazyGeckoLazerKart
set TEMPLATE=LazyGeckoLazerKart.ino
set DEVICE_HEADER=devices\%DEVICE%.h
::set BUILD_DIR=build_bulk\%SKETCH_NAME%
set BUILD_DIR=\

:: Full board name (you can change this if you're not using devkit-v1)
set FQBN=esp32:esp32:esp32



:: Check if device header exists
if not exist "%DEVICE_HEADER%" (
    echo Error: Header file "%DEVICE_HEADER%" not found.
	::List out what is in the device folder
	dir devices
    exit /b 1
)

:: Clean and recreate build directory
::if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
::mkdir "%BUILD_DIR%"

:: Copy sketch and correct header

::Keep the ino in the base dirrectory
::copy "%TEMPLATE%" "%BUILD_DIR%\%SKETCH_NAME%.ino" >nul

::Bring in the relevant header. make this overwrite what is in the dir
copy "%DEVICE_HEADER%" "%BUILD_DIR%\LGdevice_type.h" >nul

:: Compile using arduino-cli
C:\GitLG\arduino-cli_1.2.2_Windows_64bit\arduino-cli compile --fqbn %FQBN% "%BUILD_DIR%"

if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

echo Build successful.

endlocal
