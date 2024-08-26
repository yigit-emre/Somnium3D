@echo off

:COMPILE
set /p filename="Enter the file name to compile: "
for %%f in (%filename%) do set name=%%~nf
D:/visualDEV/Dependencies/VulkanSDK/1.3.268.0/Bin/glslc.exe %filename% -o %name%.spv
pause
cls
goto COMPILE