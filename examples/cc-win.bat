@echo off

if "%1"=="" (
    echo Please provide the input filename.
    exit /b 1
)

@REM Specify the paths in your environment
set "WASI_SDK_ROOT=C:\Users\30985\repo\ws\tools\wasi-sdk"
set "WABT_ROOT=C:\Users\30985\repo\ws\tools\wabt"
set "WASMTIME_ROOT=C:\Users\30985\repo\ws\wasmtime"
set "CL_ROOT=C:\Users\30985\repo\ws\OpenCL-SDK"

set "FILENAME=%1"

"%WASI_SDK_ROOT%\bin\clang.exe" ^
--target=wasm32-unknown-wasi ^
-isystem "%CL_ROOT%\install\include" ^
--sysroot "%WASI_SDK_ROOT%\share\wasi-sysroot" ^
"%FILENAME%" -c -o "%FILENAME%.o"

"%WASI_SDK_ROOT%\bin\wasm-ld.exe" -m wasm32 ^
-L"%WASI_SDK_ROOT%\share\wasi-sysroot\lib\wasm32-wasi" ^
"%WASI_SDK_ROOT%\share\wasi-sysroot\lib\wasm32-wasi\crt1-command.o" "%FILENAME%.o" -lc "%WASI_SDK_ROOT%\lib\clang\17\lib\wasi\libclang_rt.builtins-wasm32.a"  -o "%FILENAME%.wasm" --import-undefined

wasm-custom-section "%FILENAME%.wasm" add SPIRV < "sample_kernel64.spv"

del "%FILENAME%.wasm"
ren "%FILENAME%.wasm.out" "%FILENAME%.wasm"

"%WABT_ROOT%\bin\wasm2wat.exe" "%FILENAME%.wasm" > "%FILENAME%.wat"



call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" amd64

cl.exe /nologo /TC /W4 /DCL_TARGET_OPENCL_VERSION=300 /I%CL_ROOT%\install\include\ ^
%FILENAME% /Fe:%FILENAME%-CL.exe /link /LIBPATH:%CL_ROOT%\install\lib  OpenCL.lib
