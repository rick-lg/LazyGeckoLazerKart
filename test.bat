@echo off
setlocal





for /f "tokens=2 delims=!" %%A in ('findstr /b /c:"#define VERSION_STR" "C:\GitLG\LazyGeckoLazerKart\LazyGeckoLazerKart.ino"') do (
    set "versionStr1=%%A"
)
echo STR: "%versionStr1%"


endlocal
pause

