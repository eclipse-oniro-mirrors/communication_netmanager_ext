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
#include <unistd.h>

#include "eap_dt_test_p12.h"
#include "eth_eap_cert_store.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {

#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE

namespace {
constexpr const char *TEST_CERT_DIR = "/data/service/el1/public/eth/eap_certs/";
constexpr const char *TEST_CA_FILE = "/data/service/el1/public/eth/eap_certs/ca.pem";
constexpr const char *TEST_CLIENT_FILE = "/data/service/el1/public/eth/eap_certs/client.p12";
} // namespace

using namespace testing::ext;

class EthEapCertStoreTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        EthEapCertStore::GetInstance().Clear();
    }
    void TearDown() override
    {
        EthEapCertStore::GetInstance().Clear();
    }

    static bool FileExist(const std::string &path)
    {
        struct stat st = {};
        return lstat(path.c_str(), &st) == 0;
    }

    static std::vector<uint8_t> MakeValidP12()
    {
        return std::vector<uint8_t>(TEST_PKCS12_DATA, TEST_PKCS12_DATA + TEST_PKCS12_LEN);
    }
};

/**
 * @tc.name  : ProcessCertFieldsEmptyProfileTest001
 * @tc.number: EthEapCertStoreTest_001
 * @tc.desc  : 空配置（无证书字段）处理成功且无副作用
 */
