/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "dns_result_callback.h"
#include "net_conn_client.h"
#include "networksliceutil.h"
#include "hwnetworkslicemanager.h"
 
namespace OHOS {
namespace NetManagerStandard {
 
using namespace testing::ext;
 
class DnsResultCallbackTest : public testing::Test {
public:
    static void SetUpTestCase() {};
    static void TearDownTestCase() {};
    sptr<DnsResultCallbackTest> GetIfaceConfig();
    void SetUp() {};
    void TearDown() {};
    static inline std::shared_ptr<DnsResultCallback> instance_ = std::make_shared<DnsResultCallback>();
};
 
HWTEST_F(DnsResultCallbackTest, HandleConnectivityChanged001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("HandleConnectivityChanged001");
    DnsResultCallback callback;
    int32_t wifiNetId;
    int32_t cellularNetId;
    callback.HandleConnectivityChanged(wifiNetId, cellularNetId);
    EXPECT_EQ(wifiNetId, NETMANAGER_SUCCESS);
}

HWTEST_F(DnsResultCallbackTest, GetDefaultNetId001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("GetDefaultNetId001");
    DnsResultCallback callback;
    int32_t ret = callback.GetDefaultNetId();
    EXPECT_EQ(ret, 0);
}

HWTEST_F(DnsResultCallbackTest, IsValidNetId001, testing::ext::TestSize.Level1)
{
    NETMGR_EXT_LOG_I("IsValidNetId001");
    DnsResultCallback callback;
    int32_t netId = 1;
    int32_t wifiNetId = 1;
    int32_t cellularNetId = 1;
    int32_t ret = callback.IsValidNetId(netId, wifiNetId, cellularNetId);
    EXPECT_EQ(ret, true);
}
 
}
}