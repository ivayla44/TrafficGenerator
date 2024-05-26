cmake_minimum_required(VERSION 3.22)

project(traffic_generator C)

find_package(PkgConfig REQUIRED)

set(3RD_PARTY_DIR ${CMAKE_SOURCE_DIR}/3rdparty)
set(DPDK_TARGET_DIR "${3RD_PARTY_DIR}/dpdk/target")

execute_process(
        COMMAND ${CMAKE_COMMAND} ${3RD_PARTY_DIR}
        WORKING_DIRECTORY ${3RD_PARTY_DIR}
)
execute_process(
        COMMAND ${CMAKE_COMMAND} --build ${3RD_PARTY_DIR}
)

file(GLOB PROJECT_SOURCES CONFIGURE_DEPENDS
        "*.c"
        "app/*.c"
        "app/cfg/*.c"
        "gen/*.c"
        "gen/priv/*.c"
        "mgmt/*.c"
        "utils/*.c"
        )

add_executable(traffic_generator
        ${PROJECT_SOURCES}
        )


add_compile_options(-Wall -Wextra -std=c99)

set(ENV{PKG_CONFIG_PATH} "${DPDK_TARGET_DIR}/lib/x86_64-linux-gnu/pkgconfig/")

set(LIBFYAML_INCLUDE_DIR "${3RD_PARTY_DIR}/libfyaml/install/include")
set(LIBFYAML_LIBRARY_DIR "${3RD_PARTY_DIR}/libfyaml/install/lib")

set(LIBCONFIG_INCLUDE_DIR "${3RD_PARTY_DIR}/libconfig/install/include")
set(LIBCONFIG_LIBRARY_DIR "${3RD_PARTY_DIR}/libconfig/install/lib")

target_include_directories(traffic_generator
        PRIVATE ${LIBFYAML_INCLUDE_DIR} ${LIBCONFIG_INCLUDE_DIR}
        )

target_link_directories(traffic_generator PRIVATE ${LIBFYAML_LIBRARY_DIR} ${LIBCONFIG_LIBRARY_DIR})
target_link_libraries(traffic_generator PRIVATE fyaml config)


pkg_search_module(LIBDPDK REQUIRED libdpdk)
target_compile_options(traffic_generator PRIVATE ${LIBDPDK_CFLAGS})
target_link_libraries(traffic_generator PRIVATE ${LIBDPDK_STATIC_LDFLAGS} -lnuma -lpcap -lfyaml -lconfig)


target_precompile_headers(traffic_generator
        PUBLIC precompiled.h
)
