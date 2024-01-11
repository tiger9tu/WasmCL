WASI_SDK_ROOT=/home/tiger/workspace/wasi-sdk-21.0
WABT_ROOT=/home/tiger/workspace/wabt
WASMTIME_ROOT=/home/tiger/workspace/wasmtime

$WASI_SDK_ROOT/bin/clang \
--target=wasm32-unknown-wasi  \
-isystem /usr/include --sysroot   \
$WASI_SDK_ROOT/share/wasi-sysroot \
hellocl.c -c -o  \
build/hellocl.o

$WASI_SDK_ROOT/bin/wasm-ld \
-m wasm32 \
-L$WASI_SDK_ROOT/share/wasi-sysroot/lib/wasm32-wasi \
$WASI_SDK_ROOT/share/wasi-sysroot/lib/wasm32-wasi/crt1-command.o \
build/hellocl.o \
-lc $WASI_SDK_ROOT/lib/clang/17/lib/wasi/libclang_rt.builtins-wasm32.a \
-o build/hellocl.wasm --import-undefined

$WABT_ROOT/bin/wasm2wat build/hellocl.wasm > build/hellocl.wat

cc main.c -I $WASMTIME_ROOT/crates/c-api/include \
-I $WASMTIME_ROOT/crates/c-api/wasm-c-api/include \
$WASMTIME_ROOT/target/release/libwasmtime.a \
-lpthread -ldl -lm -o build/run