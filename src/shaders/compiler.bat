@echo off

cd %~dp0

C:\VulkanSDK\1.3.283.0\Bin\glslc.exe shader.vert -o shader.vert.spv
C:\VulkanSDK\1.3.283.0\Bin\glslc.exe shader.frag -o shader.frag.spv

echo "Build Complete"
pause