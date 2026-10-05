@echo off
echo ========================================================
echo Compiling Student Record Management System (C DSA Core)
echo ========================================================

if not exist "data" mkdir "data"

gcc -Wall -Wextra -O2 c_src\linked_list.c c_src\stack.c c_src\queue.c c_src\hash_table.c c_src\bst.c c_src\graph.c c_src\sorting_searching.c c_src\file_io.c c_src\main.c -o dsa_core.exe

if %ERRORLEVEL% equ 0 (
    echo.
    echo [SUCCESS] dsa_core.exe compiled successfully!
    echo Run ".\dsa_core.exe --cli" for interactive menu.
    echo Run ".\dsa_core.exe --api list" for JSON API mode.
) else (
    echo.
    echo [ERROR] Compilation failed. Please check the errors above.
)
