cmake_minimum_required(VERSION 3.10)
project(opencl2wasm)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11)
set(WASMTIME_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../wasmtime)

add_subdirectory(${WASMTIME_ROOT}/crates/c-api ${CMAKE_CURRENT_BINARY_DIR}/wasmtime)

find_package(OpenCL)

add_executable(wasmtime-cl main.c wrap.c mem.c)


if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	target_compile_options(wasmtime-cl PRIVATE -Wall -Wextra -Wno-deprecated-declarations)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
	target_compile_options(wasmtime-cl PRIVATE /W3)
endif()

set_target_properties(wasmtime-cl PROPERTIES
	OUTPUT_NAME wasmtime-cl
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/$<0:>
	CXX_VISIBILITY_PRESET hidden
	POSITION_INDEPENDENT_CODE ON)

	target_include_directories(wasmtime-cl PUBLIC wasmtime)
	target_link_libraries(wasmtime-cl PUBLIC wasmtime)

	target_include_directories(wasmtime-cl PRIVATE ${OpenCL_INCLUDE_DIRS})
	target_link_libraries(wasmtime-cl PRIVATE ${OpenCL_LIBRARIES})
