cmake_minimum_required(VERSION 3.20)

project(device_state_synchronization)

# Get kernel path
get_filename_component(WORKSPACE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(FREERTOS_KERNEL_PATH "${WORKSPACE_ROOT}/FreeRTOS/Source")

# Check if kernel path is incorrect
if(NOT EXISTS "${FREERTOS_KERNEL_PATH}/tasks.c")
    message(FATAL_ERROR "FreeRTOS kernel not found at ${FREERTOS_KERNEL_PATH}")
endif()

# FreeRTOSConfig.h location.
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config INTERFACE "${CMAKE_CURRENT_LIST_DIR}")

# Use FreeRTOS's Windows simulator port and the heap_4 allocator.
set(FREERTOS_PORT "MSVC_MINGW" CACHE STRING "FreeRTOS port")
set(FREERTOS_HEAP "4" CACHE STRING "FreeRTOS heap implementation")

# Kernel path
add_subdirectory("${FREERTOS_KERNEL_PATH}" "${CMAKE_BINARY_DIR}/FreeRTOS-Kernel")

# Components implementation files 
add_executable(state_sync_simulation
    src/main.c
    src/Common.c
    src/DeviceA.c
    src/DeviceB.c)

# Add the include directory
target_include_directories(state_sync_simulation PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/include")

# Link to the FreeRTOS kernel
target_link_libraries(state_sync_simulation PRIVATE
    freertos_kernel
    freertos_config)
