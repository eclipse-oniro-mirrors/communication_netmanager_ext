/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "net_event_report.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class NetEventReportTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void NetEventReportTest::SetUpTestCase() {}

void NetEventReportTest::TearDownTestCase() {}

void NetEventReportTest::SetUp() {}

void NetEventReportTest::TearDown() {}

HWTEST_F(NetEventReportTest, SendSetupFaultEvent, TestSize.Level1)
{
    EventInfo eventInfo = {};
    NetEventReport::SendSetupFaultEvent(eventInfo);
}

HWTEST_F(NetEventReportTest, SendCancleFaultEvent, TestSize.Level1)
{
    EventInfo eventInfo = {};
    NetEventReport::SendSetupFaultEvent(eventInfo);
}

HWTEST_F(NetEventReportTest, SendVpnConnectEvent, TestSize.Level1)
{
    VpnEventInfo eventInfo = {};
    NetEventReport::SendVpnConnectEvent(eventInfo);
}
} // namespace NetManagerStandard
} // namespace OHOS
