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
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <filesystem>
#include <thread>
#include <unistd.h>

#include "eth_eap_profile.h"
#include "eap_hdi_wpa_manager.h"
#include "netmgr_ext_log_wrapper.h"
#include "net_manager_constants.h"

#ifdef GTEST_API_
#define private public
#define protected public
#endif

namespace OHOS {
namespace NetManagerStandard {
namespace {
using namespace testing::ext;

constexpr int32_t ETH_EAP_FAKE_BLOCK_MAX_MS = 30000;
constexpr int32_t ETH_EAP_FAKE_BLOCK_POLL_MS = 10;

// HDI 假桩：struct IEthernet 是函数指针表；IEthernetReleaseInstance 按 EthernetProxy
// 布局取 {impl, remote} 前缀，remote 置空时回收为空操作，堆分配可用 free 安全释放
struct EthernetHdiFake {
    struct IEthernet impl;
    struct HdfRemoteService *remote;
};

std::atomic<int32_t> g_fakeUnregisterRet = HDF_SUCCESS;
std::atomic<int32_t> g_fakeStopRet = HDF_SUCCESS;
std::atomic<int> g_fakeUnregisterCalls = 0;
std::atomic<int> g_fakeStopCalls = 0;
std::atomic<int> g_fakeStopReturned = 0;
std::atomic<bool> g_fakeStopBlock = false;

int32_t FakeUnregisterEapEventCallback(struct IEthernet *self, struct IEthernetCallback *cbFunc,
                                       const char *ifName)
{
    g_fakeUnregisterCalls.fetch_add(1);
    return g_fakeUnregisterRet.load();
}

int32_t FakeStopEap(struct IEthernet *self, const char *ifName)
{
    g_fakeStopCalls.fetch_add(1);
    // 模拟 ethernet_host 卡死时 hdicall 挂起；上限 30s 防止用例异常时线程悬挂
    for (int32_t waited = 0; g_fakeStopBlock.load() && waited < ETH_EAP_FAKE_BLOCK_MAX_MS;
         waited += ETH_EAP_FAKE_BLOCK_POLL_MS) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ETH_EAP_FAKE_BLOCK_POLL_MS));
    }
    g_fakeStopReturned.fetch_add(1);
    return g_fakeStopRet.load();
}

EthernetHdiFake *CreateEthernetHdiFake()
{
    auto *fake = static_cast<EthernetHdiFake *>(malloc(sizeof(EthernetHdiFake)));
    if (fake == nullptr) {
        return nullptr;
    }
    memset(&fake->impl, 0, sizeof(fake->impl));
    fake->impl.UnregisterEapEventCallback = FakeUnregisterEapEventCallback;
    fake->impl.StopEap = FakeStopEap;
    fake->remote = nullptr;
    return fake;
}

void ResetEthernetHdiFake()
{
    g_fakeUnregisterRet = HDF_SUCCESS;
    g_fakeStopRet = HDF_SUCCESS;
    g_fakeUnregisterCalls = 0;
    g_fakeStopCalls = 0;
    g_fakeStopReturned = 0;
    g_fakeStopBlock = false;
}

bool WaitFakeStopReturned(int32_t timeoutMs)
{
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    while (std::chrono::steady_clock::now() < deadline) {
        if (g_fakeStopReturned.load() >= 1) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    return g_fakeStopReturned.load() >= 1;
}
}
 
static constexpr const char* CONFIG_PATH = "/data/service/el1/public/eth/";
static constexpr const char* ETH_NAME = "eth0";
 
class EapHdiWpaManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
 
void EapHdiWpaManagerTest::SetUpTestCase() {};
 
void EapHdiWpaManagerTest::TearDownTestCase() {};
 
void EapHdiWpaManagerTest::SetUp() {};
 
void EapHdiWpaManagerTest::TearDown() {};
 
