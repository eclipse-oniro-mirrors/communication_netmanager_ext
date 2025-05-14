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
#include <arpa/inet.h>

#ifdef GTEST_API_
#define private public
#define protected public
#endif

#include "mdns_client.h"
#include "mdns_client_resume.h"
#include "mdns_common.h"
#include "discovery_callback_stub.h"
#include "registration_callback_stub.h"
#include "resolve_callback_stub.h"
#include "mock_i_discovery_callback_test.h"
#include "net_conn_client.h"
#include "netmanager_ext_test_security.h"
#include "netmgr_ext_log_wrapper.h"
#include "refbase.h"
#include "mdns_protocol_impl.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;

constexpr int DEMO_PORT = 12345;
constexpr int TIME_ONE_MS = 1;
constexpr int TIME_TWO_MS = 2;
constexpr int TIME_FOUR_MS = 4;
constexpr int TIME_FIVE_MS = 5;
constexpr uint32_t DEFAULT_LOST_MS = 20000;
constexpr const char *DEMO_NAME = "ala";
constexpr const char *DEMO_TYPE = "_hellomdns._tcp";
bool g_isScreenOn = true;
constexpr int PHASE_PTR = 1;
constexpr int PHASE_DOMAIN = 3;

static const TxtRecord g_txt{{"key", {'v', 'a', 'l', 'u', 'e'}}, {"null", {'\0'}}};

int64_t MilliSecondsSinceEpochTest()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
}

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
    int32_t HandleRegister(const MDnsServiceInfo &info, int32_t retCode) override { return 0; }
    int32_t HandleUnRegister(const MDnsServiceInfo &info, int32_t retCode) override { return 0; }
    int32_t HandleRegisterResult(const MDnsServiceInfo &info, int32_t retCode) override
    {
        g_mtx.lock();
        EXPECT_EQ(retCode, NETMANAGER_EXT_SUCCESS);
        std::cerr << "registered instance " << info.name + MDNS_DOMAIN_SPLITER_STR + info.type << "\n";
        EXPECT_EQ(expected_.name, info.name);
        EXPECT_EQ(expected_.type, info.type);
        EXPECT_EQ(expected_.port, info.port);
        g_register++;
        g_mtx.unlock();
        g_cv.notify_one();
        return NETMANAGER_EXT_SUCCESS;
    }
    MDnsServiceInfo expected_;
};

class MDnsTestDiscoveryCallback : public DiscoveryCallbackStub {
public:
    explicit MDnsTestDiscoveryCallback(const std::vector<MDnsServiceInfo> &info) : expected_(info) {}
    virtual ~MDnsTestDiscoveryCallback() = default;
    int32_t HandleStartDiscover(const MDnsServiceInfo &info, int32_t retCode) override { return 0; }
    int32_t HandleStopDiscover(const MDnsServiceInfo &info, int32_t retCode) override { return 0; }
    int32_t HandleServiceFound(const MDnsServiceInfo &info, int32_t retCode) override
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
        return NETMANAGER_EXT_SUCCESS;
    }

    int32_t HandleServiceLost(const MDnsServiceInfo &info, int32_t retCode) override
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
        return NETMANAGER_EXT_SUCCESS;
    }
    std::vector<MDnsServiceInfo> expected_;
};

class MDnsTestResolveCallback : public ResolveCallbackStub {
public:
    explicit MDnsTestResolveCallback(const MDnsServiceInfo &info) : expected_(info) {}
    virtual ~MDnsTestResolveCallback() = default;
    int32_t HandleResolveResult(const MDnsServiceInfo &info, int32_t retCode) override
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
        return NETMANAGER_EXT_SUCCESS;
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

class MDnsProtocolImplTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void MDnsProtocolImplTest::SetUpTestCase() {}

void MDnsProtocolImplTest::TearDownTestCase() {}

void MDnsProtocolImplTest::SetUp() {}

void MDnsProtocolImplTest::TearDown() {}


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

    MDnsClientResume::GetInstance().ReRegisterService();

    MDnsClientResume::GetInstance().RestartDiscoverService();
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
        {"a.b",              true,  false, false, true },
        {"_xxx.tcp",         true,  false, false, true },
        {"xxx._tcp",         true,  false, false, true },
        {"xxx.yyy",          true,  false, false, true },
        {"xxx.yyy",          true,  false, false, true },
        {"_xxx._yyy",        true,  false, false, true },
        {"hello._ipp._tcp",  true,  false, true,  true },
        {"_x._y._tcp",       true,  false, true,  true },
        {"_ipp._tcp",        true,  true,  false, true },
        {"_http._tcp",       true,  true,  false, true },
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

/**
 * @tc.name: MDnsProtocolImplCesTest001
 * @tc.desc: Test SetScreenState
 * @tc.type: FUNC
 */
HWTEST_F(MDnsProtocolImplTest, MDnsProtocolImplCesTest001, TestSize.Level1)
{
    MDnsProtocolImpl mDnsProtocolImpl;
    mDnsProtocolImpl.SubscribeCes();

    EXPECT_NE(mDnsProtocolImpl.subscriber_, nullptr);

    mDnsProtocolImpl.SubscribeCes();
    mDnsProtocolImpl.SetScreenState(true);
    mDnsProtocolImpl.SetScreenState(false);
}

/**
 * @tc.name: MDnsProtocolImplCesTest002
 * @tc.desc: Test OnReceiveEvent
 * @tc.type: FUNC
 */
HWTEST_F(MDnsProtocolImplTest, MDnsProtocolImplCesTest002, TestSize.Level1)
{
    MDnsProtocolImpl mDnsProtocolImpl;
    mDnsProtocolImpl.SubscribeCes();

    EventFwk::CommonEventData eventData;
    EventFwk::Want want;
    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_ON);
    eventData.SetWant(want);
    mDnsProtocolImpl.subscriber_->OnReceiveEvent(eventData);

    want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_SCREEN_OFF);
    eventData.SetWant(want);
    mDnsProtocolImpl.subscriber_->OnReceiveEvent(eventData);

    EXPECT_NE(mDnsProtocolImpl.subscriber_, nullptr);
}

HWTEST_F(MDnsClientTest, RegisterServiceTest001, TestSize.Level1)
{
    MDnsClient mdnsclient;
    MDnsServiceInfo serviceInfo;
    serviceInfo.name = "1234";
    serviceInfo.type = "_xyz._udp";
    serviceInfo.port = 123;
    sptr<IRegistrationCallback> cb;
    EXPECT_EQ(mdnsclient.RegisterService(serviceInfo, cb), NET_MDNS_ERR_ILLEGAL_ARGUMENT);
    serviceInfo.name = "12.34";
    EXPECT_EQ(mdnsclient.RegisterService(serviceInfo, cb), NET_MDNS_ERR_ILLEGAL_ARGUMENT);
}

HWTEST_F(MDnsClientTest, UnRegisterServiceTest001, TestSize.Level1)
{
    MDnsClient mdnsclient;
    sptr<IRegistrationCallback> cb;
    EXPECT_EQ(mdnsclient.UnRegisterService(cb), NET_MDNS_ERR_ILLEGAL_ARGUMENT);
}

HWTEST_F(MDnsClientTest, StartDiscoverServiceTest001, TestSize.Level1)
{
    std::string serviceType = "_xyz._udp";
    MDnsClient mdnsclient;
    sptr<IDiscoveryCallback> cb;
    EXPECT_EQ(mdnsclient.StartDiscoverService(serviceType, cb), NET_MDNS_ERR_ILLEGAL_ARGUMENT);
}

