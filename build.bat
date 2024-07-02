@echo off

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

SET VULKAN_SDK=C:\VulkanSDK\1.3.283.0
SET GLFW_INCLUDE=C:\VulkanSDK\1.3.283.0\include\glfw
SET GLFW_LIB=C:\VulkanSDK\1.3.283.0\Include\glfw\lib-mingw-w64
SET ASSIMP_INCLUDE=C:\Program Files\Assimp\include
SET ASSIMP_LIB=C:\Program Files\Assimp\lib\x64
SET IMGUI_INCLUDE=C:\VulkanSDK\1.3.283.0\include\imgui

SET includes=/I"%VULKAN_SDK%\Include" /I"%GLFW_INCLUDE%" /I"%ASSIMP_INCLUDE%" /I"%IMGUI_INCLUDE%" /I"%IMGUI_INCLUDE%\backends"
SET links=/link /LIBPATH:"%VULKAN_SDK%\Lib" /LIBPATH:"%GLFW_LIB%" /LIBPATH:"%ASSIMP_LIB%" vulkan-1.lib libglfw3dll.a assimp-vc143-mt.lib
SET defines=/DDEBUG

echo "Building Main..."

REM Compile all .cpp files to object files first
for /R src %%f in (*.cpp) do (
    if not "%%~nxf"=="main.cpp" (
        cl /std:c++17 /c /EHsc %includes% %defines% "%%f"
    )
)

REM Compile ImGui source files
for %%f in (imgui.cpp imgui_draw.cpp imgui_widgets.cpp imgui_tables.cpp imgui_demo.cpp backends/imgui_impl_vulkan.cpp backends/imgui_impl_glfw.cpp) do (
    cl /std:c++17 /c /EHsc %includes% %defines% "%IMGUI_INCLUDE%\%%f"
)

REM Compile main.cpp last
cl /std:c++17 /c /EHsc %includes% %defines% src\main.cpp

REM Link all object files into the final executable named main.exe
cl /std:c++17 /EHsc %includes% %defines% *.obj /Fe:main.exe %links%

REM Cleanup intermediate object files
del *.obj

cd C:\Users\cleve\OneDrive\Documents\GitHub\GabexEngine\src\shaders

C:\VulkanSDK\1.3.283.0\Bin\glslc.exe shader.vert -o shader.vert.spv
C:\VulkanSDK\1.3.283.0\Bin\glslc.exe shader.frag -o shader.frag.spv
C:\VulkanSDK\1.3.283.0\Bin\glslc.exe point_light.vert -o point_light.vert.spv
C:\VulkanSDK\1.3.283.0\Bin\glslc.exe point_light.frag -o point_light.frag.spv

echo "Build complete."

C:\Users\cleve\OneDrive\Documents\GitHub\GabexEngine\main.exe
