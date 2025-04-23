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
#include "mac_address_info.h"

#include "netmgr_ext_log_wrapper.h"
#include "parcel.h"
#include "refbase.h"

#ifdef GTEST_API_
#define private public
#define protected public
#endif

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;
}

class MacAddressInfoTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void MacAddressInfoTest::SetUpTestCase() {};

void MacAddressInfoTest::TearDownTestCase() {};

void MacAddressInfoTest::SetUp() {};

void MacAddressInfoTest::TearDown() {};

HWTEST_F(MacAddressInfoTest, MarshallingTest001, TestSize.Level1)
{
    MacAddressInfo macaddressinfo;
    Parcel parcel;
    EXPECT_TRUE(macaddressinfo.Marshalling(parcel));
}

HWTEST_F(MacAddressInfoTest, UnmarshallingTest001, TestSize.Level1)
{
    MacAddressInfo macaddressinfo;
    Parcel parcel;
    EXPECT_EQ(macaddressinfo.Unmarshalling(parcel), nullptr);
}
}
}