HWTEST_F(MDnsClientTest, ResolveServiceTest001, TestSize.Level1)
{
    MDnsClient mdnsclient;
    MDnsServiceInfo serviceInfo;
    serviceInfo.name = "1234";
    serviceInfo.type = "_xyz.udp";
    serviceInfo.port = 123;
    sptr<IResolveCallback> cb;
    EXPECT_EQ(mdnsclient.ResolveService(serviceInfo, cb), NET_MDNS_ERR_ILLEGAL_ARGUMENT);
    serviceInfo.name = "12.34";
    EXPECT_EQ(mdnsclient.ResolveService(serviceInfo, cb), NET_MDNS_ERR_ILLEGAL_ARGUMENT);
    serviceInfo.name = "1234";
    serviceInfo.type = "_xyz._udp";
    EXPECT_EQ(mdnsclient.ResolveService(serviceInfo, cb), NET_MDNS_ERR_ILLEGAL_ARGUMENT);
    serviceInfo.name = "12.34";
    EXPECT_EQ(mdnsclient.ResolveService(serviceInfo, cb), NET_MDNS_ERR_ILLEGAL_ARGUMENT);
}

HWTEST_F(MDnsProtocolImplTest, InitTest001, TestSize.Level0)
{
    MDnsProtocolImpl mDnsProtocolImpl;
    mDnsProtocolImpl.config_.configAllIface = false;
    mDnsProtocolImpl.Init();
    EXPECT_EQ(mDnsProtocolImpl.listener_.runningFlag_, true);
}

HWTEST_F(MDnsProtocolImplTest, BrowseTest001, TestSize.Level0)
{
    MDnsProtocolImpl mDnsProtocolImpl;
    mDnsProtocolImpl.lastRunTime = -1;
    g_isScreenOn = true;
    bool ret = mDnsProtocolImpl.Browse();
    EXPECT_EQ(ret, false);
    g_isScreenOn = false;
    ret = mDnsProtocolImpl.Browse();
    EXPECT_EQ(ret, false);

    mDnsProtocolImpl.lastRunTime = 1;
    g_isScreenOn = true;
    ret = mDnsProtocolImpl.Browse();
    EXPECT_EQ(ret, false);
    g_isScreenOn = false;
    ret = mDnsProtocolImpl.Browse();
    EXPECT_EQ(ret, false);
}

HWTEST_F(MDnsProtocolImplTest, ConnectControlTest001, TestSize.Level0)
{
    int32_t sockfd = 0;
    sockaddr serverAddr;
    MDnsProtocolImpl mDnsProtocolImpl;
    int32_t result = mDnsProtocolImpl.ConnectControl(sockfd, &serverAddr);
    EXPECT_EQ(result, NETMANAGER_EXT_ERR_INTERNAL);
}

HWTEST_F(MDnsProtocolImplTest, IsConnectivityTest001, TestSize.Level0)
{
    MDnsProtocolImpl mDnsProtocolImpl;
    bool result = mDnsProtocolImpl.IsConnectivity("", 1234);
    EXPECT_FALSE(result);

    result = mDnsProtocolImpl.IsConnectivity("192.168.1.1", 1234);
    EXPECT_FALSE(result);
}

HWTEST_F(MDnsProtocolImplTest, HandleOfflineServiceTest001, TestSize.Level0)
{
    MDnsProtocolImpl mDnsProtocolImpl;
    std::vector<MDnsProtocolImpl::Result> res;
    mDnsProtocolImpl.handleOfflineService("test_key", res);

    MDnsProtocolImpl::Result result;
    result.state = MDnsProtocolImpl::State::LIVE;
    result.refrehTime = mDnsProtocolImpl.lastRunTime - DEFAULT_LOST_MS + 1;
    res.push_back(result);
    mDnsProtocolImpl.handleOfflineService("test_key", res);
    EXPECT_EQ(mDnsProtocolImpl.lastRunTime, -1);
}

