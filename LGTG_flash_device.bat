::Take COM and Device Type and flash with the correct image
echo off
setlocal
setlocal enabledelayedexpansion

::LGTG_flash_device.bat COM6 WATER


set COMPORT=%~1
set DEVICE=%~2

REM Folder to search (can be . for current)
set "folder=.\builds"
::set "bloader=.\build_core"
set "bloader=.\build_temp"

REM Pattern to match files (change if needed)
for %%F in (%folder%\*.bin) do (
    echo %%~nF | find /I "%DEVICE%" >nul
    if not errorlevel 1 (
		set DEVICE_FILE=%%F
    )
)


echo Device Image : %DEVICE_FILE%
"C:\Users\RickPease\AppData\Local\Arduino15\packages\esp32\tools\esptool_py\5.0.dev1/esptool.exe" --chip esp32 --port "%COMPORT%" --baud 921600  --before default-reset --after hard-reset write-flash  -z --flash-mode keep --flash-freq keep --flash-size keep 0x1000 "%bloader%\LazyGeckoLazerKart.ino.bootloader.bin" 0x8000 "%bloader%\LazyGeckoLazerKart.ino.partitions.bin" 0xe000 "C:\Users\RickPease\AppData\Local\Arduino15\packages\esp32\hardware\esp32\3.2.1/tools/partitions/boot_app0.bin" 0x10000 %DEVICE_FILE%

goto :EOF