HWTEST_F(EthEapCertStoreTest, ProcessCertFieldsEmptyProfileTest001, TestSize.Level1)
{
    EthEapProfile profile;
    EXPECT_EQ(EthEapCertStore::GetInstance().ProcessCertFields(profile), NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(profile.certEntry.empty());
    EXPECT_TRUE(profile.clientCertAliases.empty());
    EXPECT_FALSE(FileExist(TEST_CLIENT_FILE));
}

/**
 * @tc.name  : ProcessCertFieldsInvalidEntryTest002
 * @tc.number: EthEapCertStoreTest_002
 * @tc.desc  : certEntry 非法（非 PKCS12 结构）返回证书处理失败错误码 33200013
 */
HWTEST_F(EthEapCertStoreTest, ProcessCertFieldsInvalidEntryTest002, TestSize.Level1)
{
    EthEapProfile profile;
    profile.certEntry = {'t', 'h', 'i', 's', ' ', 'i', 's', ' ', 'n', 'o', 't', ' ', 'p', '1', '2'};
    EXPECT_EQ(EthEapCertStore::GetInstance().ProcessCertFields(profile), EAP_ERRCODE_CERT_PROCESS_FAILED);
    EXPECT_FALSE(FileExist(TEST_CLIENT_FILE));
}

/**
 * @tc.name  : ProcessCertFieldsValidEntryTest003
 * @tc.number: EthEapCertStoreTest_003
 * @tc.desc  : 合法 P12 内容校验通过并落盘 client.p12，clientCertAliases 回填落盘路径
 */
HWTEST_F(EthEapCertStoreTest, ProcessCertFieldsValidEntryTest003, TestSize.Level1)
{
    EthEapProfile profile;
    profile.certEntry = MakeValidP12();
    ASSERT_EQ(EthEapCertStore::GetInstance().ProcessCertFields(profile), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(profile.clientCertAliases, TEST_CLIENT_FILE);
    EXPECT_TRUE(FileExist(TEST_CLIENT_FILE));
    // 落盘内容与入参一致
    struct stat st = {};
    ASSERT_EQ(stat(TEST_CLIENT_FILE, &st), 0);
    EXPECT_EQ(static_cast<size_t>(st.st_size), TEST_PKCS12_LEN);
    // 落盘文件权限 0600
    EXPECT_EQ(st.st_mode & 0777, S_IRUSR | S_IWUSR);
}

/**
 * @tc.name  : ProcessCertFieldsInvalidEmptyEntryTest004
 * @tc.number: EthEapCertStoreTest_004
 * @tc.desc  : certEntry 为空数组视为未提供，不触发校验失败
 */
HWTEST_F(EthEapCertStoreTest, ProcessCertFieldsInvalidEmptyEntryTest004, TestSize.Level1)
{
    EthEapProfile profile;
    profile.certEntry = {};
    EXPECT_EQ(EthEapCertStore::GetInstance().ProcessCertFields(profile), NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(profile.clientCertAliases.empty());
    EXPECT_FALSE(FileExist(TEST_CLIENT_FILE));
}

/**
 * @tc.name  : ProcessCertFieldsCaKeyUriDegradeTest005
 * @tc.number: EthEapCertStoreTest_005
 * @tc.desc  : CA keyUri 在证书管理器中查不到时降级（不报错），陈旧 caPath 被清理
 */
HWTEST_F(EthEapCertStoreTest, ProcessCertFieldsCaKeyUriDegradeTest005, TestSize.Level1)
{
    EthEapProfile profile;
    profile.caCertAliases = "oh:so.not.exist.uri";
    profile.caPath = TEST_CA_FILE; // 陈旧的托管路径应被清理，防止 wpa 加载不存在的文件
    EXPECT_EQ(EthEapCertStore::GetInstance().ProcessCertFields(profile), NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(profile.caPath.empty());
}

/**
 * @tc.name  : ProcessCertFieldsCaPathPreservedTest006
 * @tc.number: EthEapCertStoreTest_006
 * @tc.desc  : caPath 指向非托管路径（企业 API 直传）时不被 keyUri 解析覆盖
 */
HWTEST_F(EthEapCertStoreTest, ProcessCertFieldsCaPathPreservedTest006, TestSize.Level1)
{
    EthEapProfile profile;
    profile.caCertAliases = "oh:so.not.exist.uri";
    profile.caPath = "/data/service/el1/public/eth/custom_ca.pem"; // 非本类管理路径
    EXPECT_EQ(EthEapCertStore::GetInstance().ProcessCertFields(profile), NETMANAGER_EXT_SUCCESS);
    EXPECT_EQ(profile.caPath, "/data/service/el1/public/eth/custom_ca.pem");
}

/**
 * @tc.name  : ProcessCertFieldsPlainCaAliasSkippedTest007
 * @tc.number: EthEapCertStoreTest_007
 * @tc.desc  : caCertAliases 非 keyUri（无 oh: 前缀）时不走证书管理器解析
 */
HWTEST_F(EthEapCertStoreTest, ProcessCertFieldsPlainCaAliasSkippedTest007, TestSize.Level1)
{
    EthEapProfile profile;
    profile.caCertAliases = "plain_alias";
    profile.caPath = "";
    EXPECT_EQ(EthEapCertStore::GetInstance().ProcessCertFields(profile), NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(profile.caPath.empty());
    EXPECT_FALSE(FileExist(TEST_CA_FILE));
}

/**
 * @tc.name  : ClearRemovesCertFilesTest008
 * @tc.number: EthEapCertStoreTest_008
 * @tc.desc  : Clear 删除落盘证书文件；文件不存在时清理幂等不崩溃
 */
HWTEST_F(EthEapCertStoreTest, ClearRemovesCertFilesTest008, TestSize.Level1)
{
    EthEapProfile profile;
    profile.certEntry = MakeValidP12();
    ASSERT_EQ(EthEapCertStore::GetInstance().ProcessCertFields(profile), NETMANAGER_EXT_SUCCESS);
    ASSERT_TRUE(FileExist(TEST_CLIENT_FILE));

    EthEapCertStore::GetInstance().Clear();
    EXPECT_FALSE(FileExist(TEST_CLIENT_FILE));
    EXPECT_FALSE(FileExist(TEST_CA_FILE));
    // 幂等清理
    EXPECT_NO_FATAL_FAILURE(EthEapCertStore::GetInstance().Clear());
}

/**
 * @tc.name  : EnsureCertFilesRebuildClientTest009
 * @tc.number: EthEapCertStoreTest_009
 * @tc.desc  : client.p12 丢失时 EnsureCertFiles 用持久化数据重建并回填路径
 */
HWTEST_F(EthEapCertStoreTest, EnsureCertFilesRebuildClientTest009, TestSize.Level1)
{
    EthEapProfile profile;
    profile.certEntry = MakeValidP12();
    ASSERT_EQ(EthEapCertStore::GetInstance().ProcessCertFields(profile), NETMANAGER_EXT_SUCCESS);
    ASSERT_EQ(unlink(TEST_CLIENT_FILE), 0);

    // 认证前自愈：文件缺失时重建
    EXPECT_EQ(EthEapCertStore::GetInstance().EnsureCertFiles(profile), NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(FileExist(TEST_CLIENT_FILE));
    EXPECT_EQ(profile.clientCertAliases, TEST_CLIENT_FILE);
}

/**
 * @tc.name  : EnsureCertFilesNoRebuildWhenExistTest010
 * @tc.number: EthEapCertStoreTest_010
 * @tc.desc  : 落盘文件存在时 EnsureCertFiles 不重复重建
 */
HWTEST_F(EthEapCertStoreTest, EnsureCertFilesNoRebuildWhenExistTest010, TestSize.Level1)
{
    EthEapProfile profile;
    profile.certEntry = MakeValidP12();
    ASSERT_EQ(EthEapCertStore::GetInstance().ProcessCertFields(profile), NETMANAGER_EXT_SUCCESS);
    struct stat before = {};
    ASSERT_EQ(stat(TEST_CLIENT_FILE, &before), 0);

    EXPECT_EQ(EthEapCertStore::GetInstance().EnsureCertFiles(profile), NETMANAGER_EXT_SUCCESS);
    struct stat after = {};
    ASSERT_EQ(stat(TEST_CLIENT_FILE, &after), 0);
    EXPECT_EQ(before.st_mtim.tv_sec, after.st_mtim.tv_sec);
    EXPECT_EQ(before.st_mtim.tv_nsec, after.st_mtim.tv_nsec);
}

/**
 * @tc.name  : EnsureCertFilesInvalidEntryToleratedTest011
 * @tc.number: EthEapCertStoreTest_011
 * @tc.desc  : certEntry 非法且文件缺失时重建失败仅告警，整体仍返回成功（不阻断认证）
 */
HWTEST_F(EthEapCertStoreTest, EnsureCertFilesInvalidEntryToleratedTest011, TestSize.Level1)
{
    EthEapProfile profile;
    profile.certEntry = {'n', 'o', 't', 'p', '1', '2'};
    EXPECT_EQ(EthEapCertStore::GetInstance().EnsureCertFiles(profile), NETMANAGER_EXT_SUCCESS);
    EXPECT_FALSE(FileExist(TEST_CLIENT_FILE));
    EXPECT_TRUE(profile.clientCertAliases.empty());
}

/**
 * @tc.name  : EnsureCertFilesCaDegradeTest012
 * @tc.number: EthEapCertStoreTest_012
 * @tc.desc  : CA keyUri 自愈失败时清理陈旧托管 caPath，保持降级（不校验服务器证书）
 */
HWTEST_F(EthEapCertStoreTest, EnsureCertFilesCaDegradeTest012, TestSize.Level1)
{
    EthEapProfile profile;
    profile.caCertAliases = "oh:so.not.exist.uri";
    profile.caPath = TEST_CA_FILE;
    EXPECT_EQ(EthEapCertStore::GetInstance().EnsureCertFiles(profile), NETMANAGER_EXT_SUCCESS);
    EXPECT_TRUE(profile.caPath.empty());
}

/**
 * @tc.name  : AtomicWriteSymlinkRejectedTest013
 * @tc.number: EthEapCertStoreTest_013
 * @tc.desc  : 落盘目标被符号链接替换时拒绝写入（安全防护），返回证书处理失败
 */
HWTEST_F(EthEapCertStoreTest, AtomicWriteSymlinkRejectedTest013, TestSize.Level1)
{
    constexpr const char *LINK_TARGET = "/data/service/el1/public/eth/eap_certs/link_target.txt";
    // 构造目标文件与符号链接
    FILE *fp = fopen(LINK_TARGET, "w");
    ASSERT_NE(fp, nullptr);
    (void)fclose(fp);
    ASSERT_EQ(symlink(LINK_TARGET, TEST_CLIENT_FILE), 0);

    EthEapProfile profile;
    profile.certEntry = MakeValidP12();
    EXPECT_EQ(EthEapCertStore::GetInstance().ProcessCertFields(profile), EAP_ERRCODE_CERT_PROCESS_FAILED);
    // 符号链接未被替换成真实文件
    struct stat st = {};
    ASSERT_EQ(lstat(TEST_CLIENT_FILE, &st), 0);
    EXPECT_TRUE(S_ISLNK(st.st_mode));

    ASSERT_EQ(unlink(TEST_CLIENT_FILE), 0);
    ASSERT_EQ(unlink(LINK_TARGET), 0);
}

#endif // NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE

} // namespace NetManagerStandard
} // namespace OHOS
