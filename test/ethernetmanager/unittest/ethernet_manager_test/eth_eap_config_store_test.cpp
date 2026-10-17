/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
#include <sstream>

#include "eth_eap_config_store.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {

#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE

namespace {
constexpr const char *ETH_EAP_CONFIG_TEST_PATH = "/data/service/el1/public/eth/eth_eap_config.json";
constexpr const char *ETH_EAP_CONFIG_TEST_LINK_TARGET = "/data/service/el1/public/eth/eth_eap_config_link_target";
} // namespace

using namespace testing::ext;

class EthEapConfigStoreTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        EthEapConfigStore::GetInstance().DeleteConfig();
    }
    void TearDown() override
    {
        EthEapConfigStore::GetInstance().DeleteConfig();
    }

    static EthEapConfig MakeFullConfig()
    {
        EthEapConfig config;
        config.profile.eapMethod = EapMethod::EAP_PEAP;
        config.profile.phase2Method = Phase2Method::PHASE2_MSCHAPV2;
        config.profile.identity = "eap-dt-identity";
        config.profile.anonymousIdentity = "eap-dt-anonymous";
        config.profile.password.assign("EapDtPass@123");
        config.profile.caCertAliases = "eap_dt_ca_alias";
        config.profile.caPath = "/data/service/el1/public/eth/eap_certs/ca.pem";
        config.profile.clientCertAliases = "oh:eap_dt_client_alias";
        // 覆盖含 0x00/0xFF 的二进制内容，验证加密通道不依赖文本语义
        config.profile.certEntry = {0x30, 0x82, 0x01, 0x00, 0xFF, 0x7F, 0x80, 0x01, 0x02, 0x03};
        config.profile.certPassword.assign("CertPass@456");
        config.profile.altSubjectMatch = "DNS:example.com";
        config.profile.domainSuffixMatch = "example.com";
        config.profile.realm = "eap-dt-realm";
        config.profile.plmn = "46000";
        config.profile.eapSubId = 1;
        config.enabled = true;
        config.autoAuth = false;
        config.locked = true;
        return config;
    }
};

/**
 * @tc.name  : SaveAndLoadConfigTest001
 * @tc.number: EthEapConfigStoreTest_001
 * @tc.desc  : 全字段配置保存后回读，所有字段（含加密的密码/证书口令/证书内容）保持一致
 */
