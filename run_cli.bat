@echo off
echo ====================================================================
echo  Student Record Management System (C Terminal Viva / CLI Mode)
echo ====================================================================
echo.

if not exist "dsa_core.exe" (
    echo [INFO] Compiling C DSA Engine...
    call build.bat
)

.\dsa_core.exe --cli
pause
