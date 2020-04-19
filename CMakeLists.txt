cmake_minimum_required(VERSION 3.12)

include_directories(/usr/local/include)

add_compile_options("-Wall")
add_compile_options("-Werror")

find_library(LIBUSB
    NAMES usb-1.0
    PATHS
      /usr/lib
      /usr/local/lib
    DOC "Path to libusb-1.0"
)

message(STATUS "libusb: ${LIBUSB}")

set(TARGET_NAME hellousb)
set(TARGET_SRC
    hellousb.cpp
)
add_executable(${TARGET_NAME} ${TARGET_SRC})
target_link_libraries(${TARGET_NAME}
    ${LIBUSB}
)