HWTEST_F(MDnsProtocolImplTest, RegisterAndUnregisterTest001, TestSize.Level0)
{
    MDnsProtocolImpl mDnsProtocolImpl;
    MDnsProtocolImpl::Result info;
    info.serviceName = "test_service";
    info.serviceType = "_test._tcp";
    info.port = 1234;

    std::vector<uint8_t> txtData;
    std::string txtStr = "test_txt";
    for (char c : txtStr) {
        txtData.push_back(static_cast<uint8_t>(c));
    }
    info.txt = txtData;

    EXPECT_EQ(mDnsProtocolImpl.Register(info), NETMANAGER_EXT_SUCCESS);
    std::string name = mDnsProtocolImpl.Decorated(info.serviceName + MDNS_DOMAIN_SPLITER_STR + info.serviceType);
    EXPECT_NE(mDnsProtocolImpl.srvMap_.find(name), mDnsProtocolImpl.srvMap_.end());
    EXPECT_EQ(mDnsProtocolImpl.Register(info), NET_MDNS_ERR_SERVICE_INSTANCE_DUPLICATE);

    EXPECT_EQ(mDnsProtocolImpl.UnRegister(info.serviceName + MDNS_DOMAIN_SPLITER_STR + info.serviceType),
        NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(mDnsProtocolImpl.srvMap_.find(name), mDnsProtocolImpl.srvMap_.end());
    EXPECT_EQ(mDnsProtocolImpl.UnRegister(info.serviceName), NET_MDNS_ERR_SERVICE_INSTANCE_NOT_FOUND);
}

HWTEST_F(MDnsProtocolImplTest, RegisterTest001, TestSize.Level0)
{
    MDnsProtocolImpl mDnsProtocolImpl;
    MDnsProtocolImpl::Result info;

    info.serviceName = "";
    info.serviceType = "_test._tcp";
    info.port = 1234;
    EXPECT_EQ(mDnsProtocolImpl.Register(info), NET_MDNS_ERR_ILLEGAL_ARGUMENT);

    info.serviceName = "test_service";
    info.serviceType = "_test";
    EXPECT_EQ(mDnsProtocolImpl.Register(info), NET_MDNS_ERR_ILLEGAL_ARGUMENT);

    info.serviceType = "_test._tcp";
    info.port = -1;
    EXPECT_EQ(mDnsProtocolImpl.Register(info), NET_MDNS_ERR_ILLEGAL_ARGUMENT);
}

HWTEST_F(MDnsProtocolImplTest, DiscoveryFromCacheTest001, TestSize.Level0) {
    MDnsProtocolImpl mDnsProtocolImpl;
    std::string serviceType = "_test._tcp";
    std::string topDomain = "local";
    mDnsProtocolImpl.config_.topDomain = topDomain;

    std::string decoratedName = serviceType + topDomain;
    MDnsProtocolImpl::Result result;
    result.state = MDnsProtocolImpl::State::LIVE;
    mDnsProtocolImpl.browserMap_[decoratedName].push_back(result);

    class MockIDiscoveryCallback : public sptr<IDiscoveryCallback> {
    public:
        MockIDiscoveryCallback() = default;
        ~MockIDiscoveryCallback() = default;

        void HandleServiceFound(const MDnsServiceInfo &info, int32_t code) {
            EXPECT_EQ(code, NETMANAGER_EXT_SUCCESS);
        }
    };
    MockIDiscoveryCallback mockCallback;
    bool ret = mDnsProtocolImpl.DiscoveryFromCache(serviceType, mockCallback);
    EXPECT_TRUE(ret);
    EXPECT_FALSE(mDnsProtocolImpl.browserMap_[decoratedName].empty());

    result.state = MDnsProtocolImpl::State::REMOVE;
    mDnsProtocolImpl.browserMap_[decoratedName].push_back(result);
    ret = mDnsProtocolImpl.DiscoveryFromCache(serviceType, mockCallback);
    EXPECT_TRUE(ret);

    result.state = MDnsProtocolImpl::State::DEAD;
    mDnsProtocolImpl.browserMap_[decoratedName].push_back(result);
    ret = mDnsProtocolImpl.DiscoveryFromCache(serviceType, mockCallback);
    EXPECT_TRUE(ret);
}

HWTEST_F(MDnsProtocolImplTest, DiscoveryFromCacheTest002, TestSize.Level0) {
    MDnsProtocolImpl mDnsProtocolImpl;
    std::string serviceType = "nonexistent_service";
    std::string topDomain = "local";
    mDnsProtocolImpl.config_.topDomain = topDomain;

    mDnsProtocolImpl.browserMap_.clear();
    class MockIDiscoveryCallback : public sptr<IDiscoveryCallback> {
    public:
        MockIDiscoveryCallback() = default;
        ~MockIDiscoveryCallback() = default;

        void HandleServiceFound(const MDnsServiceInfo &info, int32_t code) {
            FAIL() << "Callback should not be called";
        }
    };
    MockIDiscoveryCallback mockCallback;
    bool ret = mDnsProtocolImpl.DiscoveryFromCache(serviceType, mockCallback);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(mDnsProtocolImpl.browserMap_.empty());
}

HWTEST_F(MDnsProtocolImplTest, DiscoveryFromNetTest001, TestSize.Level0) {
    MDnsProtocolImpl mDnsProtocolImpl;
    std::string serviceType = "_test._tcp";
    std::string topDomain = "local";
    mDnsProtocolImpl.config_.topDomain = topDomain;

    mDnsProtocolImpl.browserMap_.clear();
    mDnsProtocolImpl.taskOnChange_.clear();
    mDnsProtocolImpl.cacheMap_.clear();

    class MockIDiscoveryCallback : public sptr<IDiscoveryCallback> {
    public:
        MockIDiscoveryCallback() = default;
        ~MockIDiscoveryCallback() = default;

        void HandleServiceFound(const MDnsServiceInfo &info, int32_t code) {
            EXPECT_EQ(code, NETMANAGER_EXT_SUCCESS);
        }

        void HandleServiceLost(const MDnsServiceInfo &info, int32_t code) {
            EXPECT_EQ(code, NETMANAGER_EXT_SUCCESS);
        }
    };
    MockIDiscoveryCallback mockCallback;
    bool ret = mDnsProtocolImpl.DiscoveryFromNet(serviceType, mockCallback);
    EXPECT_TRUE(ret);
    std::string decoratedName = serviceType + topDomain;
    EXPECT_FALSE(mDnsProtocolImpl.browserMap_.find(decoratedName) == mDnsProtocolImpl.browserMap_.end());
    EXPECT_FALSE(mDnsProtocolImpl.nameCbMap_.find(decoratedName) == mDnsProtocolImpl.nameCbMap_.end());
}

HWTEST_F(MDnsProtocolImplTest, ResolveInstanceFromCacheTest001, TestSize.Level0) {
    MDnsProtocolImpl mDnsProtocolImpl;
    std::string instanceName = "test_instance.local";
    std::string domain = "test_domain.local";
    std::string topDomain = "local";
    mDnsProtocolImpl.config_.topDomain = topDomain;

    class MockIResolveCallback : public sptr<IResolveCallback> {
    public:
        MockIResolveCallback() = default;
        ~MockIResolveCallback() = default;

        bool handleResolveResultCalled = false;

        void HandleResolveResult(const MDnsServiceInfo &info, int32_t code) {
            handleResolveResultCalled = true;
        }
    };
    MockIResolveCallback mockCallback;

    MDnsProtocolImpl::Result result;
    result.domain = domain;
    result.addr = "192.168.1.1";
    result.ipv6 = false;
    result.ttl = 100;
    result.refrehTime = MilliSecondsSinceEpochTest() - 50000;
    mDnsProtocolImpl.cacheMap_[instanceName] = result;
    bool ret = mDnsProtocolImpl.ResolveInstanceFromCache(instanceName, mockCallback);
    EXPECT_TRUE(ret);
    EXPECT_FALSE(mDnsProtocolImpl.taskQueue_.empty());
    auto task = mDnsProtocolImpl.taskQueue_.front();
    task();

    mockCallback.handleResolveResultCalled = false;
    mDnsProtocolImpl.taskQueue_.clear();
    mDnsProtocolImpl.cacheMap_.clear();

    result.domain = domain;
    result.addr = "";
    result.ipv6 = false;
    result.ttl = 100;
    result.refrehTime = MilliSecondsSinceEpochTest() - 50000;

    mDnsProtocolImpl.cacheMap_[instanceName] = result;
    ret = mDnsProtocolImpl.ResolveInstanceFromCache(instanceName, mockCallback);
    EXPECT_TRUE(ret);
    EXPECT_FALSE(mDnsProtocolImpl.taskOnChange_[domain].empty());
    auto eventTask = mDnsProtocolImpl.taskOnChange_[domain].front();
    eventTask();

    mockCallback.handleResolveResultCalled = false;
    mDnsProtocolImpl.taskQueue_.clear();
    mDnsProtocolImpl.taskOnChange_.clear();
    mDnsProtocolImpl.cacheMap_.clear();

    ret = mDnsProtocolImpl.ResolveInstanceFromCache(instanceName, mockCallback);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(mDnsProtocolImpl.taskQueue_.empty());
    EXPECT_TRUE(mDnsProtocolImpl.taskOnChange_.empty());
}

HWTEST_F(MDnsProtocolImplTest, ResolveFromCacheTest001, TestSize.Level0) {
    MDnsProtocolImpl mDnsProtocolImpl;
    std::string domain = "nonexistent_domain";
    mDnsProtocolImpl.browserMap_.clear();
    class MockIResolveCallback : public sptr<IResolveCallback> {
    public:
        MockIResolveCallback() = default;
        ~MockIResolveCallback() = default;

        void HandleResolveResult(const MDnsServiceInfo &info, int32_t code) {
            FAIL() << "Callback should not be called";
        }
    };
    MockIResolveCallback mockCallback;
    bool ret = mDnsProtocolImpl.ResolveFromCache(domain, mockCallback);
    EXPECT_FALSE(ret);
    EXPECT_TRUE(mDnsProtocolImpl.browserMap_.empty());
}

HWTEST_F(MDnsProtocolImplTest, ResolveFromCacheTest002, TestSize.Level0) {
    MDnsProtocolImpl mDnsProtocolImpl;
    std::string domain = "valid_domain";
    mDnsProtocolImpl.cacheMap_[domain].addr = "127.0.0.1";
    mDnsProtocolImpl.cacheMap_[domain].ttl = 1000;
    mDnsProtocolImpl.cacheMap_[domain].refrehTime = MilliSecondsSinceEpochTest() - 500;
    mDnsProtocolImpl.browserMap_.clear();
    class MockIResolveCallback : public sptr<IResolveCallback> {
    public:
        MockIResolveCallback() = default;
        ~MockIResolveCallback() = default;

        void HandleResolveResult(const MDnsServiceInfo &info, int32_t code) {
            FAIL() << "Callback should not be called";
        }
    };
    MockIResolveCallback mockCallback;
    bool ret = mDnsProtocolImpl.ResolveFromCache(domain, mockCallback);
    EXPECT_TRUE(ret);
}

HWTEST_F(MDnsProtocolImplTest, ResolveInstanceTest001, TestSize.Level0) {
    MDnsProtocolImpl mDnsProtocolImpl;
    std::string instance = "valid._instance._udp";
    class MockIResolveCallback : public sptr<IResolveCallback> {
    public:
        MockIResolveCallback() = default;
        ~MockIResolveCallback() = default;

        void HandleResolveResult(const MDnsServiceInfo &info, int32_t code) {
            FAIL() << "Callback should not be called";
        }
    };
    MockIResolveCallback mockCallback;
    int32_t ret = mDnsProtocolImpl.ResolveInstance(instance, mockCallback);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);

    instance = "";
    ret = mDnsProtocolImpl.ResolveInstance(instance, mockCallback);
    EXPECT_EQ(ret, NET_MDNS_ERR_ILLEGAL_ARGUMENT);

    instance = "invalid_domain";
    ret = mDnsProtocolImpl.ResolveInstance(instance, mockCallback);
    EXPECT_EQ(ret, NET_MDNS_ERR_ILLEGAL_ARGUMENT);
}

