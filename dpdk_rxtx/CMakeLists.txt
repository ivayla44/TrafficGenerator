cmake_minimum_required(VERSION 3.22)

project(dpdk_rxtx C)

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

add_executable(dpdk_rxtx trgen.c)

add_compile_options(-Wall -Wextra -std=c99)

set(ENV{PKG_CONFIG_PATH} "${DPDK_TARGET_DIR}/lib/x86_64-linux-gnu/pkgconfig/")

pkg_search_module(LIBDPDK REQUIRED libdpdk)
target_compile_options(dpdk_rxtx PRIVATE ${LIBDPDK_CFLAGS})
target_link_libraries(dpdk_rxtx PRIVATE ${LIBDPDK_STATIC_LDFLAGS} -lnuma)

