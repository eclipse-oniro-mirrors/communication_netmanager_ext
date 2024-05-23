/*
 * Copyright (C) 2023-2024 Huawei Device Co., Ltd.
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
#include <thread>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "mdns_client.h"
#include "mdns_client_resume.h"
#include "mdns_common.h"
#include "mdns_event_stub.h"
#include "mock_i_discovery_callback_test.h"
#include "net_conn_client.h"
#include "netmanager_ext_test_security.h"
#include "netmgr_ext_log_wrapper.h"
#include "refbase.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;

constexpr int DEMO_PORT = 12345;
constexpr int DEMO_PORT1 = 23456;
constexpr int TIME_ONE_MS = 1;
constexpr int TIME_TWO_MS = 2;
constexpr int TIME_FOUR_MS = 4;
constexpr int TIME_FIVE_MS = 5;
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
int g_register = 0;
int g_found = 0;
int g_lost = 0;
int g_resolve = 0;

class MDnsTestRegistrationCallback : public RegistrationCallbackStub {
public:
    explicit MDnsTestRegistrationCallback(const MDnsServiceInfo &info) : expected_(info) {}
    virtual ~MDnsTestRegistrationCallback() = default;
    void HandleRegister(const MDnsServiceInfo &info, int32_t retCode) override {}
    void HandleUnRegister(const MDnsServiceInfo &info, int32_t retCode) override {}
    void HandleRegisterResult(const MDnsServiceInfo &info, int32_t retCode) override
    {
        g_mtx.lock();
        std::cerr << "registered instance " << info.name + MDNS_DOMAIN_SPLITER_STR + info.type << "\n";
        EXPECT_EQ(expected_.name, info.name);
        EXPECT_EQ(expected_.type, info.type);
        EXPECT_EQ(expected_.port, info.port);
        g_register++;
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
        g_mtx.lock();
        EXPECT_EQ(retCode, NETMANAGER_EXT_SUCCESS);
        std::cerr << "found instance " << info.name + MDNS_DOMAIN_SPLITER_STR + info.type << "\n";
        EXPECT_TRUE(std::find_if(expected_.begin(), expected_.end(),
                                 [&](auto const &x) { return x.name == info.name; }) != expected_.end());
        EXPECT_TRUE(std::find_if(expected_.begin(), expected_.end(),
                                 [&](auto const &x) { return x.type == info.type; }) != expected_.end());
        g_found++;
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
        g_lost++;
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
        g_resolve++;
        g_mtx.unlock();
        g_cv.notify_one();
    }
    MDnsServiceInfo expected_;
};

class MDnsClientResumeTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void MDnsClientResumeTest::SetUpTestCase() {}

void MDnsClientResumeTest::TearDownTestCase() {}

void MDnsClientResumeTest::SetUp() {}

void MDnsClientResumeTest::TearDown() {}

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

class MDnsServerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void MDnsServerTest::SetUpTestCase() {}

void MDnsServerTest::TearDownTestCase() {}

void MDnsServerTest::SetUp() {}

void MDnsServerTest::TearDown() {}


struct MdnsClientTestParams {
    MDnsServiceInfo info;
    MDnsServiceInfo infoBack;
    sptr<MDnsTestRegistrationCallback> registration;
    sptr<MDnsTestRegistrationCallback> registrationBack;
    sptr<MDnsTestDiscoveryCallback> discovery;
    sptr<MDnsTestDiscoveryCallback> discoveryBack;
    sptr<MDnsTestResolveCallback> resolve;
    sptr<MDnsTestResolveCallback> resolveBack;
};

void DoTestForMdnsClient(MdnsClientTestParams param)
{
    NetManagerExtAccessToken token;
    bool flag = false;
    NetConnClient::GetInstance().HasDefaultNet(flag);
    if (!flag) {
        return;
    }
    std::unique_lock<std::mutex> lock(g_mtx);
    DelayedSingleton<MDnsClient>::GetInstance()->RegisterService(param.info, param.registration);
    DelayedSingleton<MDnsClient>::GetInstance()->RegisterService(param.infoBack, param.registrationBack);
    if (!g_cv.wait_for(lock, std::chrono::seconds(TIME_FIVE_MS), []() { return g_register == TIME_TWO_MS; })) {
        FAIL();
    }
    DelayedSingleton<MDnsClient>::GetInstance()->StartDiscoverService(param.info.type, param.discovery);
    DelayedSingleton<MDnsClient>::GetInstance()->StartDiscoverService(param.info.type, param.discoveryBack);
    if (!g_cv.wait_for(lock, std::chrono::seconds(TIME_FIVE_MS), []() { return g_found >= TIME_FOUR_MS; })) {
        FAIL();
    }
    DelayedSingleton<MDnsClient>::GetInstance()->ResolveService(param.info, param.resolve);
    if (!g_cv.wait_for(lock, std::chrono::seconds(TIME_FIVE_MS), []() { return g_resolve >= TIME_ONE_MS; })) {
        FAIL();
    }
    DelayedSingleton<MDnsClient>::GetInstance()->ResolveService(param.infoBack, param.resolveBack);
    if (!g_cv.wait_for(lock, std::chrono::seconds(TIME_FIVE_MS), []() { return g_resolve >= TIME_TWO_MS; })) {
        FAIL();
    }
    DelayedSingleton<MDnsClient>::GetInstance()->StopDiscoverService(param.discovery);
    DelayedSingleton<MDnsClient>::GetInstance()->StopDiscoverService(param.discoveryBack);
    if (!g_cv.wait_for(lock, std::chrono::seconds(TIME_FIVE_MS), []() { return g_lost >= TIME_ONE_MS; })) {
        FAIL();
    }
    DelayedSingleton<MDnsClient>::GetInstance()->UnRegisterService(param.registration);
    DelayedSingleton<MDnsClient>::GetInstance()->UnRegisterService(param.registrationBack);

    std::this_thread::sleep_for(std::chrono::seconds(TIME_ONE_MS));

    DelayedSingleton<MDnsClient>::GetInstance()->RestartResume();
}

HWTEST_F(MDnsClientResumeTest, ResumeTest001, TestSize.Level1)
{
    MDnsServiceInfo info;
    MDnsServiceInfo infoBack;
    info.name = DEMO_NAME;
    info.type = DEMO_TYPE;
    info.port = DEMO_PORT;
    info.SetAttrMap(g_txt);

    sptr<MDnsTestRegistrationCallback> registration(new (std::nothrow) MDnsTestRegistrationCallback(info));
    sptr<MDnsTestDiscoveryCallback> discovery(new (std::nothrow) MDnsTestDiscoveryCallback({info, infoBack}));
    ASSERT_NE(registration, nullptr);
    ASSERT_NE(discovery, nullptr);

    int32_t ret = MDnsClientResume::GetInstance().SaveRegisterService(info, registration);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = MDnsClientResume::GetInstance().SaveRegisterService(info, registration);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = MDnsClientResume::GetInstance().SaveStartDiscoverService(info.type, discovery);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = MDnsClientResume::GetInstance().SaveStartDiscoverService(info.type, discovery);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = MDnsClientResume::GetInstance().RemoveRegisterService(registration);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = MDnsClientResume::GetInstance().RemoveRegisterService(registration);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = MDnsClientResume::GetInstance().RemoveStopDiscoverService(discovery);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    ret = MDnsClientResume::GetInstance().RemoveStopDiscoverService(discovery);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    RegisterServiceMap *rsm = MDnsClientResume::GetInstance().GetRegisterServiceMap();
    ASSERT_NE(rsm, nullptr);

    DiscoverServiceMap *dsm = MDnsClientResume::GetInstance().GetStartDiscoverServiceMap();
    ASSERT_NE(dsm, nullptr);
}

/**
 * @tc.name: ServiceTest001
 * @tc.desc: Test mDNS register and found.
 * @tc.type: FUNC
 */
