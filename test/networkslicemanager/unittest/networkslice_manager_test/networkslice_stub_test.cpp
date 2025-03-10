/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "networkslice_stub.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {

namespace {
constexpr const char *TEST_STRING = "test";

class MockNetworkSliceStubTest : public NetworkSliceStub {
public:
    MockNetworkSliceStubTest() = default;
    ~MockNetworkSliceStubTest() override {}
    int32_t SetNetworkSliceUePolicy(std::vector<uint8_t> buffer) override
    {
        return 0;
    }
    int32_t NetworkSliceInitUePolicy() override
    {
        return 0;
    }
    int32_t NetworkSliceAllowedNssaiRpt(std::vector<uint8_t> buffer) override
    {
        return 0;
    }
    int32_t NetworkSliceEhplmnRpt(std::vector<uint8_t> buffer) override
    {
        return 0;
    }
    int32_t GetRouteSelectionDescriptorByDNN(std::string dnn, std::string& snssai, uint8_t& sscMode) override
    {
        return 0;
    }
    int32_t GetRSDByNetCap(int32_t netcap, std::map<std::string, std::string>& networkSliceParas) override
    {
        return 0;
    }
    int32_t SetSaState(bool isSaState) override
    {
        return 0;
    }
};
} // namespace


using namespace testing::ext;

class NetworkSliceStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<NetworkSliceStub> instance_ = std::make_shared<MockNetworkSliceStubTest>();
};

void NetworkSliceStubTest::SetUpTestCase() {}

void NetworkSliceStubTest::TearDownTestCase() {}

void NetworkSliceStubTest::SetUp() {}

void NetworkSliceStubTest::TearDown() {}

}
}
