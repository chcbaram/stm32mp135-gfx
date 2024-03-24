cmake_minimum_required (VERSION 3.14)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../../tools")
include(arm-none-eabi-gcc)


get_filename_component(ROOT ../../ST ABSOLUTE)

set(DSP ${ROOT}/CMSIS/DSP)
set(CMAKE_TOOLCHAIN_FILE ${DSP}/Toolchain/GCC.cmake)
set(ARM_CPU "cortex-a7")
set(NEON ON)
set(CONFIGTABLE ON)
set(CFFT_Q15_512 ON)


# Define the project
project (cmsis-lib VERSION 0.1)

### Add DSP folder to module path
list(APPEND CMAKE_MODULE_PATH ${DSP} RELATIVE)

add_subdirectory(${DSP}/Source dsp-build)