HWTEST_F(EapHdiWpaManagerTest, LoadEthernetHdiServiceTest001, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    EXPECT_EQ(manager_->LoadEthernetHdiService(), NETMANAGER_EXT_SUCCESS);
    manager_->iEthernet_ = nullptr;
    EXPECT_NE(manager_->LoadEthernetHdiService(), NETMANAGER_EXT_SUCCESS);
    manager_->devMgr_ = nullptr;
    EXPECT_NE(manager_->LoadEthernetHdiService(), NETMANAGER_EXT_SUCCESS);
    EXPECT_NE(manager_->LoadEthernetHdiService(), NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(EapHdiWpaManagerTest, StartEapTest001, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    std::string ifName = "eth0";
    EthEapProfile profile;
    manager_->LoadEthernetHdiService();
    EXPECT_NE(manager_->StartEap(ifName, profile), NETMANAGER_EXT_SUCCESS);
    manager_->iEthernet_ = nullptr;
    EXPECT_NE(manager_->StartEap(ifName, profile), NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(EapHdiWpaManagerTest, StopEapTest001, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    std::string ifName = "eth0";
    manager_->LoadEthernetHdiService();
    EXPECT_NE(manager_->StopEap(ifName), NETMANAGER_EXT_SUCCESS);
    manager_->devMgr_ = nullptr;
    EXPECT_NE(manager_->StopEap(ifName), NETMANAGER_EXT_SUCCESS);
    manager_->iEthernet_ = nullptr;
    EXPECT_NE(manager_->StopEap(ifName), NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name: StopEapHdiCallSuccessTest002
 * @tc.desc: hdicall 正常成功路径：假桩立即返回成功，限时守卫不影响正常登出时序
 * @tc.type: FUNC
 */
HWTEST_F(EapHdiWpaManagerTest, StopEapHdiCallSuccessTest002, TestSize.Level1)
{
    ResetEthernetHdiFake();
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    EthernetHdiFake *fake = CreateEthernetHdiFake();
    ASSERT_NE(fake, nullptr);
    manager_->devMgr_ = nullptr;
    manager_->iEthernet_ = &fake->impl;

    auto begin = std::chrono::steady_clock::now();
    EXPECT_EQ(manager_->StopEap(ETH_NAME), EAP_ERRCODE_SUCCESS);
    int64_t elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - begin).count();
    EXPECT_LT(elapsedMs, 1000); // 正常路径不应触发限时守卫
    EXPECT_EQ(g_fakeUnregisterCalls.load(), 1);
    EXPECT_EQ(g_fakeStopCalls.load(), 1);
    EXPECT_EQ(manager_->iEthernet_, nullptr); // 成功后释放实例
}

/**
 * @tc.name: StopEapHdiCallFailTest003
 * @tc.desc: hdicall 立即失败路径：返回 LOGOFF_FAIL 并释放实例
 * @tc.type: FUNC
 */
HWTEST_F(EapHdiWpaManagerTest, StopEapHdiCallFailTest003, TestSize.Level1)
{
    ResetEthernetHdiFake();
    g_fakeStopRet = HDF_FAILURE;
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    EthernetHdiFake *fake = CreateEthernetHdiFake();
    ASSERT_NE(fake, nullptr);
    manager_->devMgr_ = nullptr;
    manager_->iEthernet_ = &fake->impl;

    EXPECT_EQ(manager_->StopEap(ETH_NAME), EAP_ERRCODE_LOGOFF_FAIL);
    EXPECT_EQ(g_fakeStopCalls.load(), 1);
    EXPECT_EQ(manager_->iEthernet_, nullptr);
}

/**
 * @tc.name: StopEapHdiCallTimeoutRecoverTest004
 * @tc.desc: hdicall 挂起触发限时守卫强杀后，在第二个等待周期内恢复：返回 LOGOFF_FAIL 并释放实例
 * @tc.type: FUNC
 */
HWTEST_F(EapHdiWpaManagerTest, StopEapHdiCallTimeoutRecoverTest004, TestSize.Level1)
{
    ResetEthernetHdiFake();
    g_fakeStopRet = HDF_FAILURE;
    g_fakeStopBlock = true;
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    EthernetHdiFake *fake = CreateEthernetHdiFake();
    ASSERT_NE(fake, nullptr);
    manager_->devMgr_ = nullptr;
    manager_->iEthernet_ = &fake->impl;
    // 3.5s 后解除挂起：覆盖超时强杀分支与恢复收尾
    std::thread releaser([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(3500));
        g_fakeStopBlock = false;
    });

    auto begin = std::chrono::steady_clock::now();
    EXPECT_EQ(manager_->StopEap(ETH_NAME), EAP_ERRCODE_LOGOFF_FAIL);
    int64_t elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - begin).count();
    EXPECT_GE(elapsedMs, 3000); // 必然经过限时守卫超时
    releaser.join();
    EXPECT_TRUE(WaitFakeStopReturned(2000));
    EXPECT_EQ(manager_->iEthernet_, nullptr);
}

/**
 * @tc.name: StopEapHdiCallGiveUpTest005
 * @tc.desc: hdicall 挂起超过总等待上限：放弃等待返回 LOGOFF_FAIL，成员实例立即置空，
 *           释放职责原子移交给 hdicall 线程（返回后自行释放，不泄漏、不使用中释放）
 * @tc.type: FUNC
 */
HWTEST_F(EapHdiWpaManagerTest, StopEapHdiCallGiveUpTest005, TestSize.Level1)
{
    ResetEthernetHdiFake();
    g_fakeStopRet = HDF_FAILURE;
    g_fakeStopBlock = true;
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    EthernetHdiFake *fake = CreateEthernetHdiFake();
    ASSERT_NE(fake, nullptr);
    manager_->devMgr_ = nullptr;
    manager_->iEthernet_ = &fake->impl;
    // 9s 后才解除挂起：远超总等待上限，StopEap 必须先行放弃等待并返回
    std::thread releaser([]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(9000));
        g_fakeStopBlock = false;
    });

    auto begin = std::chrono::steady_clock::now();
    EXPECT_EQ(manager_->StopEap(ETH_NAME), EAP_ERRCODE_LOGOFF_FAIL);
    int64_t elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - begin).count();
    EXPECT_GE(elapsedMs, 5900);
    EXPECT_LT(elapsedMs, 8500);
    // 放弃路径特征：成员指针立即置空，不留死 proxy 供下次 LoadEthernetHdiService 覆盖泄漏
    EXPECT_EQ(manager_->iEthernet_, nullptr);
    releaser.join();
    ASSERT_TRUE(WaitFakeStopReturned(2000));
    // 实例由 hdicall 线程在返回后自行释放，无需手动回收
}
 
