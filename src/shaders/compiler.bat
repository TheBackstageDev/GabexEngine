@echo off
setlocal enabledelayedexpansion

:: Set the directory containing your shaders
set SHADER_DIR=C:\Users\cleve\OneDrive\Documents\GitHub\GabexEngine\src\shaders

:: Set the output directory for SPIR-V files
set OUTPUT_DIR=%SHADER_DIR%

:: Set the path to glslangValidator
set GLSLANG_VALIDATOR=glslangValidator

:: Create the output directory if it doesn't exist
if not exist "%OUTPUT_DIR%" (
    mkdir "%OUTPUT_DIR%"
)

:: Compile all .vert shaders
for /r "%SHADER_DIR%" %%f in (*.vert) do (
    set SHADER_FILE=%%f
    set FILE_NAME=%%~nf
    set FILE_EXT=%%~xf

    :: Set the output file name
    set OUTPUT_FILE=%OUTPUT_DIR%\!FILE_NAME!!FILE_EXT!.spv

    :: Compile the shader
    echo Compiling !SHADER_FILE! to !OUTPUT_FILE!
    %GLSLANG_VALIDATOR% -V !SHADER_FILE! -o !OUTPUT_FILE!
    if !ERRORLEVEL! NEQ 0 (
        echo Error compiling !SHADER_FILE!
        exit /b 1
    )
)

:: Compile all .frag shaders
for /r "%SHADER_DIR%" %%f in (*.frag) do (
    set SHADER_FILE=%%f
    set FILE_NAME=%%~nf
    set FILE_EXT=%%~xf

    :: Set the output file name
    set OUTPUT_FILE=%OUTPUT_DIR%\!FILE_NAME!!FILE_EXT!.spv

    :: Compile the shader
    echo Compiling !SHADER_FILE! to !OUTPUT_FILE!
    %GLSLANG_VALIDATOR% -V !SHADER_FILE! -o !OUTPUT_FILE!
    if !ERRORLEVEL! NEQ 0 (
        echo Error compiling !SHADER_FILE!
        exit /b 1
    )
)

echo All shaders compiled successfully!
exit /b 0