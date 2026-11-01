cmake_minimum_required(VERSION 3.13)

set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/pico-sdk" CACHE PATH "Path to the Raspberry Pi Pico SDK")
set(PICO_BOARD "pico2" CACHE STRING "Pico board target, such as pico2 or pico2_w")

include(${PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(otp_dumper_fw C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

option(OTP_DUMP_STDIO_USB "Enable USB CDC stdio" ON)
option(OTP_DUMP_STDIO_UART "Enable UART stdio" ON)

if(NOT OTP_DUMP_STDIO_USB AND NOT OTP_DUMP_STDIO_UART)
    message(FATAL_ERROR "Enable at least one of OTP_DUMP_STDIO_USB or OTP_DUMP_STDIO_UART")
endif()

add_executable(otp_dump otp_dump.c)
target_link_libraries(otp_dump PRIVATE pico_stdlib)
pico_enable_stdio_usb(otp_dump ${OTP_DUMP_STDIO_USB})
pico_enable_stdio_uart(otp_dump ${OTP_DUMP_STDIO_UART})
pico_add_extra_outputs(otp_dump)

# RAM variant: picotool load -x boots it without touching flash
add_executable(otp_dump_ram otp_dump.c)
target_link_libraries(otp_dump_ram PRIVATE pico_stdlib)
pico_enable_stdio_usb(otp_dump_ram ${OTP_DUMP_STDIO_USB})
pico_enable_stdio_uart(otp_dump_ram ${OTP_DUMP_STDIO_UART})
pico_set_binary_type(otp_dump_ram no_flash)
pico_add_extra_outputs(otp_dump_ram)
