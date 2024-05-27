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

################################################################################
# Install Libfyaml

set(LIBFYAML_PREFIX_DIR "${3RD_PARTY_PREFIX_DIR}/libfyaml")
set(LIBFYAML_INCLUDE_DIR ${LIBFYAML_PREFIX_DIR}/include)
set(LIBFYAML_LIBRARY_DIR ${LIBFYAML_PREFIX_DIR}/lib)

ExternalProject_Add(libfyaml
        GIT_REPOSITORY https://github.com/pantoniou/libfyaml.git
        GIT_TAG master
        PREFIX ${3RD_PARTY_PREFIX_DIR}/libfyaml
        INSTALL_DIR ${3RD_PARTY_PREFIX_DIR}/libfyaml/install
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DBUILD_SHARED_LIBS=OFF
        )

################################################################################
# Install Libconfig

set(LIBCONFIG_PREFIX_DIR "${3RD_PARTY_PREFIX_DIR}/libconfig")
set(LIBCONFIG_INCLUDE_DIR ${LIBCONFIG_PREFIX_DIR}/include)
set(LIBCONFIG_LIBRARY_DIR ${LIBCONFIG_PREFIX_DIR}/lib)

ExternalProject_Add(libconfig
        GIT_REPOSITORY https://github.com/hyperrealm/libconfig.git
        GIT_TAG master
        PREFIX ${3RD_PARTY_PREFIX_DIR}/libconfig
        INSTALL_DIR ${3RD_PARTY_PREFIX_DIR}/libconfig/install
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DBUILD_SHARED_LIBS=OFF
        )

################################################################################
# Install CJSON

set(CJSON_PREFIX_DIR "${3RD_PARTY_PREFIX_DIR}/cjson")
set(CJSON_INCLUDE_DIR ${CJSON_PREFIX_DIR}/include)
set(CJSON_LIBRARY_DIR ${CJSON_PREFIX_DIR}/lib)

ExternalProject_Add(cjson
        GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
        GIT_TAG master
        PREFIX ${3RD_PARTY_PREFIX_DIR}/cjson
        INSTALL_DIR ${CJSON_PREFIX_DIR}/install
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR> -DBUILD_SHARED_LIBS=OFF
        )


################################################################################
# Mongoose include files
#
#set(MONGOOSE_PREFIX_DIR "${3RD_PARTY_PREFIX_DIR}/mongoose")
#
#file(DOWNLOAD "https://raw.githubusercontent.com/cesanta/mongoose/master/mongoose.c"
#        "${MONGOOSE_PREFIX_DIR}/mongoose.c")
#file(DOWNLOAD "https://raw.githubusercontent.com/cesanta/mongoose/master/mongoose.h"
#        "${MONGOOSE_PREFIX_DIR}/mongoose.h")
