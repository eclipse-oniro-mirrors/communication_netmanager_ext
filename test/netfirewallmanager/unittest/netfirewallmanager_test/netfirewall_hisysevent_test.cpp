/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "netfirewall_hisysevent.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
constexpr int32_t USER_ID1 = 100;
}

class NetFirewallHisysEventTest : public testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    void SetUp(){};
    void TearDown(){};
};

HWTEST_F(NetFirewallHisysEventTest, SendRecordRequestReport001, TestSize.Level1)
{
    int32_t errorCode = FIREWALL_SUCCESS;
    NetFirewallHisysEvent::SendRecordRequestReport(USER_ID1, errorCode);
    errorCode = FIREWALL_ERR_PERMISSION_DENIED;
    NetFirewallHisysEvent::SendRecordRequestReport(USER_ID1, errorCode);
    EXPECT_EQ(errorCode, FIREWALL_ERR_PERMISSION_DENIED);
}
}
}