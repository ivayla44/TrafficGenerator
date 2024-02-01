cmake_minimum_required(VERSION 3.22)

project(3rdparty)

include(FetchContent)
include(ExternalProject)

set(CPU_TYPE "corei7")
set(3RD_PARTY_PREFIX_DIR "${CMAKE_SOURCE_DIR}")
set(3RD_PARTY_DOWNLOAD_DIR "${3RD_PARTY_PREFIX_DIR}")
set(3RD_PARTY_TMP_DIR "${3RD_PARTY_PREFIX_DIR}/tmp")

execute_process(
        COMMAND grep processor /proc/cpuinfo
        COMMAND wc -l
        OUTPUT_VARIABLE NUM_CPUS
)

################################################################################
# DPDK Libraries

set(DPDK_PREFIX_DIR "${3RD_PARTY_PREFIX_DIR}/dpdk")
set(DPDK_BUILD_DIR "${DPDK_PREFIX_DIR}/build")
set(DPDK_TARGET_DIR "${DPDK_PREFIX_DIR}/target")

ExternalProject_Add(
        dpdk

        GIT_REPOSITORY "https://github.com/DPDK/dpdk"
        GIT_TAG "v23.07"

        PREFIX ${3RD_PARTY_PREFIX_DIR}
        TMP_DIR ${3RD_PARTY_TMP_DIR}
        DOWNLOAD_DIR ${3RD_PARTY_DOWNLOAD_DIR}
        SOURCE_DIR ${DPDK_PREFIX_DIR}
        STAMP_DIR ${DPDK_PREFIX_DIR}-stamp
        #BINARY_DIR ${DPDK_PREFIX_DIR}-build

        CONFIGURE_COMMAND mkdir -p ${DPDK_BUILD_DIR} &&
            cd ${DPDK_BUILD_DIR} &&
            meson setup
            --prefix ${DPDK_TARGET_DIR}
            --buildtype release
            --default-library static
            --warnlevel 2
            -Denable_kmods=false
            -Denable_apps=dumpcap
            -Denable_drivers=net/bonding,net/e1000,net/i40e
            -Dmachine=${CPU_TYPE}

        BUILD_ALWAYS TRUE
        BUILD_IN_SOURCE TRUE
        BUILD_COMMAND cd ${DPDK_BUILD_DIR} && meson compile

        INSTALL_COMMAND cd ${DPDK_BUILD_DIR} && meson install
)

ExternalProject_Add_Step(
        dpdk
        remove_dpdk_so_files
        COMMAND find ${DPDK_TARGET_DIR} -name *.so -delete &&
        find ${DPDK_TARGET_DIR} -name *.so.* -delete
        DEPENDEES install
)
