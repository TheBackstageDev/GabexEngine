@echo off

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

SET VULKAN_SDK=C:\VulkanSDK\1.3.283.0
SET GLFW_INCLUDE=C:\VulkanSDK\1.3.283.0\includes\glfw\include
SET GLFW_LIB=C:\VulkanSDK\1.3.283.0\Include\glfw\lib-mingw-w64

SET includes=/Isrc /I%VULKAN_SDK%/Include /I%GLFW_INCLUDE%
SET links=/link /LIBPATH:%VULKAN_SDK%/Lib /LIBPATH:%GLFW_LIB% vulkan-1.lib libglfw3dll.a
SET defines=/DDEBUG

echo "Building Main..."

REM Compile all .cpp files to object files first
cl /c /EHsc %includes% %defines% src\*.cpp

REM Link all object files into the final executable named main.exe
cl /EHsc %includes% %defines% *.obj /Fe:main.exe %links%

REM Cleanup intermediate object files
del *.obj

echo "Build complete."
