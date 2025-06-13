cmake_minimum_required(VERSION 3.10)
project(elvira)

set(CMAKE_EXPORT_COMPILE_COMMANDS True)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PKGCONFIG "pkg-config")
set(DEPS 
    "wlroots-0.20"
    "wayland-server"
    "xkbcommon"
    "pixman-1")

execute_process(COMMAND ${PKGCONFIG} --variable=pkgdatadir wayland-protocols
                OUTPUT_VARIABLE WAYLAND_PROTOCOLS
                COMMAND ${PKGCONFIG} --variable=wayland_scanner wayland-scanner
                OUTPUT_VARIABLE WAYLAND_SCANNER
                COMMAND ${PKGCONFIG} --cflags ${DEPS}
                OUTPUT_VARIABLE CFLAGS_PKG_CONFIG
                #COMMAND ${WAYLAND_SCANNER} server-header
                COMMAND_ERROR_IS_FATAL ANY)

separate_arguments(CFLAGS_PKG_CONFIG UNIX_COMMAND ${CFLAGS_PKG_CONFIG})

file(GLOB_RECURSE ELVIRASRC "elvira/*.cpp")

add_executable(elvira ${ELVIRASRC})

target_compile_definitions(elvira PUBLIC WLR_USE_UNSTABLE)
target_compile_options(elvira PUBLIC -Werror ${CFLAGS_PKG_CONFIG})
target_link_libraries(elvira PUBLIC ${DEPS})