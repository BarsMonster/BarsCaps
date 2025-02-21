@echo off
REM Check if Visual Studio tools are available
where cl >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Setting up Visual Studio environment...
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat" || (
        echo Error: Could not find Visual Studio tools
        exit /b 1
    )
)

taskkill /IM "test.exe" /F

REM Get the most recently modified .cpp file
for /f "delims=" %%F in ('dir *.cpp /b /o-d') do (
    echo Compiling latest file: %%F
    cl /O1 /MT /Gy /GL /GS- /EHsc /Fe:test.exe "%%F" /link /NOLOGO /SUBSYSTEM:WINDOWS /LTCG
    goto :done
)

echo No .cpp files found
exit /b 1

:done
if %ERRORLEVEL% equ 0 (
    echo Compilation successful
    start test.exe
) else (
    echo Compilation failed
)

