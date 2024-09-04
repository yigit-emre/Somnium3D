@echo off
setlocal enabledelayedexpansion

for %%f in (*.vert *.frag) do (
    D:/visualDEV/Dependencies/VulkanSDK/1.3.268.0/Bin/glslc.exe %%f -o %%~nf.spv
    if !ERRORLEVEL! neq 0 (
        pause
        exit /b 1
    )
)
exit