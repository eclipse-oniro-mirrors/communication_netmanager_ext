/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <map>
#include <mutex>
#include <set>
#include <vector>
#include <gtest/gtest.h>
#include "networkslicecommconfig.h"
#include "allowednssaiconfig.h"
#include "networksliceutil.h"
#include "urspconfig.h"
#include "nrunsolicitedmsgparser.h"
#include "networkslicemanager.h"
#include "networkslice_service.h"
#include "networkslice_client.h"
#include "hwnetworkslicemanager.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;

class HwNetworkSliceManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HwNetworkSliceManagerTest::SetUpTestCase() {}

void HwNetworkSliceManagerTest::TearDownTestCase() {}

void HwNetworkSliceManagerTest::SetUp() {}

void HwNetworkSliceManagerTest::TearDown() {}

}
}
