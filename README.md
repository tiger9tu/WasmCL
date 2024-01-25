# opencl2wasm

#### 介绍
能够让opencl在wasmtime虚拟机中运行

#### 示例

编译程序：
```
cd build
cmake ..
cmake build .
```

将opencl文件编译为wasm:
```
cd examples
./cc-win.bat hellocl.c
```

运行：
```
./wasmtime-cl ../examples/hellocl.c.wasm
```

