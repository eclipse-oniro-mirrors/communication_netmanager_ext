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

#include "sharing_event_callback_proxy.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;
class SharingEventCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    static inline auto instance_ = std::make_shared<SharingEventCallbackProxy>(nullptr);
};

void SharingEventCallbackProxyTest::SetUpTestCase() {}

void SharingEventCallbackProxyTest::TearDownTestCase() {}

void SharingEventCallbackProxyTest::SetUp() {}

void SharingEventCallbackProxyTest::TearDown() {}


HWTEST_F(SharingEventCallbackProxyTest, InterfaceTest, TestSize.Level1)
{
    bool isRunning = false;
    instance_->OnSharingStateChanged(isRunning);

    SharingIfaceType type = SharingIfaceType::SHARING_USB;
    std::string iface = "test";
    SharingIfaceState state = SharingIfaceState::SHARING_NIC_ERROR;
    instance_->OnInterfaceSharingStateChanged(type, iface, state);

    instance_->OnSharingUpstreamChanged(nullptr);
}

} // namespace NetManagerStandard
} // namespace OHOS