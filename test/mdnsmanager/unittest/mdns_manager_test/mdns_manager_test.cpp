/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "netmgr_ext_log_wrapper.h"
#include "refbase.h"

#include "mdns_client.h"
#include "mdns_common.h"
#include "mdns_event_stub.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;

constexpr int FOUND_LIMIT = 2;
constexpr int DEMO_PORT = 12345;
constexpr const char *DEMO_NAME = "ala";
constexpr const char *DEMO_NAME1 = "ala1";
constexpr const char *DEMO_TYPE = "_hellomdns._tcp";

static const TxtRecord g_txt{{"key", {'v', 'a', 'l', 'u', 'e'}}, {"null", {'\0'}}};

enum class EventType {
    UNKNOWN,
    REGISTER,
    FOUND,
    LOST,
    RESOLVE,
};

std::mutex g_mtx;
std::condition_variable g_cv;
int g_found = 0;
EventType g_type = EventType::UNKNOWN;

class MDnsTestRegistrationCallback : public RegistrationCallbackStub {
public:
    explicit MDnsTestRegistrationCallback(const MDnsServiceInfo &info) : expected_(info) {}
    virtual ~MDnsTestRegistrationCallback() = default;
    void HandleRegister(const MDnsServiceInfo &info, int32_t retCode) override {}
    void HandleUnRegister(const MDnsServiceInfo &info, int32_t retCode) override {}
    void HandleRegisterResult(const MDnsServiceInfo &info, int32_t retCode) override
    {
        g_mtx.lock();
        EXPECT_EQ(retCode, NETMANAGER_EXT_SUCCESS);
        std::cerr << "registered instance " << info.name + MDNS_DOMAIN_SPLITER_STR + info.type << "\n";
        EXPECT_EQ(expected_.name, info.name);
        EXPECT_EQ(expected_.type, info.type);
        EXPECT_EQ(expected_.port, info.port);
        g_type = EventType::REGISTER;
        g_mtx.unlock();
        g_cv.notify_one();
    }
    MDnsServiceInfo expected_;
};

class MDnsTestDiscoveryCallback : public DiscoveryCallbackStub {
public:
    explicit MDnsTestDiscoveryCallback(const std::vector<MDnsServiceInfo> &info) : expected_(info) {}
    virtual ~MDnsTestDiscoveryCallback() = default;
    void HandleStartDiscover(const MDnsServiceInfo &info, int32_t retCode) override {}
    void HandleStopDiscover(const MDnsServiceInfo &info, int32_t retCode) override {}
    void HandleServiceFound(const MDnsServiceInfo &info, int32_t retCode) override
    {
        if (++g_found > FOUND_LIMIT) {
            return;
        }
        g_mtx.lock();
        EXPECT_EQ(retCode, NETMANAGER_EXT_SUCCESS);
        std::cerr << "found instance " << info.name + MDNS_DOMAIN_SPLITER_STR + info.type << "\n";
        EXPECT_TRUE(std::find_if(expected_.begin(), expected_.end(),
                                 [&](auto const &x) { return x.name == info.name; }) != expected_.end());
        EXPECT_TRUE(std::find_if(expected_.begin(), expected_.end(),
                                 [&](auto const &x) { return x.type == info.type; }) != expected_.end());
        g_type = EventType::FOUND;
        g_mtx.unlock();
        g_cv.notify_one();
    }

    void HandleServiceLost(const MDnsServiceInfo &info, int32_t retCode) override
    {
        g_mtx.lock();
        EXPECT_EQ(retCode, NETMANAGER_EXT_SUCCESS);
        std::cerr << "lost instance " << info.name + MDNS_DOMAIN_SPLITER_STR + info.type << "\n";
        EXPECT_TRUE(std::find_if(expected_.begin(), expected_.end(),
                                 [&](auto const &x) { return x.name == info.name; }) != expected_.end());
        EXPECT_TRUE(std::find_if(expected_.begin(), expected_.end(),
                                 [&](auto const &x) { return x.type == info.type; }) != expected_.end());
        g_type = EventType::LOST;
        g_mtx.unlock();
        g_cv.notify_one();
    }
    std::vector<MDnsServiceInfo> expected_;
};

