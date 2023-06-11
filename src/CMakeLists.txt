# KevSays ("Kevin Says") Main Project File

cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_BOARD pico CACHE STRING "Board type")
set(PICO_USE_MALLOC_MUTEX 1)

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
if(PICO_SDK_VERSION_STRING VERSION_LESS "1.4.0")
        message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.4.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Use fully-deoptimized debug build for true single-step and data viewing. Set to '0' for some optimizations
set(PICO_DEOPTIMIZED_DEBUG 1)

pico_sdk_init()

project(KevSays C CXX ASM)

add_compile_options(
        -O0 # No optimizations for now, for debugging
        -Wall
        -Wno-format # int != int32_t as far as the compiler is concerned
        -Wno-unused-function
        -Wno-maybe-uninitialized
        # -H                       # list include header paths
)

# External libraries
#add_subdirectory(lib/sd_card)

# Local libraries (additional sources)
add_subdirectory(be)
add_subdirectory(cmt)
add_subdirectory(gfx)
add_subdirectory(ui)
add_subdirectory(util)

add_executable(KevSays
        main.c
        board.c
        debug.c
        spi_ops.c
)

pico_set_program_name(KevSays "KevSays")
pico_set_program_version(KevSays "0.1")

# Use the UART while using the Picoprobe
# For Release the USB will be better
pico_enable_stdio_uart(KevSays 1)
pico_enable_stdio_usb(KevSays 0)

# Add the required include file paths to the build
include_directories(
        ${CMAKE_CURRENT_LIST_DIR}
        ${PICO_SDK_PATH}/src/common/pico_base/include
        ${PICO_SDK_PATH}/src/common/pico_stdlib/include
        ${PICO_SDK_PATH}/src/rp2_common/hardware_dma/include
        ${PICO_SDK_PATH}/src/rp2_common/hardware_spi/include
        ${PICO_SDK_PATH}/src/rp2_common/hardware_pio/include
        ${PICO_SDK_PATH}/src/rp2_common/hardware_rtc/include
        ${PICO_SDK_PATH}/src/rp2_common/pico_multicore/include
        be
        cmt
        gfx
        ui
        ui/display
        ui/display/ili_lcd_spi
        util
)

# ZZZ Figure out how to set `PICO_DEBUG_MALLOC` only if 'debug build'
add_compile_definitions(
        PICO_MALLOC_PANIC
        PICO_USE_MALLOC_MUTEX
        PICO_USE_STACK_GUARDS
        PICO_STACK_SIZE=4096
        PICO_CORE1_STACK_SIZE=4096

        # PICO_DEBUG_MALLOC
)

# Add the libraries required by the system to the build
target_link_libraries(KevSays
        hardware_adc
        hardware_clocks
        hardware_exception
        hardware_i2c
        hardware_pio
        hardware_spi
        hardware_timer
        pico_multicore
        pico_stdlib
        be
        cmt
        display
        gfx
        ili_lcd_spi
        ui
)

# Use our linker script
pico_set_linker_script(KevSays ${CMAKE_SOURCE_DIR}/memmap_custom.ld)

# create map/bin/hex file etc.
pico_add_extra_outputs(KevSays)