HWTEST_F(MDnsClientTest, ClientTest001, TestSize.Level1)
{
    MDnsServiceInfo info;
    MDnsServiceInfo infoBack;
    info.name = DEMO_NAME;
    info.type = DEMO_TYPE;
    info.port = DEMO_PORT;
    info.SetAttrMap(g_txt);
    infoBack = info;
    infoBack.name = DEMO_NAME1;
    infoBack.port = DEMO_PORT1;

    auto client = DelayedSingleton<MDnsClient>::GetInstance();
    sptr<MDnsTestRegistrationCallback> registration(new (std::nothrow) MDnsTestRegistrationCallback(info));
    sptr<MDnsTestRegistrationCallback> registrationBack(new (std::nothrow) MDnsTestRegistrationCallback(infoBack));
    sptr<MDnsTestDiscoveryCallback> discovery(new (std::nothrow) MDnsTestDiscoveryCallback({info, infoBack}));
    sptr<MDnsTestDiscoveryCallback> discoveryBack(new (std::nothrow) MDnsTestDiscoveryCallback({info, infoBack}));
    sptr<MDnsTestResolveCallback> resolve(new (std::nothrow) MDnsTestResolveCallback(info));
    sptr<MDnsTestResolveCallback> resolveBack(new (std::nothrow) MDnsTestResolveCallback(infoBack));
    ASSERT_NE(registration, nullptr);
    ASSERT_NE(registrationBack, nullptr);
    ASSERT_NE(discovery, nullptr);
    ASSERT_NE(discoveryBack, nullptr);
    ASSERT_NE(resolve, nullptr);
    ASSERT_NE(resolveBack, nullptr);

    MdnsClientTestParams mdnsClientTestParams;
    mdnsClientTestParams.info = info;
    mdnsClientTestParams.infoBack = infoBack;
    mdnsClientTestParams.registration = registration;
    mdnsClientTestParams.registrationBack = registrationBack;
    mdnsClientTestParams.discovery = discovery;
    mdnsClientTestParams.discoveryBack = discoveryBack;
    mdnsClientTestParams.resolve = resolve;
    mdnsClientTestParams.resolveBack = resolveBack;
    DoTestForMdnsClient(mdnsClientTestParams);
}

