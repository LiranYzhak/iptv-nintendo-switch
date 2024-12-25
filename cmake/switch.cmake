set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_SYSTEM_PROCESSOR "aarch64")
set(CMAKE_CROSSCOMPILING TRUE)

if(NOT DEFINED ENV{DEVKITPRO})
    set(DEVKITPRO "C:/devkitPro")
else()
    set(DEVKITPRO "$ENV{DEVKITPRO}")
endif()

# devkitPro paths
set(DEVKITA64 "${DEVKITPRO}/devkitA64")
set(LIBNX "${DEVKITPRO}/libnx")
set(PORTLIBS "${DEVKITPRO}/portlibs/switch")

# Binaries
set(CMAKE_C_COMPILER "${DEVKITA64}/bin/aarch64-none-elf-gcc.exe")
set(CMAKE_CXX_COMPILER "${DEVKITA64}/bin/aarch64-none-elf-g++.exe")
set(CMAKE_ASM_COMPILER "${DEVKITA64}/bin/aarch64-none-elf-gcc.exe")

# Architecture
set(ARCH "-march=armv8-a+crc+crypto -mtune=cortex-a57 -mtp=soft -fPIE")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ARCH}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ARCH}")
set(CMAKE_ASM_FLAGS "${CMAKE_ASM_FLAGS} ${ARCH}")

set(CMAKE_FIND_ROOT_PATH ${DEVKITA64} ${LIBNX} ${PORTLIBS})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY) 