HWTEST_F(EthEapConfigStoreTest, SaveAndLoadConfigTest001, TestSize.Level1)
{
    EthEapConfig in = MakeFullConfig();
    int32_t saveRet = EthEapConfigStore::GetInstance().SaveConfig(in);
    EXPECT_EQ(saveRet, NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(EthEapConfigStore::GetInstance().HasConfig());

    EthEapConfig out;
    int32_t loadRet = EthEapConfigStore::GetInstance().LoadConfig(out);
    EXPECT_EQ(loadRet, NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(out.profile.eapMethod, in.profile.eapMethod);
    EXPECT_EQ(out.profile.phase2Method, in.profile.phase2Method);
    EXPECT_EQ(out.profile.identity, in.profile.identity);
    EXPECT_EQ(out.profile.anonymousIdentity, in.profile.anonymousIdentity);
    EXPECT_STREQ(out.profile.password.c_str(), in.profile.password.c_str());
    EXPECT_EQ(out.profile.caCertAliases, in.profile.caCertAliases);
    EXPECT_EQ(out.profile.caPath, in.profile.caPath);
    EXPECT_EQ(out.profile.clientCertAliases, in.profile.clientCertAliases);
    EXPECT_EQ(out.profile.certEntry, in.profile.certEntry);
    EXPECT_STREQ(out.profile.certPassword.c_str(), in.profile.certPassword.c_str());
    EXPECT_EQ(out.profile.altSubjectMatch, in.profile.altSubjectMatch);
    EXPECT_EQ(out.profile.domainSuffixMatch, in.profile.domainSuffixMatch);
    EXPECT_EQ(out.profile.realm, in.profile.realm);
    EXPECT_EQ(out.profile.plmn, in.profile.plmn);
    EXPECT_EQ(out.profile.eapSubId, in.profile.eapSubId);
    EXPECT_EQ(out.enabled, in.enabled);
    EXPECT_EQ(out.autoAuth, in.autoAuth);
    EXPECT_EQ(out.locked, in.locked);
}

/**
 * @tc.name  : LoadConfigWithoutFileTest002
 * @tc.number: EthEapConfigStoreTest_002
 * @tc.desc  : 未保存配置时回读返回文件不存在错误码（2200209），上层据此区分未配置
 */
HWTEST_F(EthEapConfigStoreTest, LoadConfigWithoutFileTest002, TestSize.Level1)
{
    EthEapConfig out;
    int32_t ret = EthEapConfigStore::GetInstance().LoadConfig(out);
    EXPECT_EQ(ret, NETMANAGER_EXT_ERR_FILE_NOT_FOUND);
    EXPECT_FALSE(EthEapConfigStore::GetInstance().HasConfig());
}

/**
 * @tc.name  : DeleteConfigLifecycleTest003
 * @tc.number: EthEapConfigStoreTest_003
 * @tc.desc  : HasConfig 状态随保存/删除翻转，删除幂等（不存在时不报错）
 */
HWTEST_F(EthEapConfigStoreTest, DeleteConfigLifecycleTest003, TestSize.Level1)
{
    EXPECT_FALSE(EthEapConfigStore::GetInstance().HasConfig());
    EthEapConfig in = MakeFullConfig();
    EXPECT_EQ(EthEapConfigStore::GetInstance().SaveConfig(in), NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(EthEapConfigStore::GetInstance().HasConfig());
    EXPECT_EQ(EthEapConfigStore::GetInstance().DeleteConfig(), NETMANAGER_EXT_SUCCESS);
    EXPECT_FALSE(EthEapConfigStore::GetInstance().HasConfig());
    // 幂等删除：文件已不存在仍返回成功
    EXPECT_EQ(EthEapConfigStore::GetInstance().DeleteConfig(), NETMANAGER_EXT_SUCCESS);
}

/**
 * @tc.name  : LoadConfigCorruptedFileTest004
 * @tc.number: EthEapConfigStoreTest_004
 * @tc.desc  : 配置文件内容损坏（非法 JSON）时回读返回内部错误，不崩溃
 */
HWTEST_F(EthEapConfigStoreTest, LoadConfigCorruptedFileTest004, TestSize.Level1)
{
    EthEapConfig in = MakeFullConfig();
    ASSERT_EQ(EthEapConfigStore::GetInstance().SaveConfig(in), NETMANAGER_EXT_SUCCESS);
    // 直接写坏配置文件模拟掉电/损坏场景
    std::ofstream badFile(ETH_EAP_CONFIG_TEST_PATH, std::ios::out | std::ios::trunc);
    ASSERT_TRUE(badFile.is_open());
    badFile << "{ this is not a valid json !!";
    badFile.close();

    EthEapConfig out;
    EXPECT_EQ(EthEapConfigStore::GetInstance().LoadConfig(out), NETMANAGER_EXT_ERR_INTERNAL);
}

/**
 * @tc.name  : LoadConfigSymlinkRejectedTest005
 * @tc.number: EthEapConfigStoreTest_005
 * @tc.desc  : 配置文件被替换为符号链接时拒绝回读（安全防护），HasConfig 判定不存在
 */
HWTEST_F(EthEapConfigStoreTest, LoadConfigSymlinkRejectedTest005, TestSize.Level1)
{
    EthEapConfig in = MakeFullConfig();
    ASSERT_EQ(EthEapConfigStore::GetInstance().SaveConfig(in), NETMANAGER_EXT_SUCCESS);
    ASSERT_EQ(unlink(ETH_EAP_CONFIG_TEST_PATH), 0);
    // 构造链接目标文件与符号链接
    std::ofstream target(ETH_EAP_CONFIG_TEST_LINK_TARGET, std::ios::out | std::ios::trunc);
    ASSERT_TRUE(target.is_open());
    target << "{}";
    target.close();
    ASSERT_EQ(symlink(ETH_EAP_CONFIG_TEST_LINK_TARGET, ETH_EAP_CONFIG_TEST_PATH), 0);

    EthEapConfig out;
    EXPECT_EQ(EthEapConfigStore::GetInstance().LoadConfig(out), NETMANAGER_EXT_ERR_INTERNAL);
    // 符号链接不算已配置（防止误判）
    EXPECT_FALSE(EthEapConfigStore::GetInstance().HasConfig());
    // 清理链接与目标文件
    ASSERT_EQ(unlink(ETH_EAP_CONFIG_TEST_PATH), 0);
    ASSERT_EQ(unlink(ETH_EAP_CONFIG_TEST_LINK_TARGET), 0);
}

/**
 * @tc.name  : SaveConfigDefaultConfigTest006
 * @tc.number: EthEapConfigStoreTest_006
 * @tc.desc  : 全默认值配置（未设置任何字段）保存回读成功且保持默认值
 */
HWTEST_F(EthEapConfigStoreTest, SaveConfigDefaultConfigTest006, TestSize.Level1)
{
    EthEapConfig in;
    EXPECT_EQ(EthEapConfigStore::GetInstance().SaveConfig(in), NETMANAGER_EXT_SUCCESS);
    EthEapConfig out;
    EXPECT_EQ(EthEapConfigStore::GetInstance().LoadConfig(out), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(out.profile.eapMethod, EapMethod::EAP_NONE);
    EXPECT_EQ(out.profile.phase2Method, Phase2Method::PHASE2_NONE);
    EXPECT_TRUE(out.profile.identity.empty());
    EXPECT_TRUE(out.profile.password.empty());
    EXPECT_FALSE(out.enabled);
    EXPECT_TRUE(out.autoAuth);
    EXPECT_FALSE(out.locked);
}

/**
 * @tc.name  : SaveConfigSkipSameContentTest007
 * @tc.number: EthEapConfigStoreTest_007
 * @tc.desc  : 配置内容完全一致时跳过落盘（文件 mtime 不变），减少 flash 磨损
 */
HWTEST_F(EthEapConfigStoreTest, SaveConfigSkipSameContentTest007, TestSize.Level1)
{
    EthEapConfig in = MakeFullConfig();
    ASSERT_EQ(EthEapConfigStore::GetInstance().SaveConfig(in), NETMANAGER_EXT_SUCCESS);
    struct stat before = {};
    ASSERT_EQ(stat(ETH_EAP_CONFIG_TEST_PATH, &before), 0);
    // 二次保存相同内容：序列化密文含随机 nonce，必须按字段比较后跳过写入
    EXPECT_EQ(EthEapConfigStore::GetInstance().SaveConfig(in), NETMANAGER_EXT_SUCCESS);
    struct stat after = {};
    ASSERT_EQ(stat(ETH_EAP_CONFIG_TEST_PATH, &after), 0);
    EXPECT_EQ(before.st_mtim.tv_sec, after.st_mtim.tv_sec);
    EXPECT_EQ(before.st_mtim.tv_nsec, after.st_mtim.tv_nsec);
}

/**
 * @tc.name  : SaveConfigContentChangedRewriteTest008
 * @tc.number: EthEapConfigStoreTest_008
 * @tc.desc  : 配置内容变化时正常重写（mtime 变化），新内容生效
 */
HWTEST_F(EthEapConfigStoreTest, SaveConfigContentChangedRewriteTest008, TestSize.Level1)
{
    EthEapConfig in = MakeFullConfig();
    ASSERT_EQ(EthEapConfigStore::GetInstance().SaveConfig(in), NETMANAGER_EXT_SUCCESS);
    struct stat before = {};
    ASSERT_EQ(stat(ETH_EAP_CONFIG_TEST_PATH, &before), 0);

    in.profile.identity = "eap-dt-identity-v2";
    EXPECT_EQ(EthEapConfigStore::GetInstance().SaveConfig(in), NETMANAGER_EXT_SUCCESS);
    struct stat after = {};
    ASSERT_EQ(stat(ETH_EAP_CONFIG_TEST_PATH, &after), 0);
    bool mtimeChanged = (before.st_mtim.tv_sec != after.st_mtim.tv_sec) ||
                        (before.st_mtim.tv_nsec != after.st_mtim.tv_nsec);
    EXPECT_TRUE(mtimeChanged);

    EthEapConfig out;
    ASSERT_EQ(EthEapConfigStore::GetInstance().LoadConfig(out), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(out.profile.identity, "eap-dt-identity-v2");
}

/**
 * @tc.name  : SaveConfigEncryptedAtRestTest009
 * @tc.number: EthEapConfigStoreTest_009
 * @tc.desc  : 敏感字段（密码/证书口令）落盘为密文，明文不出现在配置文件中
 */
HWTEST_F(EthEapConfigStoreTest, SaveConfigEncryptedAtRestTest009, TestSize.Level1)
{
    EthEapConfig in = MakeFullConfig();
    ASSERT_EQ(EthEapConfigStore::GetInstance().SaveConfig(in), NETMANAGER_EXT_SUCCESS);
    std::ifstream file(ETH_EAP_CONFIG_TEST_PATH, std::ios::in);
    ASSERT_TRUE(file.is_open());
    std::stringstream ss;
    ss << file.rdbuf();
    file.close();
    std::string content = ss.str();
    // 明文密码与证书口令不得出现在文件中
    EXPECT_EQ(content.find("EapDtPass@123"), std::string::npos);
    EXPECT_EQ(content.find("CertPass@456"), std::string::npos);
    // 字段名存在（密文以字符串字段形式存储）
    EXPECT_NE(content.find("password"), std::string::npos);
    EXPECT_NE(content.find("certPassword"), std::string::npos);
}

/**
 * @tc.name  : SaveConfigFilePermissionTest010
 * @tc.number: EthEapConfigStoreTest_010
 * @tc.desc  : 配置文件权限为 0600（仅属主可读写），防止其他进程读取
 */
HWTEST_F(EthEapConfigStoreTest, SaveConfigFilePermissionTest010, TestSize.Level1)
{
    EthEapConfig in = MakeFullConfig();
    ASSERT_EQ(EthEapConfigStore::GetInstance().SaveConfig(in), NETMANAGER_EXT_SUCCESS);
    struct stat st = {};
    ASSERT_EQ(stat(ETH_EAP_CONFIG_TEST_PATH, &st), 0);
    EXPECT_EQ(st.st_mode & 0777, S_IRUSR | S_IWUSR);
}

/**
 * @tc.name  : SaveConfigOverwriteLegacyPlainJsonTest011
 * @tc.number: EthEapConfigStoreTest_011
 * @tc.desc  : 已存在旧明文 JSON（含明文 identity、无加密字段）时保存成功并完成加密迁移
 */
HWTEST_F(EthEapConfigStoreTest, SaveConfigOverwriteLegacyPlainJsonTest011, TestSize.Level1)
{
    // 模拟升级场景：磁盘上存在一份不含加密字段的旧配置
    std::ofstream legacy(ETH_EAP_CONFIG_TEST_PATH, std::ios::out | std::ios::trunc);
    ASSERT_TRUE(legacy.is_open());
    legacy << "{\"eapMethod\":2,\"identity\":\"legacy-identity\",\"enabled\":true}";
    legacy.close();

    EthEapConfig in = MakeFullConfig();
    EXPECT_EQ(EthEapConfigStore::GetInstance().SaveConfig(in), NETMANAGER_EXT_SUCCESS);
    EthEapConfig out;
    ASSERT_EQ(EthEapConfigStore::GetInstance().LoadConfig(out), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(out.profile.identity, "eap-dt-identity");
    EXPECT_STREQ(out.profile.password.c_str(), "EapDtPass@123");
}

/**
 * @tc.name  : LoadConfigPartialJsonKeepsDefaultTest012
 * @tc.number: EthEapConfigStoreTest_012
 * @tc.desc  : 配置文件仅含部分字段时容错解析，缺失字段保持默认值
 */
HWTEST_F(EthEapConfigStoreTest, LoadConfigPartialJsonKeepsDefaultTest012, TestSize.Level1)
{
    std::ofstream partial(ETH_EAP_CONFIG_TEST_PATH, std::ios::out | std::ios::trunc);
    ASSERT_TRUE(partial.is_open());
    partial << "{\"eapMethod\":3,\"identity\":\"partial-identity\"}";
    partial.close();

    EthEapConfig out;
    EXPECT_EQ(EthEapConfigStore::GetInstance().LoadConfig(out), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(out.profile.eapMethod, EapMethod::EAP_TTLS);
    EXPECT_EQ(out.profile.identity, "partial-identity");
    // 缺失字段保持默认
    EXPECT_EQ(out.profile.phase2Method, Phase2Method::PHASE2_NONE);
    EXPECT_TRUE(out.profile.password.empty());
    EXPECT_FALSE(out.enabled);
    EXPECT_TRUE(out.autoAuth);
    EXPECT_FALSE(out.locked);
}

/**
 * @tc.name  : SaveConfigLongPasswordRoundtripTest013
 * @tc.number: EthEapConfigStoreTest_013
 * @tc.desc  : 长密码（256 字节）与多字节 UTF-8 内容加密保存后完整回读
 */
HWTEST_F(EthEapConfigStoreTest, SaveConfigLongPasswordRoundtripTest013, TestSize.Level1)
{
    EthEapConfig in = MakeFullConfig();
    std::string longPassword(256, 'A');
    longPassword += "-中文口令-";
    in.profile.password.assign(longPassword);
    in.profile.identity = "utf8-身份标识";
    ASSERT_EQ(EthEapConfigStore::GetInstance().SaveConfig(in), NETMANAGER_EXT_SUCCESS);

    EthEapConfig out;
    ASSERT_EQ(EthEapConfigStore::GetInstance().LoadConfig(out), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(out.profile.password, longPassword);
    EXPECT_EQ(out.profile.identity, "utf8-身份标识");
}

#endif // NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE

} // namespace NetManagerStandard
} // namespace OHOS