HWTEST_F(MDnsServerTest, MDnsCommonTest004, TestSize.Level0)
{
    std::string str = "abc";
    std::string pat = "abcd";
    EXPECT_FALSE(StartsWith(str, pat));

    std::string instance = "1.2.3";
    std::string name = "";
    std::string type = "";
    ExtractNameAndType(instance, name, type);
    EXPECT_NE(name, "");

    instance = "1.2.3.4.5";
    name = "";
    ExtractNameAndType(instance, name, type);
    EXPECT_EQ(name, "");

    instance = "1.2.3._tcp.4";
    ExtractNameAndType(instance, name, type);
    EXPECT_NE(name, "");
}

HWTEST_F(MDnsClientTest, IsKeyValueVaildTest001, TestSize.Level0)
{
    MDnsServiceInfo serviceInfo;
    std::string key = "";
    std::vector<uint8_t> value;
    EXPECT_FALSE(serviceInfo.IsKeyValueVaild(key, value));
    key = "1234567890";
    EXPECT_TRUE(serviceInfo.IsKeyValueVaild(key, value));
    key = "\tdef";
    EXPECT_FALSE(serviceInfo.IsKeyValueVaild(key, value));
    key = "\x80";
    EXPECT_FALSE(serviceInfo.IsKeyValueVaild(key, value));
    key = "=";
    EXPECT_FALSE(serviceInfo.IsKeyValueVaild(key, value));
    key = "abc";
    EXPECT_TRUE(serviceInfo.IsKeyValueVaild(key, value));
}

HWTEST_F(MDnsClientTest, GetAttrMapTest001, TestSize.Level0)
{
    MDnsServiceInfo serviceInfo;
    serviceInfo.txtRecord = {0};
    auto result = serviceInfo.GetAttrMap();
    EXPECT_TRUE(result.empty());
}

HWTEST_F(MDnsClientTest, SetAttrMapTest001, TestSize.Level0)
{
    MDnsServiceInfo serviceInfo;
    TxtRecord map;
    map["abc=def"] = {};
    serviceInfo.SetAttrMap(map);
    EXPECT_FALSE(map.empty());
}

HWTEST_F(MDnsProtocolImplTest, ReceivePacketTest, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    int sock = 0;
    MDnsPayload payload;
    mDnsProtocolImpl.ReceivePacket(sock, payload);
    payload = {0x00, 0x00, 0x00, 0x00};
    mDnsProtocolImpl.ReceivePacket(sock, payload);
    payload = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    mDnsProtocolImpl.ReceivePacket(sock, payload);
    EXPECT_EQ(mDnsProtocolImpl.lastRunTime, -1);
}

HWTEST_F(MDnsProtocolImplTest, ProcessQuestionTest, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    int sock = 0;
    MDnsMessage msg;
    mDnsProtocolImpl.ProcessQuestion(sock, msg);
    mDnsProtocolImpl.ProcessAnswer(sock, msg);
    EXPECT_EQ(mDnsProtocolImpl.lastRunTime, -1);
}

HWTEST_F(MDnsProtocolImplTest, ProcessQuestionRecordTest, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    MDnsMessage msg;
    std::string serviceName = "test_service";
    MDnsProtocolImpl::Result result;
    result.port = 1234;
    const char* txtData = "test_txt";
    size_t txtLength = strlen(txtData);
    result.txt = std::vector<unsigned char>(txtData, txtData + txtLength);
    mDnsProtocolImpl.srvMap_[serviceName] = result;

    DNSProto::Question qu;
    qu.name = "example.local";
    qu.qtype = DNSProto::RRTYPE_ANY;

    std::any anyAddr = in_addr_t{INADDR_ANY};
    DNSProto::RRType anyAddrType = DNSProto::RRTYPE_A;
    int phase = 0;
    MDnsMessage response;

    mDnsProtocolImpl.ProcessQuestionRecord(anyAddr, anyAddrType, qu, phase, response);
    EXPECT_GE(phase, PHASE_PTR);

    qu.qtype = DNSProto::RRTYPE_PTR;
    mDnsProtocolImpl.ProcessQuestionRecord(anyAddr, anyAddrType, qu, phase, response);
    EXPECT_GE(phase, PHASE_PTR);

    qu.name = "service1";
    qu.qtype = DNSProto::RRTYPE_SRV;
    mDnsProtocolImpl.ProcessQuestionRecord(anyAddr, anyAddrType, qu, phase, response);
    EXPECT_EQ(phase, 1);

    qu.name = "service1";
    qu.qtype = DNSProto::RRTYPE_TXT;
    mDnsProtocolImpl.ProcessQuestionRecord(anyAddr, anyAddrType, qu, phase, response);
    EXPECT_EQ(phase, 1);

    qu.name = mDnsProtocolImpl.GetHostDomain();
    qu.qtype = DNSProto::RRTYPE_A;
    in_addr_t ipv4Addr = INADDR_ANY;
    anyAddr = ipv4Addr;
    mDnsProtocolImpl.ProcessQuestionRecord(anyAddr, anyAddrType, qu, phase, response);
    EXPECT_GE(phase, PHASE_DOMAIN);

    qu.name = mDnsProtocolImpl.GetHostDomain();
    qu.qtype = DNSProto::RRTYPE_AAAA;
    in6_addr ipv6Addr = in6_addr();
    anyAddr = ipv6Addr;
    anyAddrType = DNSProto::RRTYPE_AAAA;
    mDnsProtocolImpl.ProcessQuestionRecord(anyAddr, anyAddrType, qu, phase, response);
    EXPECT_GE(phase, PHASE_DOMAIN);
}

HWTEST_F(MDnsProtocolImplTest, UpdatePtrTest_RdataNull, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.ttl = 100;
    rr.length = 0;
    rr.rdata = std::any();
    std::set<std::string> changed;
    mDnsProtocolImpl.UpdatePtr(false, rr, changed);
    EXPECT_EQ(mDnsProtocolImpl.browserMap_.size(), 0);
    EXPECT_EQ(changed.size(), 0);
    rr.rdata = std::string("srv.example.com");
    mDnsProtocolImpl.UpdatePtr(false, rr, changed);
    EXPECT_EQ(mDnsProtocolImpl.browserMap_.size(), 0);
    EXPECT_EQ(changed.size(), 0);
}

HWTEST_F(MDnsProtocolImplTest, UpdatePtrTest001, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.ttl = 100;
    rr.length = 0;
    rr.rdata = std::string("srv.example.com");
    std::set<std::string> changed;
    mDnsProtocolImpl.UpdatePtr(false, rr, changed);
    EXPECT_EQ(mDnsProtocolImpl.browserMap_.size(), 0);
    EXPECT_EQ(changed.size(), 0);

    rr.rdata = std::string("");
    mDnsProtocolImpl.browserMap_["test"] = std::vector<MDnsProtocolImpl::Result>();
    mDnsProtocolImpl.UpdatePtr(false, rr, changed);
    EXPECT_EQ(mDnsProtocolImpl.browserMap_["test"].size(), 0);
    EXPECT_EQ(changed.size(), 0);
}