HWTEST_F(EapHdiWpaManagerTest, SetEapConfigTest001, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    EthEapProfile profile;
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
 
    profile.eapMethod = EapMethod::EAP_PEAP;
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.eapMethod = EapMethod::EAP_TTLS;
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.eapMethod = EapMethod::EAP_TLS;
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.eapMethod = EapMethod::EAP_PWD;
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.eapMethod = EapMethod::EAP_NONE;
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(EapHdiWpaManagerTest, SetEapConfigTest002, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    EthEapProfile profile;
    profile.identity = "identity";
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.password.append("password", strlen("password"));
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.eapMethod = EapMethod::EAP_PEAP;
    profile.caPath = "";
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.caPath = "caPath";
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.phase2Method = Phase2Method::PHASE2_NONE;
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.phase2Method = Phase2Method::PHASE2_PAP;
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.eapMethod = EapMethod::EAP_TLS;
    profile.caPath = "";
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.caPath = "caPath";
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.clientCertAliases = "";
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.clientCertAliases = "clientCertAliases";
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.certPassword.clear();
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.certPassword.append("certPassword", strlen("certPassword"));
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(EapHdiWpaManagerTest, SetEapConfigTest003, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    EthEapProfile profile;
    profile.eapMethod = EapMethod::EAP_UNAUTH_TLS;
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.caCertAliases = "caCertAliases";
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.altSubjectMatch = "altSubjectMatch";
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.domainSuffixMatch = "domainSuffixMatch";
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.eapMethod = EapMethod::EAP_SIM;
    profile.realm = "realm";
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.eapMethod = EapMethod::EAP_AKA;
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.eapMethod = EapMethod::EAP_AKA_PRIME;
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(EapHdiWpaManagerTest, SetEapConfigTest004, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    EthEapProfile profile;
    profile.eapMethod = EapMethod::EAP_PEAP;
    profile.anonymousIdentity = "anonymousIdentity";
    profile.caCertAliases = "caCertAliases";
    profile.altSubjectMatch = "altSubjectMatch";
    profile.domainSuffixMatch = "domainSuffixMatch";
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    profile.eapMethod = EapMethod::EAP_TTLS;
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(EapHdiWpaManagerTest, SetEapConfigTest005, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    EthEapProfile profile;
    profile.eapMethod = static_cast<EapMethod>(100);
    EXPECT_EQ(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(EapHdiWpaManagerTest, SetEapConfigTest006, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    EthEapProfile profile;
    profile.eapMethod = EapMethod::EAP_PEAP;
    std::string configPath = std::string(CONFIG_PATH) + "eth_wpa_supplicant.conf";
    std::remove(configPath.c_str());
    symlink("/nonexistent_target", configPath.c_str());
    EXPECT_NE(manager_->SetEapConfig(profile), NETMANAGER_EXT_SUCCESS);
    std::remove(configPath.c_str());
}
 
HWTEST_F(EapHdiWpaManagerTest, AppendIfNotEmptyTest001, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    manager_->setNetCmd_.clear();
    manager_->AppendIfNotEmpty("key=", "");
    EXPECT_TRUE(manager_->setNetCmd_.empty());
    manager_->AppendIfNotEmpty("key=", "value");
    EXPECT_FALSE(manager_->setNetCmd_.empty());
}
 
HWTEST_F(EapHdiWpaManagerTest, AppendCommonTlsParamsTest001, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    EthEapProfile profile;
    manager_->setNetCmd_.clear();
    manager_->AppendCommonTlsParams(profile);
    EXPECT_TRUE(manager_->setNetCmd_.empty());
    profile.caPath = "caPath";
    manager_->AppendCommonTlsParams(profile);
    EXPECT_FALSE(manager_->setNetCmd_.empty());
    profile.caCertAliases = "caCertAliases";
    manager_->setNetCmd_.clear();
    manager_->AppendCommonTlsParams(profile);
    EXPECT_FALSE(manager_->setNetCmd_.empty());
    profile.altSubjectMatch = "altSubjectMatch";
    profile.domainSuffixMatch = "domainSuffixMatch";
    manager_->AppendCommonTlsParams(profile);
    EXPECT_FALSE(manager_->setNetCmd_.empty());
}

HWTEST_F(EapHdiWpaManagerTest, AppendIfNotEmptyTest002, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    manager_->setNetCmd_.clear();
    manager_->AppendIfNotEmpty("key=", "value\"with\"quotes");
    EXPECT_FALSE(manager_->setNetCmd_.empty());
    manager_->setNetCmd_.clear();
    manager_->AppendIfNotEmpty("key=", "value\nwith\nnewline");
    EXPECT_FALSE(manager_->setNetCmd_.empty());
    manager_->setNetCmd_.clear();
    manager_->AppendIfNotEmpty("key=", "value\rwith\rcr");
    EXPECT_FALSE(manager_->setNetCmd_.empty());
}

HWTEST_F(EapHdiWpaManagerTest, Phase2MethodToStrTest002, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    EXPECT_EQ(manager_->Phase2MethodToStr(EapMethod::EAP_PEAP, Phase2Method::PHASE2_AKA_PRIME), "auth=AKA'");
    EXPECT_EQ(manager_->Phase2MethodToStr(EapMethod::EAP_PEAP, Phase2Method::PHASE2_AKA), "auth=AKA");
    EXPECT_EQ(manager_->Phase2MethodToStr(EapMethod::EAP_TTLS, Phase2Method::PHASE2_GTC), "autheap=GTC");
}

HWTEST_F(EapHdiWpaManagerTest, Phase2MethodToStrTest003, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    EXPECT_EQ(manager_->Phase2MethodToStr(EapMethod::EAP_PEAP, static_cast<Phase2Method>(100)), "auth=NONE");
    EXPECT_EQ(manager_->Phase2MethodToStr(EapMethod::EAP_TTLS, static_cast<Phase2Method>(100)), "auth=NONE");
}
 
HWTEST_F(EapHdiWpaManagerTest, RemoveHistoryCtrl001, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    std::string destPath = std::string(CONFIG_PATH) + "a.txt";
    std::ofstream file;
    file.open(destPath, std::ios::out);
    if (file.is_open()) {
        file << "test" << std::endl;
        file.close();
    }
    manager_->RemoveHistoryCtrl();
    EXPECT_EQ(manager_->iEthernet_, nullptr);
}
 
HWTEST_F(EapHdiWpaManagerTest, EapShellCmdTest001, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    std::string ifName = "eth0";
    std::string cmd = "EAP_AUTH 2:1:1";
    manager_->LoadEthernetHdiService();
    EXPECT_NE(manager_->EapShellCmd(ifName, cmd), NETMANAGER_EXT_SUCCESS);
    manager_->iEthernet_ = nullptr;
    EXPECT_NE(manager_->EapShellCmd(ifName, cmd), NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(EapHdiWpaManagerTest, RegisterEapEventCallbackTest001, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    std::string ifName = "eth0";
    manager_->LoadEthernetHdiService();
    EXPECT_NE(manager_->RegisterEapEventCallback(ifName), NETMANAGER_EXT_SUCCESS);
    manager_->iEthernet_ = nullptr;
    EXPECT_NE(manager_->RegisterEapEventCallback(ifName), NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(EapHdiWpaManagerTest, UnregisterEapEventCallbackTest001, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    std::string ifName = "eth0";
    manager_->LoadEthernetHdiService();
    EXPECT_NE(manager_->UnregisterEapEventCallback(ifName), NETMANAGER_EXT_SUCCESS);
    manager_->iEthernet_ = nullptr;
    EXPECT_NE(manager_->UnregisterEapEventCallback(ifName), NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(EapHdiWpaManagerTest, Phase2MethodToStrTest001, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    EapMethod eap = EapMethod::EAP_NONE;
    Phase2Method method = Phase2Method::PHASE2_GTC;
    std::string resultStr = "auth=NONE";
    EXPECT_NE(manager_->Phase2MethodToStr(eap, method), resultStr);
    eap = EapMethod::EAP_NONE;
    method = Phase2Method::PHASE2_GTC;
    EXPECT_NE(manager_->Phase2MethodToStr(eap, method), resultStr);
    eap = EapMethod::EAP_TTLS;
    method = Phase2Method::PHASE2_GTC;
    EXPECT_NE(manager_->Phase2MethodToStr(eap, method), resultStr);
    eap = EapMethod::EAP_TTLS;
    method = Phase2Method::PHASE2_NONE;
    EXPECT_EQ(manager_->Phase2MethodToStr(eap, method), resultStr);
    eap = EapMethod::EAP_TTLS;
    method = Phase2Method::PHASE2_GTC;
    EXPECT_NE(manager_->Phase2MethodToStr(eap, method), resultStr);
    eap = EapMethod::EAP_NONE;
    method = Phase2Method::PHASE2_NONE;
    EXPECT_EQ(manager_->Phase2MethodToStr(eap, method), resultStr);
}
 
HWTEST_F(EapHdiWpaManagerTest, WriteEapConfigToFile001, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    std::string context = "test";
    EXPECT_TRUE(manager_->WriteEapConfigToFile(context));
    context = "test2";
    EXPECT_TRUE(manager_->WriteEapConfigToFile(context));
}

HWTEST_F(EapHdiWpaManagerTest, WriteEapConfigToFile002, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    std::string configPath = std::string(CONFIG_PATH) + "eth_wpa_supplicant.conf";
    std::remove(configPath.c_str());
    symlink("/nonexistent_target", configPath.c_str());
    EXPECT_FALSE(manager_->WriteEapConfigToFile("test"));
    std::remove(configPath.c_str());
}
 
HWTEST_F(EapHdiWpaManagerTest, WriteEapConfigToFile003, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    std::error_code ec;
    std::filesystem::remove_all(CONFIG_PATH, ec);
    EXPECT_FALSE(manager_->WriteEapConfigToFile("test"));
    std::filesystem::create_directories(CONFIG_PATH, ec);
}
 
HWTEST_F(EapHdiWpaManagerTest, OnEapEventReport001, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    const char *value = "1:1:1:3:111:111";
    const char *ifName = "eth0";
    EXPECT_NE(manager_->OnEapEventReport(nullptr, ifName, value), NETMANAGER_EXT_SUCCESS);
    value = "1:1:1:3";
    EXPECT_NE(manager_->OnEapEventReport(nullptr, ifName, value), NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(EapHdiWpaManagerTest, OnEapEventReport002, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    const char *ifName = "eth0";
    const char *authValue = "802.1 Authentication Complete";  // AUTHENTICATION_RES ctrl 事件
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
    // PC 形态：ctrl 事件被 HandleEthCtrlEventOnEth0 拦截，直接返回成功
    EXPECT_EQ(manager_->OnEapEventReport(nullptr, ifName, authValue), NETMANAGER_SUCCESS);
#else
    // phone 形态：ctrl 事件拦截未编译，按普通报文解析因段数不符返回内部错误
    EXPECT_EQ(manager_->OnEapEventReport(nullptr, ifName, authValue), EAP_ERRCODE_INTERNAL_ERROR);
#endif
}

HWTEST_F(EapHdiWpaManagerTest, OnEapEventReport003, TestSize.Level1)
{
    // Supplemental regression: malformed ctrl events must not report authentication success.
    EXPECT_EQ(EapHdiWpaManager::OnEapEventReport(nullptr, "eth0", "invalid"), EAP_ERRCODE_INTERNAL_ERROR);
}

HWTEST_F(EapHdiWpaManagerTest, ConvertStrToInt001, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    std::string value = "1:1:1:3:111";
    int32_t res;
    EXPECT_FALSE(manager_->ConvertStrToInt(value, res));
    value = "10";
    EXPECT_TRUE(manager_->ConvertStrToInt(value, res));
}
 
HWTEST_F(EapHdiWpaManagerTest, RegisterCustomEapCallbackTest001, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    std::string ifName = "eth0";
    std::string regCmd = "2:277:278";
    manager_->LoadEthernetHdiService();
    EXPECT_NE(manager_->RegisterCustomEapCallback(ifName, regCmd), NETMANAGER_EXT_SUCCESS);
    manager_->iEthernet_ = nullptr;
    EXPECT_NE(manager_->RegisterCustomEapCallback(ifName, regCmd), NETMANAGER_EXT_SUCCESS);
}
 
HWTEST_F(EapHdiWpaManagerTest, ReplyCustomEapDataTest001, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    std::string ifName = "eth0";
    int32_t result = 1;
    std::string regCmd = "2:277:278";
    sptr<EapData> eapData =new (std::nothrow) EapData();
    eapData->eapCode = 1;
    eapData->eapType = 13;
    eapData->msgId = 55;
    eapData->bufferLen = 4;
    std::vector<uint8_t> tmp = {0x11, 0x12};
    eapData->eapBuffer = tmp;
    manager_->LoadEthernetHdiService();
    EXPECT_NE(manager_->ReplyCustomEapData(ifName, result, eapData), NETMANAGER_EXT_SUCCESS);
    manager_->iEthernet_ = nullptr;
    EXPECT_NE(manager_->ReplyCustomEapData(ifName, result, eapData), NETMANAGER_EXT_SUCCESS);
}
 
/**
 * @tc.name: AppendCommonTlsParamsTest002
 * @tc.desc: Test AppendCommonTlsParams keyUri skip branch (empty caPath + "oh:" prefix alias).
 * @tc.type: FUNC
 */
HWTEST_F(EapHdiWpaManagerTest, AppendCommonTlsParamsTest002, TestSize.Level1)
{
    std::shared_ptr<EapHdiWpaManager> manager_ = std::make_shared<EapHdiWpaManager>();
    EthEapProfile profile;
    // caPath 为空且证书别名为 keyUri（oh: 前缀）：跳过 ca_cert 写入
    profile.caCertAliases = "oh:ak=TestKeyUri";
    manager_->AppendCommonTlsParams(profile);
    EXPECT_TRUE(manager_->setNetCmd_.empty());
    profile.altSubjectMatch = "altSubjectMatch";
    manager_->AppendCommonTlsParams(profile);
    EXPECT_FALSE(manager_->setNetCmd_.empty());
}

/**
 * @tc.name: ParseEapEventDataTest001
 * @tc.desc: Test ParseEapEventData with non-numeric msgId.
 * @tc.type: FUNC
 */
HWTEST_F(EapHdiWpaManagerTest, ParseEapEventDataTest001, TestSize.Level1)
{
    sptr<EapData> eapData = new (std::nothrow) EapData();
    // msgId 非数字：字符串转整数失败
    std::vector<std::string> vec = {"1", "abc", "1", "1", "1", "AAAA"};
    EXPECT_EQ(EapHdiWpaManager::ParseEapEventData(vec, eapData), NETMANAGER_EXT_ERR_INTERNAL);
}

/**
 * @tc.name: ParseEapEventDataTest002
 * @tc.desc: Test ParseEapEventData with bufferLen exceeding the upper limit.
 * @tc.type: FUNC
 */
HWTEST_F(EapHdiWpaManagerTest, ParseEapEventDataTest002, TestSize.Level1)
{
    sptr<EapData> eapData = new (std::nothrow) EapData();
    // bufferLen 超上限
    std::vector<std::string> vec = {"1", "1", "1", "1", "99999", "AAAA"};
    EXPECT_EQ(EapHdiWpaManager::ParseEapEventData(vec, eapData), NETMANAGER_EXT_ERR_INTERNAL);
}

/**
 * @tc.name: ParseEapEventDataTest003
 * @tc.desc: Test ParseEapEventData with bufferLen mismatching the decoded buffer size.
 * @tc.type: FUNC
 */
HWTEST_F(EapHdiWpaManagerTest, ParseEapEventDataTest003, TestSize.Level1)
{
    sptr<EapData> eapData = new (std::nothrow) EapData();
    // bufferLen 与 Base64 解码后长度不一致：AAAA 解码为 3 字节
    std::vector<std::string> vec = {"1", "1", "1", "1", "1", "AAAA"};
    EXPECT_EQ(EapHdiWpaManager::ParseEapEventData(vec, eapData), NETMANAGER_EXT_ERR_INTERNAL);
}

/**
 * @tc.name: OnEapEventReportTest005
 * @tc.desc: Test OnEapEventReport with valid field count but parse failure.
 * @tc.type: FUNC
 */
HWTEST_F(EapHdiWpaManagerTest, OnEapEventReportTest005, TestSize.Level1)
{
    // 6 段格式合法但 bufferLen 不匹配：ParseEapEventData 失败后向上返回
    int32_t ret = EapHdiWpaManager::OnEapEventReport(nullptr, "wlan0", "1:1:1:1:1:AAAA");
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_INTERNAL);
}

/**
 * @tc.name: OnEapEventReportTest006
 * @tc.desc: Test OnEapEventReport with fully valid message, parse success and notify path.
 * @tc.type: FUNC
 */
HWTEST_F(EapHdiWpaManagerTest, OnEapEventReportTest006, TestSize.Level1)
{
    // 完整合法报文：bufferLen 与 Base64 解码字节数一致，解析成功后走通知链路
    int32_t ret = EapHdiWpaManager::OnEapEventReport(nullptr, "wlan0", "0:1:4:258:3:QUJD");
    EXPECT_EQ(ret, NETMANAGER_SUCCESS);
}

} // namespace NetManagerStandard
} // namespace OHOS