HWTEST_F(MDnsServerTest, ServerTest, TestSize.Level1)
{
    MDnsServiceInfo info;
    info.name = DEMO_NAME;
    info.type = DEMO_TYPE;
    info.port = DEMO_PORT;
    TxtRecord txt{};
    info.SetAttrMap(txt);
    info.SetAttrMap(g_txt);
    auto retMap = info.GetAttrMap();
    EXPECT_NE(retMap.size(), 0);

    MessageParcel parcel;
    auto retMar = info.Marshalling(parcel);
    EXPECT_EQ(retMar, true);

    auto serviceInfo = info.Unmarshalling(parcel);
    retMar = info.Marshalling(parcel, serviceInfo);
    EXPECT_EQ(retMar, true);
}

/**
 * @tc.name: MDnsCommonTest001
 * @tc.desc: Test MDnsServerTest
 * @tc.type: FUNC
 */
HWTEST_F(MDnsServerTest, MDnsCommonTest001, TestSize.Level1)
{
    std::string testStr = "abbcccddddcccbba";
    for (size_t i = 0; i < testStr.size(); ++i) {
    EXPECT_TRUE(EndsWith(testStr, testStr.substr(i)));
    }

    for (size_t i = 0; i < testStr.size(); ++i) {
    EXPECT_TRUE(StartsWith(testStr, testStr.substr(0, testStr.size() - i)));
    }

    auto lhs = Split(testStr, 'c');
    auto rhs = std::vector<std::string_view>{
        "abb",
        "dddd",
        "bba",
    };
    EXPECT_EQ(lhs, rhs);
}