HWTEST_F(MDnsProtocolImplTest, UpdatePtrTest002, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.ttl = 100;
    rr.length = 0;
    rr.rdata = std::string("srv.example.com");
    std::set<std::string> changed;

    mDnsProtocolImpl.browserMap_["test"] = std::vector<MDnsProtocolImpl::Result>();
    mDnsProtocolImpl.UpdatePtr(false, rr, changed);
    EXPECT_EQ(mDnsProtocolImpl.browserMap_["test"].size(), 1);
    EXPECT_EQ(changed.size(), 1);
    EXPECT_EQ(changed.count("test"), 1);

    mDnsProtocolImpl.browserMap_.clear();
    mDnsProtocolImpl.browserMap_["test1"] = std::vector<MDnsProtocolImpl::Result>
        {MDnsProtocolImpl::Result{.serviceName = "srv", .state = MDnsProtocolImpl::State::ADD}};
    mDnsProtocolImpl.UpdatePtr(false, rr, changed);
    EXPECT_EQ(mDnsProtocolImpl.browserMap_["test1"].size(), 1);
    EXPECT_EQ(changed.size(), 1);
    EXPECT_EQ(changed.count("test1"), 0);
    EXPECT_EQ(mDnsProtocolImpl.browserMap_["test1"][0].state, MDnsProtocolImpl::State::ADD);
}

HWTEST_F(MDnsProtocolImplTest, UpdatePtrTest003, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.ttl = 100;
    rr.length = 0;
    rr.rdata = std::string("srv.example.com");
    std::set<std::string> changed;
    mDnsProtocolImpl.browserMap_["test"] = std::vector<MDnsProtocolImpl::Result>
        {MDnsProtocolImpl::Result{.serviceName = "srv", .state = MDnsProtocolImpl::State::DEAD}};
    mDnsProtocolImpl.UpdatePtr(false, rr, changed);

    EXPECT_EQ(mDnsProtocolImpl.browserMap_["test"].size(), 1);
    EXPECT_EQ(changed.size(), 1);
    EXPECT_EQ(changed.count("test"), 1);
    EXPECT_EQ(mDnsProtocolImpl.browserMap_["test"][0].state, MDnsProtocolImpl::State::REFRESH);

    rr.ttl = 0;
    mDnsProtocolImpl.browserMap_["test"] = std::vector<MDnsProtocolImpl::Result>
        {MDnsProtocolImpl::Result{.serviceName = "srv", .state = MDnsProtocolImpl::State::ADD}};
    mDnsProtocolImpl.UpdatePtr(false, rr, changed);
    EXPECT_EQ(mDnsProtocolImpl.browserMap_["test"].size(), 1);
    EXPECT_EQ(changed.size(), 1);
    EXPECT_EQ(changed.count("test"), 1);
    EXPECT_EQ(mDnsProtocolImpl.browserMap_["test"][0].state, MDnsProtocolImpl::State::REMOVE);
}

HWTEST_F(MDnsProtocolImplTest, UpdateSrvTest001, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.ttl = 100;
    rr.length = 0;
    rr.rdata = std::string("invalid");
    std::set<std::string> changed;

    mDnsProtocolImpl.UpdateSrv(false, rr, changed);

    EXPECT_EQ(mDnsProtocolImpl.cacheMap_.size(), 0);
    EXPECT_EQ(changed.size(), 0);
}

HWTEST_F(MDnsProtocolImplTest, UpdateSrvTest002, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.ttl = 100;
    rr.length = 0;
    DNSProto::RDataSrv srv;
    srv.name = "srv.example.com";
    srv.port = 1234;
    rr.rdata = srv;
    std::set<std::string> changed;

    mDnsProtocolImpl.UpdateSrv(false, rr, changed);

    EXPECT_EQ(mDnsProtocolImpl.cacheMap_.size(), 1);
    EXPECT_EQ(mDnsProtocolImpl.cacheMap_["test"].state, MDnsProtocolImpl::State::ADD);
    EXPECT_EQ(changed.size(), 1);
    EXPECT_EQ(changed.count("test"), 1);
}

HWTEST_F(MDnsProtocolImplTest, UpdateSrvTest003, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.ttl = 100;
    rr.length = 0;
    DNSProto::RDataSrv srv;
    srv.name = "srv.example.com";
    srv.port = 1234;
    rr.rdata = srv;
    std::set<std::string> changed;

    mDnsProtocolImpl.cacheMap_["test"] = MDnsProtocolImpl::Result {
        .serviceName = "srv",
        .serviceType = "_srv._tcp",
        .domain = "old.example.com",
        .port = 5678,
        .state = MDnsProtocolImpl::State::LIVE
    };

    mDnsProtocolImpl.UpdateSrv(false, rr, changed);

    EXPECT_EQ(mDnsProtocolImpl.cacheMap_["test"].domain, "srv.example.com");
    EXPECT_EQ(mDnsProtocolImpl.cacheMap_["test"].port, 1234);
    EXPECT_EQ(mDnsProtocolImpl.cacheMap_["test"].state, MDnsProtocolImpl::State::REFRESH);
    EXPECT_EQ(changed.size(), 1);
    EXPECT_EQ(changed.count("test"), 1);
}

HWTEST_F(MDnsProtocolImplTest, UpdateSrvTest004, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.ttl = 0;
    rr.length = 0;
    DNSProto::RDataSrv srv;
    srv.name = "srv.example.com";
    srv.port = 1234;
    rr.rdata = srv;
    std::set<std::string> changed;

    mDnsProtocolImpl.cacheMap_["test"] = MDnsProtocolImpl::Result {
        .serviceName = "srv",
        .serviceType = "_srv._tcp",
        .domain = "srv.example.com",
        .port = 1234,
        .state = MDnsProtocolImpl::State::LIVE
    };

    mDnsProtocolImpl.UpdateSrv(false, rr, changed);

    EXPECT_EQ(mDnsProtocolImpl.cacheMap_["test"].state, MDnsProtocolImpl::State::REMOVE);
    EXPECT_EQ(changed.size(), 1);
    EXPECT_EQ(changed.count("test"), 1);
}

HWTEST_F(MDnsProtocolImplTest, UpdateSrvTest005, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.ttl = 100;
    rr.length = 0;
    DNSProto::RDataSrv srv;
    srv.name = "srv.example.com";
    srv.port = 1234;
    rr.rdata = srv;
    std::set<std::string> changed;

    mDnsProtocolImpl.cacheMap_["test"] = MDnsProtocolImpl::Result {
        .serviceName = "srv",
        .serviceType = "_srv._tcp",
        .domain = "old.example.com",
        .port = 5678,
        .state = MDnsProtocolImpl::State::DEAD
    };

    mDnsProtocolImpl.UpdateSrv(false, rr, changed);

    EXPECT_EQ(mDnsProtocolImpl.cacheMap_["test"].domain, "srv.example.com");
    EXPECT_EQ(mDnsProtocolImpl.cacheMap_["test"].port, 1234);
    EXPECT_EQ(mDnsProtocolImpl.cacheMap_["test"].state, MDnsProtocolImpl::State::REFRESH);
    EXPECT_EQ(changed.size(), 1);
    EXPECT_EQ(changed.count("test"), 1);
}

HWTEST_F(MDnsProtocolImplTest, UpdateSrvTest006, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.ttl = 100;
    rr.length = 0;
    DNSProto::RDataSrv srv;
    srv.name = "srv.example.com";
    srv.port = 1234;
    rr.rdata = srv;
    std::set<std::string> changed;

    mDnsProtocolImpl.cacheMap_["test"] = MDnsProtocolImpl::Result {
        .serviceName = "srv",
        .serviceType = "_srv._tcp",
        .domain = "srv.example.com",
        .port = 1234,
        .state = MDnsProtocolImpl::State::LIVE
    };

    mDnsProtocolImpl.UpdateSrv(false, rr, changed);

    EXPECT_EQ(mDnsProtocolImpl.cacheMap_["test"].state, MDnsProtocolImpl::State::LIVE);
    EXPECT_EQ(changed.size(), 0);
}

