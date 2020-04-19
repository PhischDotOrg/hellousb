cmake_minimum_required(VERSION 3.12)
project(hellousb)

include_directories(/usr/local/include)

add_compile_options("-Wall")
add_compile_options("-Werror")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules)
find_package(LibUSB REQUIRED)

set(TARGET_NAME hellousb)
set(TARGET_SRC
    hellousb.cpp
)
add_executable(${TARGET_NAME} ${TARGET_SRC})
target_include_directories(${TARGET_NAME} PRIVATE
    ${LIBUSB_1_INCLUDE_DIRS}
)
target_compile_definitions(${TARGET_NAME} PRIVATE
    ${LIBUSB_1_DEFINITIONS}
)
target_link_libraries(${TARGET_NAME}
    ${LIBUSB_1_LIBRARIES}
)
