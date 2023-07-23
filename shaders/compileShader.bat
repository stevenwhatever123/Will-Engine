@echo off

Setlocal EnableDelayedExpansion

for /r %%f in (.\*.vert, .\*.frag, .\*.geom, .\*.comp) do (
    set FILE_NAME=%%~f
    echo Compiling !FILE_NAME!
    set TARGET_FILE_NAME=!FILE_NAME!.spv
    C:/VulkanSDK/1.3.250.1/Bin/glslc.exe !FILE_NAME! -o !TARGET_FILE_NAME!
)

pause