HWTEST_F(MDnsProtocolImplTest, UpdateTxtTest001, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.ttl = 100;
    rr.length = 0;
    rr.rdata = std::string("invalid");
    std::set<std::string> changed;

    mDnsProtocolImpl.UpdateTxt(false, rr, changed);

    EXPECT_EQ(mDnsProtocolImpl.cacheMap_.size(), 0);
    EXPECT_EQ(changed.size(), 0);
}

HWTEST_F(MDnsProtocolImplTest, UpdateTxtTest002, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.ttl = 100;
    rr.length = 0;
    TxtRecordEncoded txt;
    txt = {0x6B, 0x65, 0x79, 0x00, 0x76, 0x61, 0x6C, 0x75, 0x65};
    rr.rdata = txt;
    std::set<std::string> changed;

    mDnsProtocolImpl.UpdateTxt(false, rr, changed);

    EXPECT_EQ(mDnsProtocolImpl.cacheMap_.size(), 1);
    EXPECT_EQ(mDnsProtocolImpl.cacheMap_["test"].state, MDnsProtocolImpl::State::ADD);
    EXPECT_EQ(changed.size(), 1);
    EXPECT_EQ(changed.count("test"), 1);
}

HWTEST_F(MDnsProtocolImplTest, UpdateTxtTest003, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.ttl = 100;
    rr.length = 0;
    TxtRecordEncoded txt;
    txt = {0x6B, 0x65, 0x79, 0x00, 0x76, 0x61, 0x6C, 0x75, 0x65};
    rr.rdata = txt;
    std::set<std::string> changed;

    mDnsProtocolImpl.cacheMap_["test"] = MDnsProtocolImpl::Result{
        .serviceName = "srv",
        .serviceType = "_srv._tcp",
        .txt = {0x6F, 0x6C, 0x64, 0x5F, 0x6B, 0x65, 0x79, 0x00, 0x6F,
            0x6C, 0x64, 0x5F, 0x76, 0x61, 0x6C, 0x75, 0x65},
        .state = MDnsProtocolImpl::State::LIVE
    };

    mDnsProtocolImpl.UpdateTxt(false, rr, changed);

    EXPECT_EQ(mDnsProtocolImpl.cacheMap_["test"].txt.size(), 9);
    EXPECT_EQ(mDnsProtocolImpl.cacheMap_["test"].state, MDnsProtocolImpl::State::REFRESH);
    EXPECT_EQ(changed.size(), 1);
    EXPECT_EQ(changed.count("test"), 1);
}

HWTEST_F(MDnsProtocolImplTest, UpdateTxtTest004, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.ttl = 0;
    rr.length = 0;
    TxtRecordEncoded txt;
    txt = {0x6B, 0x65, 0x79, 0x00, 0x76, 0x61, 0x6C, 0x75, 0x65};
    rr.rdata = txt;
    std::set<std::string> changed;

    mDnsProtocolImpl.cacheMap_["test"] = MDnsProtocolImpl::Result{
        .serviceName = "srv",
        .serviceType = "_srv._tcp",
        .txt = {0x6B, 0x65, 0x79, 0x00, 0x76, 0x61, 0x6C, 0x75, 0x65},
        .state = MDnsProtocolImpl::State::LIVE
    };

    mDnsProtocolImpl.UpdateTxt(false, rr, changed);

    EXPECT_EQ(mDnsProtocolImpl.cacheMap_["test"].state, MDnsProtocolImpl::State::REMOVE);
    EXPECT_EQ(changed.size(), 1);
    EXPECT_EQ(changed.count("test"), 1);
}

HWTEST_F(MDnsProtocolImplTest, UpdateTxtTest005, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.ttl = 100;
    rr.length = 0;
    TxtRecordEncoded txt = {0x6B, 0x65, 0x79, 0x00, 0x76, 0x61, 0x6C, 0x75, 0x65};
    rr.rdata = txt;
    std::set<std::string> changed;

    mDnsProtocolImpl.cacheMap_["test"] = MDnsProtocolImpl::Result{
        .serviceName = "srv",
        .serviceType = "_srv._tcp",
        .txt = {0x6F, 0x6C, 0x64, 0x5F, 0x6B, 0x65, 0x79, 0x00, 0x6F,
            0x6C, 0x64, 0x5F, 0x76, 0x61, 0x6C, 0x75, 0x65},
        .state = MDnsProtocolImpl::State::DEAD
    };

    mDnsProtocolImpl.UpdateTxt(false, rr, changed);

    EXPECT_EQ(mDnsProtocolImpl.cacheMap_["test"].state, MDnsProtocolImpl::State::REFRESH);
    EXPECT_EQ(changed.size(), 1);
    EXPECT_EQ(changed.count("test"), 1);
}

HWTEST_F(MDnsProtocolImplTest, UpdateTxtTest006, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.ttl = 100;
    rr.length = 0;
    TxtRecordEncoded txt = {0x6B, 0x65, 0x79, 0x00, 0x76, 0x61, 0x6C, 0x75, 0x65};
    rr.rdata = txt;
    std::set<std::string> changed;

    mDnsProtocolImpl.cacheMap_["test"] = MDnsProtocolImpl::Result {
        .serviceName = "srv",
        .serviceType = "_srv._tcp",
        .txt = {0x6B, 0x65, 0x79, 0x00, 0x76, 0x61, 0x6C, 0x75, 0x65},
        .state = MDnsProtocolImpl::State::LIVE
    };

    mDnsProtocolImpl.UpdateTxt(false, rr, changed);

    EXPECT_EQ(mDnsProtocolImpl.cacheMap_["test"].state, MDnsProtocolImpl::State::LIVE);
    EXPECT_EQ(changed.size(), 0);
}

HWTEST_F(MDnsProtocolImplTest, UpdateAddrTest001, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.rtype = DNSProto::RRTYPE_A;
    rr.ttl = 100;
    rr.length = 0;
    rr.rdata = "192.168.1.1";

    std::set<std::string> changed;

    mDnsProtocolImpl.UpdateAddr(true, rr, changed);

    EXPECT_EQ(mDnsProtocolImpl.cacheMap_.size(), 0);
    EXPECT_EQ(changed.size(), 0);
}

HWTEST_F(MDnsProtocolImplTest, UpdateAddrTest002, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.rtype = DNSProto::RRTYPE_AAAA;
    rr.ttl = 100;
    rr.length = 0;
    rr.rdata = "";

    std::set<std::string> changed;

    mDnsProtocolImpl.UpdateAddr(true, rr, changed);

    EXPECT_EQ(mDnsProtocolImpl.cacheMap_.size(), 0);
    EXPECT_EQ(changed.size(), 0);
}

HWTEST_F(MDnsProtocolImplTest, UpdateAddrTest003, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "testnew";
    rr.rtype = DNSProto::RRTYPE_AAAA;
    rr.ttl = 100;
    rr.length = 0;

    in6_addr ipv6Addr;
    inet_pton(AF_INET6, "2001:0db8:85a3:0000:0000:8a2e:0370:7334", &ipv6Addr);
    rr.rdata = std::any(ipv6Addr);

    mDnsProtocolImpl.cacheMap_.clear();
    std::set<std::string> changed;

    mDnsProtocolImpl.UpdateAddr(true, rr, changed);

    EXPECT_EQ(mDnsProtocolImpl.cacheMap_.size(), 1);
    EXPECT_EQ(changed.size(), 1);
    EXPECT_NE(changed.find("testnew"), changed.end());

    const auto& result = mDnsProtocolImpl.cacheMap_.at("testnew");
    EXPECT_EQ(result.state, MDnsProtocolImpl::State::ADD);
    EXPECT_EQ(result.ipv6, true);
    EXPECT_EQ(result.addr, "2001:db8:85a3::8a2e:370:7334");
    EXPECT_EQ(result.ttl, 100);
    EXPECT_GT(result.refrehTime, 0);
}