class MDnsTestResolveCallback : public ResolveCallbackStub {
public:
    explicit MDnsTestResolveCallback(const MDnsServiceInfo &info) : expected_(info) {}
    virtual ~MDnsTestResolveCallback() = default;
    void HandleResolveResult(const MDnsServiceInfo &info, int32_t retCode) override
    {
        g_mtx.lock();
        EXPECT_EQ(retCode, NETMANAGER_EXT_SUCCESS);
        std::cerr << "resolved instance " << info.addr + MDNS_HOSTPORT_SPLITER_STR + std::to_string(info.port) << "\n";
        EXPECT_EQ(expected_.name, info.name);
        EXPECT_EQ(expected_.type, info.type);
        EXPECT_EQ(expected_.port, info.port);
        EXPECT_EQ(expected_.txtRecord, info.txtRecord);
        g_type = EventType::RESOLVE;
        g_mtx.unlock();
        g_cv.notify_one();
    }
    MDnsServiceInfo expected_;
};

class MDnsClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void MDnsClientTest::SetUpTestCase() {}

void MDnsClientTest::TearDownTestCase() {}

void MDnsClientTest::SetUp() {}

void MDnsClientTest::TearDown() {}

/**
 * @tc.name: ServiceTest001
 * @tc.desc: Test mDNS register and found.
 * @tc.type: FUNC
 */
HWTEST_F(MDnsClientTest, ClientTest001, TestSize.Level1)
{
    MDnsServiceInfo info;
    MDnsServiceInfo info1;
    info.name = DEMO_NAME;
    info.type = DEMO_TYPE;
    info.port = DEMO_PORT;
    info.SetAttrMap(g_txt);
    info1 = info;
    info1.name = DEMO_NAME1;

    auto client = DelayedSingleton<MDnsClient>::GetInstance();
    sptr<MDnsTestRegistrationCallback> registration(new (std::nothrow) MDnsTestRegistrationCallback(info));
    sptr<MDnsTestRegistrationCallback> registration1(new (std::nothrow) MDnsTestRegistrationCallback(info1));
    sptr<MDnsTestDiscoveryCallback> discovery(new (std::nothrow) MDnsTestDiscoveryCallback({info, info1}));
    sptr<MDnsTestResolveCallback> resolve(new (std::nothrow) MDnsTestResolveCallback(info));
    sptr<MDnsTestResolveCallback> resolve1(new (std::nothrow) MDnsTestResolveCallback(info1));
    ASSERT_NE(registration, nullptr);
    ASSERT_NE(registration1, nullptr);
    ASSERT_NE(discovery, nullptr);
    ASSERT_NE(resolve, nullptr);
    ASSERT_NE(resolve1, nullptr);

    std::unique_lock<std::mutex> lock(g_mtx);

    client->RegisterService(info, registration);
    g_type = EventType::UNKNOWN;
    g_cv.wait(lock, []() { return g_type == EventType::REGISTER; });

    client->RegisterService(info1, registration1);
    g_type = EventType::UNKNOWN;
    g_cv.wait(lock, []() { return g_type == EventType::REGISTER; });

    client->StartDiscoverService(info.type, discovery);
    g_type = EventType::UNKNOWN;
    g_cv.wait(lock, []() { return g_type == EventType::FOUND; });
    g_type = EventType::UNKNOWN;
    g_cv.wait(lock, []() { return g_type == EventType::FOUND; });

    client->ResolveService(info, resolve);
    g_type = EventType::UNKNOWN;
    g_cv.wait(lock, []() { return g_type == EventType::RESOLVE; });

    client->ResolveService(info1, resolve1);
    g_type = EventType::UNKNOWN;
    g_cv.wait(lock, []() { return g_type == EventType::RESOLVE; });

    client->UnRegisterService(registration);
    g_type = EventType::UNKNOWN;
    g_cv.wait(lock, []() { return g_type == EventType::LOST; });

    client->UnRegisterService(registration1);
    g_type = EventType::UNKNOWN;
    g_cv.wait(lock, []() { return g_type == EventType::LOST; });

    client->StopDiscoverService(discovery);
}

} // namespace NetManagerStandard
} // namespace OHOS