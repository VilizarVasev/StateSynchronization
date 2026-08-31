cmake_minimum_required(VERSION 3.20)

project(device_state_synchronization LANGUAGES C)

# Export the exact compiler command for each source file.  Editors such as
# VS Code use this database to resolve FreeRTOS types, configuration macros,
# include paths, and target-specific definitions correctly.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

get_filename_component(WORKSPACE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(FREERTOS_KERNEL_PATH "${WORKSPACE_ROOT}/FreeRTOS/Source")

if(NOT EXISTS "${FREERTOS_KERNEL_PATH}/tasks.c")
    message(FATAL_ERROR "FreeRTOS kernel not found at ${FREERTOS_KERNEL_PATH}")
endif()

# FreeRTOS expects the application to supply this interface target.  It tells
# the kernel where the application's FreeRTOSConfig.h is located.
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
target_compile_definitions(freertos_config INTERFACE _WIN32_WINNT=0x0601)

# Use FreeRTOS's Windows simulator port and the heap_4 allocator.
set(FREERTOS_PORT "MSVC_MINGW" CACHE STRING "" FORCE)
set(FREERTOS_HEAP "4" CACHE STRING "" FORCE)
add_subdirectory("${FREERTOS_KERNEL_PATH}" "${CMAKE_BINARY_DIR}/FreeRTOS-Kernel")

add_executable(state_sync_simulation
    src/main.c
    src/Common.c
    src/DeviceA.c
    src/DeviceB.c)
target_include_directories(state_sync_simulation PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/include")
target_link_libraries(state_sync_simulation PRIVATE
    freertos_kernel
    freertos_config)
target_compile_options(state_sync_simulation PRIVATE
    -Wall
    -Wextra
    -Wpedantic)
set_property(TARGET state_sync_simulation PROPERTY C_STANDARD 11)
set_property(TARGET state_sync_simulation PROPERTY C_STANDARD_REQUIRED ON)