HWTEST_F(MDnsProtocolImplTest, UpdateAddrTest004, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "testupdate";
    rr.rtype = DNSProto::RRTYPE_AAAA;
    rr.ttl = 100;
    rr.length = 0;

    in6_addr oldIpv6Addr;
    inet_pton(AF_INET6, "2001:0db8:85a3:0000:0000:8a2e:0370:7334", &oldIpv6Addr);
    rr.rdata = std::any(oldIpv6Addr);

    mDnsProtocolImpl.cacheMap_.clear();
    mDnsProtocolImpl.cacheMap_["testupdate"].addr = "2001:db8:85a3::8a2e:370:7334";
    mDnsProtocolImpl.cacheMap_["testupdate"].ipv6 = true;
    mDnsProtocolImpl.cacheMap_["testupdate"].state = MDnsProtocolImpl::State::DEAD;

    std::set<std::string> changed;
    mDnsProtocolImpl.UpdateAddr(true, rr, changed);

    EXPECT_EQ(mDnsProtocolImpl.cacheMap_.size(), 1);
    EXPECT_EQ(changed.size(), 1);
    EXPECT_NE(changed.find("testupdate"), changed.end());

    const auto& result = mDnsProtocolImpl.cacheMap_.at("testupdate");
    EXPECT_EQ(result.state, MDnsProtocolImpl::State::REFRESH);
    EXPECT_EQ(result.ipv6, true);
    EXPECT_EQ(result.addr, "2001:db8:85a3::8a2e:370:7334");
    EXPECT_EQ(result.ttl, 100);
    EXPECT_GT(result.refrehTime, 0);

    mDnsProtocolImpl.cacheMap_["testupdate"].addr = "2001:db8:85a3::8a2e:370:7334";
    mDnsProtocolImpl.cacheMap_["testupdate"].ipv6 = false;
    mDnsProtocolImpl.cacheMap_["testupdate"].state = MDnsProtocolImpl::State::ADD;
    mDnsProtocolImpl.UpdateAddr(true, rr, changed);
    EXPECT_EQ(result.ipv6, true);

    mDnsProtocolImpl.cacheMap_["testupdate"].ipv6 = false;
    mDnsProtocolImpl.cacheMap_["testupdate"].state = MDnsProtocolImpl::State::DEAD;
    mDnsProtocolImpl.UpdateAddr(true, rr, changed);
    EXPECT_EQ(result.ipv6, true);

    mDnsProtocolImpl.cacheMap_["testupdate"].addr = "2001:db8:85a3::8a2e:370:7335";
    mDnsProtocolImpl.cacheMap_["testupdate"].ipv6 = true;
    mDnsProtocolImpl.cacheMap_["testupdate"].state = MDnsProtocolImpl::State::DEAD;
    mDnsProtocolImpl.UpdateAddr(true, rr, changed);
    EXPECT_EQ(result.addr, "2001:db8:85a3::8a2e:370:7334");

    mDnsProtocolImpl.cacheMap_["testupdate"].ipv6 = false;
    mDnsProtocolImpl.UpdateAddr(true, rr, changed);
    EXPECT_EQ(result.ipv6, true);

    mDnsProtocolImpl.cacheMap_["testupdate"].ipv6 = true;
    mDnsProtocolImpl.cacheMap_["testupdate"].state = MDnsProtocolImpl::State::ADD;
    mDnsProtocolImpl.UpdateAddr(true, rr, changed);

    mDnsProtocolImpl.cacheMap_["testupdate"].ipv6 = false;
    mDnsProtocolImpl.UpdateAddr(true, rr, changed);
    EXPECT_EQ(result.ipv6, true);
}

HWTEST_F(MDnsProtocolImplTest, UpdateAddrTest005, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "testupdate";
    rr.rtype = DNSProto::RRTYPE_AAAA;
    rr.ttl = 100;
    rr.length = 0;

    in6_addr oldIpv6Addr;
    inet_pton(AF_INET6, "2001:0db8:85a3:0000:0000:8a2e:0370:7334", &oldIpv6Addr);
    rr.rdata = std::any(oldIpv6Addr);

    mDnsProtocolImpl.cacheMap_.clear();
    mDnsProtocolImpl.cacheMap_["testupdate"].addr = "2001:db8:85a3::8a2e:370:7335";
    mDnsProtocolImpl.cacheMap_["testupdate"].ipv6 = false;
    mDnsProtocolImpl.cacheMap_["testupdate"].state = MDnsProtocolImpl::State::ADD;
    std::set<std::string> changed;
    mDnsProtocolImpl.UpdateAddr(true, rr, changed);

    rr.ttl = 0;
    mDnsProtocolImpl.UpdateAddr(true, rr, changed);
    const auto& result = mDnsProtocolImpl.cacheMap_.at("testupdate");
    EXPECT_EQ(result.state, MDnsProtocolImpl::State::REMOVE);

    mDnsProtocolImpl.cacheMap_["testupdate"].state = MDnsProtocolImpl::State::LIVE;
    mDnsProtocolImpl.UpdateAddr(true, rr, changed);
}

HWTEST_F(MDnsProtocolImplTest, ProcessAnswerRecordTest001, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.rtype = DNSProto::RRTYPE_SRV;
    mDnsProtocolImpl.srvMap_["test"] = MDnsProtocolImpl::Result{};
    std::set<std::string> changed;
    mDnsProtocolImpl.ProcessAnswerRecord(false, rr, changed);
    EXPECT_TRUE(changed.empty());
}

HWTEST_F(MDnsProtocolImplTest, ProcessAnswerRecordTest002, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.rtype = DNSProto::RRTYPE_PTR;
    mDnsProtocolImpl.cacheMap_["test"] = MDnsProtocolImpl::Result{};
    std::set<std::string> changed;
    mDnsProtocolImpl.ProcessAnswerRecord(false, rr, changed);
    EXPECT_GE(changed.size(), 0);
}

HWTEST_F(MDnsProtocolImplTest, ProcessAnswerRecordTest003, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.rtype = DNSProto::RRTYPE_SRV;
    mDnsProtocolImpl.browserMap_["test"] = std::vector<MDnsProtocolImpl::Result>();
    std::set<std::string> changed;
    mDnsProtocolImpl.ProcessAnswerRecord(false, rr, changed);
    EXPECT_GE(changed.size(), 0);

    rr.rtype = DNSProto::RRTYPE_TXT;
    mDnsProtocolImpl.ProcessAnswerRecord(false, rr, changed);
    EXPECT_GE(changed.size(), 0);

    rr.rtype = DNSProto::RRTYPE_A;
    mDnsProtocolImpl.ProcessAnswerRecord(false, rr, changed);
    EXPECT_GE(changed.size(), 0);

    rr.rtype = DNSProto::RRTYPE_AAAA;
    mDnsProtocolImpl.ProcessAnswerRecord(false, rr, changed);
    EXPECT_GE(changed.size(), 0);
}

HWTEST_F(MDnsProtocolImplTest, ProcessAnswerRecordTest004, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    DNSProto::ResourceRecord rr;
    rr.name = "test";
    rr.rtype = static_cast<DNSProto::RRType>(999);
    mDnsProtocolImpl.cacheMap_["test"] = MDnsProtocolImpl::Result{};
    std::set<std::string> changed;
    mDnsProtocolImpl.ProcessAnswerRecord(false, rr, changed);
    EXPECT_TRUE(changed.empty());
}

HWTEST_F(MDnsProtocolImplTest, KillCacheTest001, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    std::string key = "test1";

    mDnsProtocolImpl.browserMap_[key].emplace_back();
    mDnsProtocolImpl.browserMap_[key].back().state = MDnsProtocolImpl::State::REMOVE;

    mDnsProtocolImpl.cacheMap_[key].state = MDnsProtocolImpl::State::REMOVE;
    mDnsProtocolImpl.cacheMap_[key].ttl = 1000;
    mDnsProtocolImpl.cacheMap_[key].refrehTime = MilliSecondsSinceEpochTest() - 500;

    mDnsProtocolImpl.KillCache(key);

    EXPECT_TRUE(mDnsProtocolImpl.browserMap_[key].empty());
    EXPECT_FALSE(mDnsProtocolImpl.cacheMap_.count(key));
}

