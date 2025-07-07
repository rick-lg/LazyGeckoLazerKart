echo off
setlocal
setlocal enabledelayedexpansion
:: Check for device argument
if "%~1"=="" (
    echo Usage: LGTG_build_update.bat [device_name]
    exit /b 1
)


set SKETCH_NAME=LazyGeckoLazerKart
set TEMPLATE=LazyGeckoLazerKart.ino
set BUILD_DIR=%~dp0\


::If ALL was passed, then loop through all available device headers
echo that
if /I "%~1"=="ALL" (
	echo this
    echo Parameter is ALL
    echo Listing device types from devices folder...

    REM Initialize list
    set "deviceList="

	set loop_target=LOOP_NEXT

    REM Loop through *.h files in "devices" folder
    for %%F in (devices\*.h) do (
        REM Get filename without extension
       :: set "filename=%%~nF"
        set "filename=%%~nF"
        echo Found device type: !filename!	
		call :processDevice "!filename!"
		
    )
	
	goto :EOF
	
) else (

   
    call :processDevice "%~1"
	goto :EOF
	
)

::Start of loop
:processDevice

set DEVICE=%~1
set DEVICE_HEADER=%~dp0devices\%DEVICE%.h

:: Full board name (you can change this if you're not using devkit-v1)
set FQBN=esp32:esp32:esp32da
::set FQBN=esp32:esp32:esp32wrover


set VERSION=findstr /b /c:"#define VERSION_STR" "C:\GitLG\LazyGeckoLazerKart\LazyGeckoLazerKart.ino"
for /f "tokens=2 delims=!" %%A in ('findstr /b /c:"#define VERSION_STR" "C:\GitLG\LazyGeckoLazerKart\LazyGeckoLazerKart.ino"') do (
    set "VERSION=%%A"
)

set "VERSION=%VERSION:"=%"
echo VERSION: "%VERSION%"

:: Check if device header exists
if not exist "%DEVICE_HEADER%" (
    echo Error: Header file "%DEVICE_HEADER%" not found.
	::List out what is in the device folder
	dir devices
    exit /b 1
)

echo HEADER: "%DEVICE_HEADER%"

::Bring in the relevant header. make this overwrite what is in the dir
copy "%DEVICE_HEADER%" "%BUILD_DIR%\LGdevice_type.h" >nul

:: Compile using arduino-cli
C:\GitLG\arduino-cli\arduino-cli compile --fqbn %FQBN% --output-dir ./build_temp --libraries "C:\GitLG\LazyGeckoLazerKart\libraries" "%BUILD_DIR%"

::C:\GitLG\arduino-cli\arduino-cli compile --fqbn esp32:esp32:esp32da --verbose --libraries "C:\GitLG\LazyGeckoLazerKart\libraries" C:\GitLG\LazyGeckoLazerKart\

if errorlevel 1 (
    echo Build failed.
    exit /b 1
)

echo Build successful.


::Copy the .bin over to the builds folder renamed with the version number and the device type
::#define VERSION_STR "7.03.2025_d2.0_DEMO"
::07.03.2025_d2.0_DEMO_WATER.bin
::The d2.0 will be used to tell if the deivce should update or not.
set "BIN_NAME=%VERSION%_%DEVICE%.bin"
echo BIN Name: "%BIN_NAME%"



copy .\build_temp\LazyGeckoLazerKart.ino.bin .\builds\%BIN_NAME%

::End of loop
exit /b

echo 
echo We did it!! Start flashing boards!!

endlocal
