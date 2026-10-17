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
#include <fstream>
 
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