HWTEST_F(MDnsProtocolImplTest, KillCacheTest002, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    std::string key = "test2";
    mDnsProtocolImpl.browserMap_[key].emplace_back();
    mDnsProtocolImpl.browserMap_[key].back().state = MDnsProtocolImpl::State::REMOVE;
    mDnsProtocolImpl.cacheMap_.clear();
    mDnsProtocolImpl.KillCache(key);

    EXPECT_TRUE(mDnsProtocolImpl.browserMap_[key].empty());
    EXPECT_FALSE(mDnsProtocolImpl.IsCacheAvailable(key));
}

HWTEST_F(MDnsProtocolImplTest, KillCacheTest003, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    std::string key = "test3";
    mDnsProtocolImpl.cacheMap_[key].state = MDnsProtocolImpl::State::ADD;
    mDnsProtocolImpl.cacheMap_[key].ttl = 1000;
    mDnsProtocolImpl.cacheMap_[key].refrehTime = MilliSecondsSinceEpochTest() - 500;
    mDnsProtocolImpl.KillCache(key);
    EXPECT_EQ(mDnsProtocolImpl.cacheMap_[key].state, MDnsProtocolImpl::State::LIVE);
}

HWTEST_F(MDnsProtocolImplTest, KillCacheTest004, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    std::string key = "test4";
    mDnsProtocolImpl.cacheMap_[key].state = MDnsProtocolImpl::State::REFRESH;
    mDnsProtocolImpl.cacheMap_[key].ttl = 1000;
    mDnsProtocolImpl.cacheMap_[key].refrehTime = MilliSecondsSinceEpochTest() - 500;
    mDnsProtocolImpl.KillCache(key);
    EXPECT_EQ(mDnsProtocolImpl.cacheMap_[key].state, MDnsProtocolImpl::State::LIVE);
}

HWTEST_F(MDnsProtocolImplTest, KillCacheTest005, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    std::string key = "test5";
    mDnsProtocolImpl.cacheMap_[key].state = MDnsProtocolImpl::State::ADD;
    mDnsProtocolImpl.cacheMap_[key].ttl = 1;
    mDnsProtocolImpl.cacheMap_[key].refrehTime = MilliSecondsSinceEpochTest() - 2000;
    mDnsProtocolImpl.KillCache(key);
    EXPECT_EQ(mDnsProtocolImpl.cacheMap_[key].state, MDnsProtocolImpl::State::ADD);
}

HWTEST_F(MDnsProtocolImplTest, KillCacheTest006, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    std::string key = "test6";
    mDnsProtocolImpl.cacheMap_[key].state = MDnsProtocolImpl::State::DEAD;
    mDnsProtocolImpl.cacheMap_[key].ttl = 1000;
    mDnsProtocolImpl.cacheMap_[key].refrehTime = MilliSecondsSinceEpochTest() - 500;
    mDnsProtocolImpl.KillCache(key);
    EXPECT_EQ(mDnsProtocolImpl.cacheMap_[key].state, MDnsProtocolImpl::State::DEAD);
}

HWTEST_F(MDnsProtocolImplTest, KillBrowseCacheTest001, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    std::string key = "test1";
    MDnsProtocolImpl::Result resultAdd;
    resultAdd.state = MDnsProtocolImpl::State::ADD;
    resultAdd.serviceName = "service1";
    resultAdd.serviceType = "_http._tcp";
    mDnsProtocolImpl.browserMap_[key].push_back(resultAdd);
    auto it = mDnsProtocolImpl.browserMap_[key].begin();
    mDnsProtocolImpl.KillBrowseCache(key, it);
    EXPECT_EQ(mDnsProtocolImpl.browserMap_[key].front().state, MDnsProtocolImpl::State::LIVE);
}

HWTEST_F(MDnsProtocolImplTest, KillBrowseCacheTest002, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    std::string key = "test2";
    MDnsProtocolImpl::Result resultRefresh;
    resultRefresh.state = MDnsProtocolImpl::State::REFRESH;
    resultRefresh.serviceName = "service2";
    resultRefresh.serviceType = "_http._tcp";
    mDnsProtocolImpl.browserMap_[key].push_back(resultRefresh);
    auto it = mDnsProtocolImpl.browserMap_[key].begin();
    mDnsProtocolImpl.KillBrowseCache(key, it);
    EXPECT_EQ(mDnsProtocolImpl.browserMap_[key].front().state, MDnsProtocolImpl::State::LIVE);
}

HWTEST_F(MDnsProtocolImplTest, KillBrowseCacheTest003, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    std::string key = "test3";
    MDnsProtocolImpl::Result resultRefresh;
    resultRefresh.state = MDnsProtocolImpl::State::REMOVE;
    resultRefresh.serviceName = "service3";
    resultRefresh.serviceType = "_http._tcp";
    mDnsProtocolImpl.browserMap_[key].push_back(resultRefresh);
    auto it = mDnsProtocolImpl.browserMap_[key].begin();
    mDnsProtocolImpl.KillBrowseCache(key, it);
    EXPECT_EQ(mDnsProtocolImpl.browserMap_[key].front().state, MDnsProtocolImpl::State::DEAD);
}

HWTEST_F(MDnsProtocolImplTest, KillBrowseCacheTest004, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    std::string key = "test4";
    MDnsProtocolImpl::Result resultRefresh;
    resultRefresh.state = MDnsProtocolImpl::State::DEAD;
    resultRefresh.serviceName = "service4";
    resultRefresh.serviceType = "_http._tcp";
    mDnsProtocolImpl.browserMap_[key].push_back(resultRefresh);
    auto it = mDnsProtocolImpl.browserMap_[key].begin();
    mDnsProtocolImpl.KillBrowseCache(key, it);
    EXPECT_EQ(mDnsProtocolImpl.browserMap_[key].front().state, MDnsProtocolImpl::State::DEAD);
}

HWTEST_F(MDnsProtocolImplTest, StopCbMapTest001, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    mDnsProtocolImpl.config_.topDomain = ".local";
    std::string serviceType = "_http._tcp";
    std::string name = mDnsProtocolImpl.Decorated(serviceType);
    int32_t ret = mDnsProtocolImpl.StopCbMap(serviceType);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    EXPECT_EQ(mDnsProtocolImpl.nameCbMap_.size(), 0);
    EXPECT_EQ(mDnsProtocolImpl.taskOnChange_.size(), 0);
    EXPECT_EQ(mDnsProtocolImpl.browserMap_.size(), 0);
}

HWTEST_F(MDnsProtocolImplTest, StopCbMapTest002, TestSize.Level1) {
    MDnsProtocolImpl mDnsProtocolImpl;
    mDnsProtocolImpl.config_.topDomain = ".local";
    std::string serviceType = "_http._tcp";
    std::string name = mDnsProtocolImpl.Decorated(serviceType);
    MDnsProtocolImpl::Result result1, result2;
    result1.serviceName = "service1";
    result1.serviceType = serviceType;
    result2.serviceName = "service2";
    result2.serviceType = serviceType;
    mDnsProtocolImpl.browserMap_[name].push_back(result1);
    mDnsProtocolImpl.browserMap_[name].push_back(result2);
    int32_t ret = mDnsProtocolImpl.StopCbMap(serviceType);
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
    EXPECT_EQ(mDnsProtocolImpl.browserMap_.find(name), mDnsProtocolImpl.browserMap_.end());
}
} // namespace NetManagerStandard
} // namespace OHOS