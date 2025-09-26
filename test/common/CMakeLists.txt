# Copyright (C) 2023-2025 Intel Corporation
# Under the Apache License v2.0 with LLVM Exceptions. See LICENSE.TXT.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

set(COMMON_SOURCES
    test_helpers.c
    pool_null.c
    pool_trace.c
    provider_null.c
    provider_trace.c)

if(LINUX)
    set(COMMON_SOURCES ${COMMON_SOURCES} test_helpers_linux.c)
endif(LINUX)

add_umf_library(
    NAME umf_test_common
    TYPE STATIC
    SRCS ${COMMON_SOURCES})

target_include_directories(umf_test_common
                           PRIVATE ${UMF_CMAKE_SOURCE_DIR}/include)

if(UMF_BUILD_LEVEL_ZERO_PROVIDER)

    pkg_check_modules(ZE_LOADER ze_loader)
    if(NOT ZE_LOADER_FOUND)
        find_package(ZE_LOADER REQUIRED ze_loader)
    endif()

    add_library(umf_test_mocks STATIC level_zero_mocks.cpp)

    target_link_libraries(umf_test_mocks GTest::gmock umf_utils)

    target_include_directories(
        umf_test_mocks
        PRIVATE ${UMF_CMAKE_SOURCE_DIR}/include ${UMF_CMAKE_SOURCE_DIR}/src
                ${UMF_CMAKE_SOURCE_DIR}/src/utils ${ZE_LOADER_INCLUDE_DIRS})

    add_library(umf_ze_loopback SHARED ze_loopback.h ze_loopback.cpp)
    target_include_directories(
        umf_ze_loopback PRIVATE ${ZE_LOADER_INCLUDE_DIRS} ${UMF_UTILS_DIR}
                                ${UMF_SRC_DIR} ${UMF_CMAKE_SOURCE_DIR}/include)
endif()