/**
 * @tc.name: MDnsCommonTest002
 * @tc.desc: Test MDnsServerTest
 * @tc.type: FUNC
 */
HWTEST_F(MDnsServerTest, MDnsCommonTest002, TestSize.Level1)
{
    constexpr size_t isNameIndex = 1;
    constexpr size_t isTypeIndex = 2;
    constexpr size_t isInstanceIndex = 3;
    constexpr size_t isDomainIndex = 4;
    std::vector<std::tuple<std::string, bool, bool, bool, bool>> test = {
        {"abbcccddddcccbba", true,  false, false, true },
        {"",                 false, false, false, true },
        {"a.b",              false, false, false, true },
        {"_xxx.tcp",         false, false, false, true },
        {"xxx._tcp",         false, false, false, true },
        {"xxx.yyy",          false, false, false, true },
        {"xxx.yyy",          false, false, false, true },
        {"_xxx._yyy",        false, false, false, true },
        {"hello._ipp._tcp",  false, false, true,  true },
        {"_x._y._tcp",       false, false, true,  true },
        {"_ipp._tcp",        false, true,  false, true },
        {"_http._tcp",       false, true,  false, true },
    };

    for (auto line : test) {
        EXPECT_EQ(IsNameValid(std::get<0>(line)), std::get<isNameIndex>(line));
        EXPECT_EQ(IsTypeValid(std::get<0>(line)), std::get<isTypeIndex>(line));
        EXPECT_EQ(IsInstanceValid(std::get<0>(line)), std::get<isInstanceIndex>(line));
        EXPECT_EQ(IsDomainValid(std::get<0>(line)), std::get<isDomainIndex>(line));
    }

    EXPECT_TRUE(IsPortValid(22));
    EXPECT_TRUE(IsPortValid(65535));
    EXPECT_TRUE(IsPortValid(0));
    EXPECT_FALSE(IsPortValid(-1));
    EXPECT_FALSE(IsPortValid(65536));
}

/**
 * @tc.name: MDnsCommonTest003
 * @tc.desc: Test MDnsServerTest
 * @tc.type: FUNC
 */
HWTEST_F(MDnsServerTest, MDnsCommonTest003, TestSize.Level1)
{
    std::string instance = "hello._ipp._tcp";
    std::string instance1 = "_x._y._tcp";
    std::string name;
    std::string type;
    ExtractNameAndType(instance, name, type);
    EXPECT_EQ(name, "hello");
    EXPECT_EQ(type, "_ipp._tcp");

    ExtractNameAndType(instance1, name, type);
    EXPECT_EQ(name, "_x");
    EXPECT_EQ(type, "_y._tcp");
}

HWTEST_F(MDnsServerTest, MDnsServerBranchTest001, TestSize.Level1)
{
    std::string serviceType = "test";
    sptr<IDiscoveryCallback> callback = new (std::nothrow) MockIDiscoveryCallbackTest();
    EXPECT_TRUE(callback != nullptr);
    if (callback == nullptr) {
        return;
    }
    auto ret = DelayedSingleton<MDnsClient>::GetInstance()->StartDiscoverService(serviceType, callback);
    EXPECT_EQ(ret, NET_MDNS_ERR_ILLEGAL_ARGUMENT);

    ret = DelayedSingleton<MDnsClient>::GetInstance()->StopDiscoverService(callback);
    EXPECT_EQ(ret, NET_MDNS_ERR_CALLBACK_NOT_FOUND);

    callback = nullptr;
    ret = DelayedSingleton<MDnsClient>::GetInstance()->StartDiscoverService(serviceType, callback);
    EXPECT_EQ(ret, NET_MDNS_ERR_ILLEGAL_ARGUMENT);

    ret = DelayedSingleton<MDnsClient>::GetInstance()->StopDiscoverService(callback);
    EXPECT_EQ(ret, NET_MDNS_ERR_ILLEGAL_ARGUMENT);
}
} // namespace NetManagerStandard
} // namespace OHOS