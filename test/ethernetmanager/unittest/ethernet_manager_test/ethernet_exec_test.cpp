/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ethernet_exec.h"
#include "net_manager_constants.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::NetManagerStandard;
using namespace OHOS::NetManagerStandard::EthernetExec;

#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE

class MapToNapiErrorCodeTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name  : MapToNapiErrorCode_ShouldReturnPermissionDenied_WhenInputIsPermissionDenied
 * @tc.number: MapToNapiErrorCodeTest_001
 * @tc.desc  : 测试当输入为 NETMANAGER_EXT_ERR_PERMISSION_DENIED 时,函数返回 NETMANAGER_EXT_ERR_PERMISSION_DENIED
 */
HWTEST_F(MapToNapiErrorCodeTest, MapToNapiErrorCode_ShouldReturnPermissionDenied_WhenInputIsPermissionDenied,
    TestSize.Level0)
{
    int32_t result = MapToNapiErrorCode(NETMANAGER_EXT_ERR_PERMISSION_DENIED);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_PERMISSION_DENIED);
}

/**
 * @tc.name  : MapToNapiErrorCode_ShouldReturnNotSystemCall_WhenInputIsNotSystemCall
 * @tc.number: MapToNapiErrorCodeTest_002
 * @tc.desc  : 测试当输入为 NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL 时,函数返回 NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL
 */
HWTEST_F(MapToNapiErrorCodeTest, MapToNapiErrorCode_ShouldReturnNotSystemCall_WhenInputIsNotSystemCall,
    TestSize.Level0)
{
    int32_t result = MapToNapiErrorCode(NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL);
}

/**
 * @tc.name  : MapToNapiErrorCode_ShouldReturnIpcConnectStubFail_WhenInputIsIpcConnectStubFail
 * @tc.number: MapToNapiErrorCodeTest_003
 * @tc.desc  : 测试当输入为 NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL 时,函数返回 NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL
 */
HWTEST_F(MapToNapiErrorCodeTest, MapToNapiErrorCode_ShouldReturnIpcConnectStubFail_WhenInputIsIpcConnectStubFail,
    TestSize.Level0)
{
    int32_t result = MapToNapiErrorCode(NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL);
}

/**
 * @tc.name  : MapToNapiErrorCode_ShouldReturnInternal_WhenInputIsUnknown
 * @tc.number: MapToNapiErrorCodeTest_004
 * @tc.desc  : 测试当输入为未知值时,函数返回 NETMANAGER_EXT_ERR_INTERNAL
 */
HWTEST_F(MapToNapiErrorCodeTest, MapToNapiErrorCode_ShouldReturnInternal_WhenInputIsUnknown, TestSize.Level0)
{
    int32_t result = MapToNapiErrorCode(9999);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_INTERNAL);
}

class GetNapiErrorMessageTest : public ::testing::Test {
};

/**
 * @tc.name  : GetNapiErrorMessage_ShouldReturnPermissionDenied_WhenNapiCodeIsPermissionDenied
 * @tc.number: GetNapiErrorMessageTest_001
 * @tc.desc  : 测试当napiCode为NETMANAGER_EXT_ERR_PERMISSION_DENIED时,返回"Permission denied."
 */
HWTEST_F(GetNapiErrorMessageTest,
    GetNapiErrorMessage_ShouldReturnPermissionDenied_WhenNapiCodeIsPermissionDenied, TestSize.Level0)
{
    int32_t napiCode = NETMANAGER_EXT_ERR_PERMISSION_DENIED;
    std::string expectedMessage = "Permission denied.";
    EXPECT_EQ(GetNapiErrorMessage(napiCode), expectedMessage);
}

/**
 * @tc.name  : GetNapiErrorMessage_ShouldReturnNonSystemApplication_WhenNapiCodeIsNotSystemCall
 * @tc.number: GetNapiErrorMessageTest_002
 * @tc.desc  : 测试当napiCode为NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL时,返回"Non-system applications use system APIs."
 */
HWTEST_F(GetNapiErrorMessageTest, GetNapiErrorMessage_ShouldReturnNonSystemApplication_WhenNapiCodeIsNotSystemCall,
    TestSize.Level0)
{
    int32_t napiCode = NETMANAGER_EXT_ERR_NOT_SYSTEM_CALL;
    std::string expectedMessage = "Non-system applications use system APIs.";
    EXPECT_EQ(GetNapiErrorMessage(napiCode), expectedMessage);
}

/**
 * @tc.name  : GetNapiErrorMessage_ShouldReturnFailedToConnectToService_WhenNapiCodeIsIpcConnectStubFail
 * @tc.number: GetNapiErrorMessageTest_003
 * @tc.desc  : 测试当napiCode为NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL时,返回"Failed to connect to the service."
 */
HWTEST_F(GetNapiErrorMessageTest,
    GetNapiErrorMessage_ShouldReturnFailedToConnectToService_WhenNapiCodeIsIpcConnectStubFail, TestSize.Level0)
{
    int32_t napiCode = NETMANAGER_EXT_ERR_IPC_CONNECT_STUB_FAIL;
    std::string expectedMessage = "Failed to connect to the service.";
    EXPECT_EQ(GetNapiErrorMessage(napiCode), expectedMessage);
}

/**
 * @tc.name  : GetNapiErrorMessage_ShouldReturnSystemInternalError_WhenNapiCodeIsInternal
 * @tc.number: GetNapiErrorMessageTest_004
 * @tc.desc  : 测试当napiCode为NETMANAGER_EXT_ERR_INTERNAL时,返回"System internal error."
 */
HWTEST_F(GetNapiErrorMessageTest, GetNapiErrorMessage_ShouldReturnSystemInternalError_WhenNapiCodeIsInternal,
    TestSize.Level0)
{
    int32_t napiCode = NETMANAGER_EXT_ERR_INTERNAL;
    std::string expectedMessage = "System internal error.";
    EXPECT_EQ(GetNapiErrorMessage(napiCode), expectedMessage);
}

/**
 * @tc.name  : GetNapiErrorMessage_ShouldReturnUnknownError_WhenNapiCodeIsUnknown
 * @tc.number: GetNapiErrorMessageTest_005
 * @tc.desc  : 测试当napiCode为未知值时,返回"Unknown error."
 */
HWTEST_F(GetNapiErrorMessageTest, GetNapiErrorMessage_ShouldReturnUnknownError_WhenNapiCodeIsUnknown,
    TestSize.Level0)
{
    int32_t napiCode = 9999;
    std::string expectedMessage = "Unknown error.";
    EXPECT_EQ(GetNapiErrorMessage(napiCode), expectedMessage);
}

#endif // NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
