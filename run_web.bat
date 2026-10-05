@echo off
echo ====================================================================
echo  Starting Student Record Management System (Web Application Mode)
echo ====================================================================
echo.

REM Ensure binary is compiled
if not exist "dsa_core.exe" (
    echo [INFO] Compiling C DSA Engine...
    call build.bat
)

echo [INFO] Launching Python Flask Web Server...
echo [INFO] Open your browser and navigate to: http://127.0.0.1:5000
echo.
python app.py
pause
