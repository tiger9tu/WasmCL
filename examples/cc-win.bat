@echo off
set "WASI_SDK_ROOT=C:\Users\30985\repo\workplace\install\wasi-sdk-20.0+m"
set "WABT_ROOT=C:\Users\30985\repo\workplace\install\wabt"
set "WASMTIME_ROOT=C:\Users\30985\repo\workplace\wasmtime"
set "CL_ROOT=C:\Users\30985\repo\workplace\OpenCL-SDK"

"%WASI_SDK_ROOT%\bin\clang.exe" ^
--target=wasm32-unknown-wasi ^
-isystem "%CL_ROOT%\install\include" ^
--sysroot "%WASI_SDK_ROOT%\share\wasi-sysroot" ^
hellocl.c -c -o hellocl.o

"%WASI_SDK_ROOT%\bin\wasm-ld.exe" -m wasm32 ^
-L"%WASI_SDK_ROOT%\share\wasi-sysroot\lib\wasm32-wasi" ^
"%WASI_SDK_ROOT%\share\wasi-sysroot\lib\wasm32-wasi\crt1-command.o" hellocl.o -lc "%WASI_SDK_ROOT%\lib\clang\16\lib\wasi\libclang_rt.builtins-wasm32.a" -o hellocl.wasm --import-undefined

"%WABT_ROOT%\bin\wasm2wat.exe" hellocl.wasm > hellocl.wat
