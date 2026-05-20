@echo off
echo.
echo Recompiling MemGuard library...
gcc -Wall -Wextra -g -I./include -c src/memguard.c -o build/memguard.o 2>&1

echo.
echo Compiling %1.c...
gcc -Wall -Wextra -g -I./include -o build/%1.exe examples/%1.c build/memguard.o 2>&1

echo.
echo Running %1.exe...
echo ================================
.\build\%1.exe