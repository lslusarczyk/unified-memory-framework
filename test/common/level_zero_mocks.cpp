/*
 *
 * Copyright (C) 2025 Intel Corporation
 *
 * Under the Apache License v2.0 with LLVM Exceptions. See LICENSE.TXT.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 */

#include "level_zero_mocks.h"
#include "umf/providers/provider_level_zero.h"
#include <cstdlib>

using namespace ::testing;

umf_memory_provider_handle_t
LevelZeroMock::initializeMemoryProviderWithResidentDevices(
    ze_device_handle_t device, std::vector<ze_device_handle_t> residentDevices,
    ze_context_handle_t context, ze_device_properties_t device_properties,
    ze_memory_allocation_properties_t memory_allocation_properties) {
    umf_level_zero_memory_provider_params_handle_t params = nullptr;
    EXPECT_EQ(umfLevelZeroMemoryProviderParamsCreate(&params),
              UMF_RESULT_SUCCESS);
    EXPECT_EQ(umfLevelZeroMemoryProviderParamsSetContext(params, context),
              UMF_RESULT_SUCCESS);
    EXPECT_EQ(umfLevelZeroMemoryProviderParamsSetDevice(params, device),
              UMF_RESULT_SUCCESS);
    EXPECT_EQ(umfLevelZeroMemoryProviderParamsSetMemoryType(
                  params, UMF_MEMORY_TYPE_DEVICE),
              UMF_RESULT_SUCCESS);

    EXPECT_EQ(umfLevelZeroMemoryProviderParamsSetResidentDevices(
                  params, residentDevices.data(), residentDevices.size()),
              UMF_RESULT_SUCCESS);

    // query min page size operation upon provider initialization
    EXPECT_CALL(*this, zeDeviceGetProperties(device, _))
        .WillRepeatedly(DoAll(SetArgPointee<1>(device_properties),
                              Return(ZE_RESULT_SUCCESS)));
    EXPECT_CALL(*this, zeMemAllocDevice(CONTEXT, _, _, _, device, _))
        .WillOnce(Return(ZE_RESULT_SUCCESS));
    for (auto dev : residentDevices) {
        EXPECT_CALL(*this, zeContextMakeMemoryResident(context, dev, _, _))
            .WillOnce(Return(ZE_RESULT_SUCCESS));
    }
    EXPECT_CALL(*this, zeMemGetAllocProperties(context, _, _, _))
        .WillOnce(DoAll(SetArgPointee<2>(memory_allocation_properties),
                        Return(ZE_RESULT_SUCCESS)));
    EXPECT_CALL(*this, zeMemFree(CONTEXT, _))
        .WillOnce(Return(ZE_RESULT_SUCCESS));

    umf_memory_provider_handle_t provider = nullptr;
    EXPECT_EQ(umfMemoryProviderCreate(umfLevelZeroMemoryProviderOps(), params,
                                      &provider),
              UMF_RESULT_SUCCESS);
    EXPECT_NE(provider, nullptr);

    umfLevelZeroMemoryProviderParamsDestroy(params);
    return provider;
}

ze_device_properties_t TestCreateDeviceProperties() {
    return ze_device_properties_t{ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES,
                                  nullptr,
                                  ZE_DEVICE_TYPE_GPU,
                                  0,
                                  0,
                                  0,
                                  0,
                                  0,
                                  1024,
                                  100,
                                  20,
                                  16,
                                  256,
                                  8,
                                  2,
                                  4,
                                  1,
                                  8,
                                  8,
                                  {123},
                                  "TESTGPU"};
};

ze_memory_allocation_properties_t
TestCreateMemoryAllocationProperties(uint32_t modifier) {
    return ze_memory_allocation_properties_t{
        ZE_STRUCTURE_TYPE_MEMORY_ALLOCATION_PROPERTIES, nullptr,
        ZE_MEMORY_TYPE_DEVICE, modifier, 2048};
}

void MockedLevelZeroTestEnvironment::SetUp() {
    setenv("UMF_ZE_LOADER_LIB_NAME", "libumf_ze_loopback.so", 1);
}
void MockedLevelZeroTestEnvironment::TearDown() {}
