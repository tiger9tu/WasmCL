#!/bin/bash

if [ -z "$1" ]; then
    echo "Please provide the input filename."
    exit 1
fi

# Specify the paths in your environment
WASI_SDK_ROOT=/home/tiger/workspace/wasi-sdk-21.0
WABT_ROOT=/home/tiger/workspace/wabt
WASMTIME_ROOT=/home/tiger/workspace/wasmtime
FILENAME="$1"

$WASI_SDK_ROOT/bin/clang \
--target=wasm32-unknown-wasi  \
-isystem /usr/include --sysroot   \
$WASI_SDK_ROOT/share/wasi-sysroot \
"$FILENAME" -c -o  \
"$FILENAME.o"

$WASI_SDK_ROOT/bin/wasm-ld \
-m wasm32 \
-L$WASI_SDK_ROOT/share/wasi-sysroot/lib/wasm32-wasi \
$WASI_SDK_ROOT/share/wasi-sysroot/lib/wasm32-wasi/crt1-command.o \
"$FILENAME.o" \
-lc $WASI_SDK_ROOT/lib/clang/17/lib/wasi/libclang_rt.builtins-wasm32.a \
-o "$FILENAME.wasm" --import-undefined

$WABT_ROOT/bin/wasm2wat "$FILENAME.wasm" > "$FILENAME.wat"

