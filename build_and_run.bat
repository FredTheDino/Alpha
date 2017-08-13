@echo off
del bin /Q
call build.bat

echo -
echo ============ Done Building
echo -

call bin\windows.exe