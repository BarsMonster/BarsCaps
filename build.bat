@echo off
echo Building x86...
call vcvars32.bat"
if errorlevel 1 (
    echo Failed to set up x86 environment.
    exit /b 1
)
cl /O1 /MT /Gy /GL /GS- /EHsc /Fe:barscaps_x86.exe barscaps.cpp /link /NOLOGO /SUBSYSTEM:WINDOWS /LTCG
if errorlevel 1 (
    echo x86 build failed.
    exit /b 1
)

echo Building x64...
call vcvars64.bat"
if errorlevel 1 (
    echo Failed to set up x64 environment.
    exit /b 1
)
cl /O1 /MT /Gy /GL /GS- /EHsc /Fe:barscaps_x64.exe barscaps.cpp /link /NOLOGO /SUBSYSTEM:WINDOWS /LTCG
if errorlevel 1 (
    echo x64 build failed.
    exit /b 1
)

echo Building ARM64...
call vcvarsamd64_arm64.bat"
if errorlevel 1 (
    echo Failed to set up ARM64 environment.
    exit /b 1
)
cl /O1 /MT /Gy /GL /GS- /EHsc /Fe:barscaps_arm64.exe barscaps.cpp /link /NOLOGO /SUBSYSTEM:WINDOWS /LTCG
if errorlevel 1 (
    echo ARM64 build failed.
    exit /b 1
)

echo Build completed successfully.