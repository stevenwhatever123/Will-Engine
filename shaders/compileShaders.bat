@echo off

Setlocal EnableDelayedExpansion

set VULKAN_VERSION=0.0.0.0

rem Get vulkan version from vulkan_version.txt
for /f %%i in (vulkan_version.txt) do (
    set VULKAN_VERSION=%%i
    echo Vulkan Version: %%i
)

for /r %%f in (.\*.vert, .\*.frag, .\*.geom, .\*.comp) do (
    set FILE_NAME=%%~f
    echo Compiling !FILE_NAME!
    set TARGET_FILE_NAME=!FILE_NAME!.spv
    C:/VulkanSDK/!VULKAN_VERSION!/Bin/glslc.exe !FILE_NAME! -o !TARGET_FILE_NAME!
)

echo Complete

pause