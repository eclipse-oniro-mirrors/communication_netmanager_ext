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
#ifdef GTEST_API_
#define private public
#define protected public
#endif
#include "networkshare_notification.h"
#include "ability_manager_ipc_interface_code.h"
#include "extension_manager_client.h"
#include "iservice_registry.h"
#include "message_parcel.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
} // namespace

class NetworkShareNotificationTest : public testing::Test {
public:
    void SetUp(){};
    void TearDown(){};
};

HWTEST_F(NetworkShareNotificationTest, PublishNetworkShareNotificationTest, TestSize.Level1)
{
    NetworkShareNotification::GetInstance().PublishNetworkShareNotification(
        NotificationId::HOTSPOT_IDLE_NOTIFICATION_ID);
    EXPECT_EQ(NetworkShareNotification::GetInstance().isNtfPublished, true);
}

HWTEST_F(NetworkShareNotificationTest, CancelNetworkShareNotificationTest, TestSize.Level1)
{
    NetworkShareNotification::GetInstance().CancelNetworkShareNotification(
        NotificationId::HOTSPOT_IDLE_NOTIFICATION_ID);
    EXPECT_EQ(NetworkShareNotification::GetInstance().isNtfPublished, false);
}

HWTEST_F(NetworkShareNotificationTest, StartAbilityTest001, TestSize.Level1)
{
    OHOS::AAFwk::Want want;
    auto result = NetworkShareNotification::GetInstance().StartAbility(want);
    EXPECT_TRUE(result != -1);
}
} // namespace NetManagerStandard
} // namespace OHOS