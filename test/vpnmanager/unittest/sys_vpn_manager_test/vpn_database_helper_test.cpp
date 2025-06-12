/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "net_manager_constants.h"
#include "vpn_database_defines.h"

#ifdef GTEST_API_
#define private public
#endif

#include "vpn_database_helper.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace testing::ext;

constexpr int32_t OLD_VERSION = 1;
constexpr int32_t NEW_VERSION = 2;

class VpnDatabaseHelperTest : public testing::Test {
public:
    static inline auto &vpnDataHelper_ = VpnDatabaseHelper::GetInstance();
};

HWTEST_F(VpnDatabaseHelperTest, OnCreate001, TestSize.Level1)
{
    auto dbCallBack = new (std::nothrow) VpnDataBaseCallBack();
    std::string vpnDatabaseName = VpnDatabaseDefines::VPN_DATABASE_PATH + VpnDatabaseDefines::VPN_DB_NAME;
    int32_t errCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbStoreConfig config(vpnDatabaseName);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    VpnDataBaseCallBack sqliteOpenHelperCallback;
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store_ =
        OHOS::NativeRdb::RdbHelper::GetRdbStore(config, OLD_VERSION, sqliteOpenHelperCallback, errCode);
    int32_t ret = dbCallBack->OnCreate(*(store_));
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VpnDatabaseHelperTest, OnUpgrade001, TestSize.Level1)
{
    auto dbCallBack = new (std::nothrow) VpnDataBaseCallBack();
    std::string vpnDatabaseName = VpnDatabaseDefines::VPN_DATABASE_PATH + VpnDatabaseDefines::VPN_DB_NAME;
    int32_t errCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbStoreConfig config(vpnDatabaseName);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    VpnDataBaseCallBack sqliteOpenHelperCallback;
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store_ =
        OHOS::NativeRdb::RdbHelper::GetRdbStore(config, NEW_VERSION, sqliteOpenHelperCallback, errCode);
    int32_t ret = dbCallBack->OnUpgrade(*(store_), OLD_VERSION, NEW_VERSION);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VpnDatabaseHelperTest, OnDowngrade001, TestSize.Level1)
{
    auto dbCallBack = new (std::nothrow) VpnDataBaseCallBack();
    std::string vpnDatabaseName = VpnDatabaseDefines::VPN_DATABASE_PATH + VpnDatabaseDefines::VPN_DB_NAME;
    int32_t errCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbStoreConfig config(vpnDatabaseName);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    VpnDataBaseCallBack sqliteOpenHelperCallback;
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store_ =
        OHOS::NativeRdb::RdbHelper::GetRdbStore(config, NEW_VERSION, sqliteOpenHelperCallback, errCode);
    int32_t ret = dbCallBack->OnDowngrade(*(store_), OLD_VERSION, NEW_VERSION);
    EXPECT_EQ(ret, NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VpnDatabaseHelperTest, IsVpnInfoExists001, TestSize.Level1)
{
    std::string vpnId = "0000";
    EXPECT_EQ(vpnDataHelper_.IsVpnInfoExists(vpnId), false);
}

HWTEST_F(VpnDatabaseHelperTest, IsVpnInfoExists002, TestSize.Level1)
{
    std::string vpnId = "1234";
    sptr<VpnDataBean> vpnBean = new (std::nothrow) VpnDataBean();
    if (vpnBean == nullptr) {
        return;
    }
    vpnBean->vpnId_ = "1234";
    vpnBean->userId_ = 100;
    vpnBean->vpnType_ = 1;
    vpnBean->vpnName_ = "name";
    vpnBean->vpnAddress_ = "1.1.1.1";
    vpnBean->isLegacy_ = 1;
    vpnBean->saveLogin_ = 1;
    vpnDataHelper_.InsertData(vpnBean);
    EXPECT_EQ(vpnDataHelper_.IsVpnInfoExists(vpnId), true);
}

HWTEST_F(VpnDatabaseHelperTest, InsertData001, TestSize.Level1)
{
    sptr<VpnDataBean> vpnBean = nullptr;
    EXPECT_EQ(vpnDataHelper_.InsertData(vpnBean), NETMANAGER_EXT_ERR_INVALID_PARAMETER);
}

HWTEST_F(VpnDatabaseHelperTest, InsertData002, TestSize.Level1)
{
    sptr<VpnDataBean> vpnBean = new (std::nothrow) VpnDataBean();
    ASSERT_NE(vpnBean, nullptr);
    vpnBean->vpnId_ = "1234";
    vpnBean->userId_ = 100;
    vpnBean->vpnType_ = 1;
    vpnBean->vpnName_ = "name";
    vpnBean->vpnAddress_ = "1.1.1.1";
    vpnBean->isLegacy_ = 1;
    vpnBean->saveLogin_ = 1;
    vpnDataHelper_.DeleteVpnData("1234");
    EXPECT_EQ(vpnDataHelper_.InsertData(vpnBean), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VpnDatabaseHelperTest, InsertOrUpdateData001, TestSize.Level1)
{
    sptr<VpnDataBean> vpnBean = nullptr;
    EXPECT_EQ(vpnDataHelper_.InsertOrUpdateData(vpnBean), NETMANAGER_EXT_ERR_INVALID_PARAMETER);
}

HWTEST_F(VpnDatabaseHelperTest, InsertOrUpdateData002, TestSize.Level1)
{
    sptr<VpnDataBean> vpnBean = new (std::nothrow) VpnDataBean();
    ASSERT_NE(vpnBean, nullptr);
    vpnBean->vpnId_ = "1234";
    vpnBean->userId_ = 100;
    vpnBean->vpnType_ = 1;
    vpnBean->vpnName_ = "name";
    vpnBean->vpnAddress_ = "1.1.1.1";
    vpnBean->isLegacy_ = 1;
    vpnBean->saveLogin_ = 1;
    EXPECT_EQ(vpnDataHelper_.InsertOrUpdateData(vpnBean), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VpnDatabaseHelperTest, QueryVpnData001, TestSize.Level1)
{
    std::string vpnId;
    sptr<VpnDataBean> vpnBean = nullptr;
    EXPECT_EQ(vpnDataHelper_.QueryVpnData(vpnBean, vpnId), NETMANAGER_EXT_ERR_INVALID_PARAMETER);
    vpnBean = new (std::nothrow) VpnDataBean();
    ASSERT_NE(vpnBean, nullptr);
    EXPECT_EQ(vpnDataHelper_.QueryVpnData(vpnBean, vpnId), NETMANAGER_EXT_ERR_INVALID_PARAMETER);
    vpnId = "1234";
    vpnBean->vpnId_ = "1234";
    vpnBean->userId_ = 100;
    vpnBean->vpnType_ = 1;
    vpnBean->vpnName_ = "name";
    vpnBean->vpnAddress_ = "1.1.1.1";
    vpnBean->isLegacy_ = 1;
    vpnBean->saveLogin_ = 1;
    vpnDataHelper_.InsertData(vpnBean);
    EXPECT_EQ(vpnDataHelper_.QueryVpnData(vpnBean, vpnId), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VpnDatabaseHelperTest, QueryAllData001, TestSize.Level1)
{
    std::vector<sptr<SysVpnConfig>> list;
    int32_t userId = 100;
    EXPECT_EQ(vpnDataHelper_.QueryAllData(list, userId), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VpnDatabaseHelperTest, DeleteVpnData001, TestSize.Level1)
{
    std::string vpnId;
    EXPECT_EQ(vpnDataHelper_.DeleteVpnData(vpnId), NETMANAGER_EXT_ERR_INVALID_PARAMETER);
    vpnId = "1234";
    sptr<VpnDataBean> vpnBean = new (std::nothrow) VpnDataBean();
    if (vpnBean == nullptr) {
        return;
    }
    vpnBean->vpnId_ = "1234";
    vpnBean->userId_ = 100;
    vpnBean->vpnType_ = 1;
    vpnBean->vpnName_ = "name";
    vpnBean->vpnAddress_ = "1.1.1.1";
    vpnBean->isLegacy_ = 1;
    vpnBean->saveLogin_ = 1;
    vpnDataHelper_.InsertData(vpnBean);
    EXPECT_EQ(vpnDataHelper_.DeleteVpnData(vpnId), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VpnDatabaseHelperTest, UpdateData001, TestSize.Level1)
{
    sptr<VpnDataBean> vpnBean = new (std::nothrow) VpnDataBean();
    if (vpnBean == nullptr) {
        return;
    }
    vpnBean->vpnId_ = "1234";
    vpnBean->userId_ = 100;
    vpnBean->vpnType_ = 1;
    vpnBean->vpnName_ = "name";
    vpnBean->vpnAddress_ = "1.1.1.1";
    vpnBean->isLegacy_ = 1;
    vpnBean->saveLogin_ = 1;
    vpnDataHelper_.InsertData(vpnBean);

    vpnBean = nullptr;
    EXPECT_EQ(vpnDataHelper_.UpdateData(vpnBean), NETMANAGER_EXT_ERR_INVALID_PARAMETER);
    vpnBean = new (std::nothrow) VpnDataBean();
    ASSERT_NE(vpnBean, nullptr);
    vpnBean->vpnId_ = "1234";
    vpnBean->userId_ = 100;
    vpnBean->vpnType_ = 1;
    vpnBean->vpnName_ = "name_update";
    vpnBean->vpnAddress_ = "2.2.2.2";
    vpnBean->isLegacy_ = 1;
    vpnBean->saveLogin_ = 1;
    EXPECT_EQ(vpnDataHelper_.UpdateData(vpnBean), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VpnDatabaseHelperTest, NoStore001, TestSize.Level1)
{
    std::shared_ptr<OHOS::NativeRdb::RdbStore> tmp = vpnDataHelper_.store_;
    vpnDataHelper_.store_ = nullptr;
    sptr<VpnDataBean> vpnBean = new (std::nothrow) VpnDataBean();
    ASSERT_NE(vpnBean, nullptr);
    vpnBean->vpnId_ = "1234";
    vpnBean->userId_ = 100;
    vpnBean->vpnType_ = 1;
    vpnBean->vpnName_ = "name";
    vpnBean->vpnAddress_ = "1.1.1.1";
    vpnBean->isLegacy_ = 1;
    vpnBean->saveLogin_ = 1;
    EXPECT_EQ(vpnDataHelper_.InsertData(vpnBean), NETMANAGER_EXT_ERR_OPERATION_FAILED);
    EXPECT_EQ(vpnDataHelper_.UpdateData(vpnBean), NETMANAGER_EXT_ERR_OPERATION_FAILED);
    EXPECT_EQ(vpnDataHelper_.QueryVpnData(vpnBean, vpnBean->vpnId_), NETMANAGER_EXT_ERR_OPERATION_FAILED);
    std::vector<sptr<SysVpnConfig>> list;
    EXPECT_EQ(vpnDataHelper_.QueryAllData(list, vpnBean->userId_), NETMANAGER_EXT_ERR_OPERATION_FAILED);
    EXPECT_EQ(vpnDataHelper_.DeleteVpnData(vpnBean->vpnId_), NETMANAGER_EXT_ERR_OPERATION_FAILED);
    vpnDataHelper_.store_ = tmp;
}

HWTEST_F(VpnDatabaseHelperTest, EncryptData001, TestSize.Level1)
{
    sptr<VpnDataBean> vpnBean = new (std::nothrow) VpnDataBean();
    ASSERT_NE(vpnBean, nullptr);
    vpnBean->vpnId_ = "1234";
    vpnBean->userName_ = "TEST";
    vpnBean->password_ = "111111";
    vpnBean->ipsecPreSharedKey_ = "ipsecPreSharedKey_";
    vpnBean->l2tpSharedKey_ = "l2tpSharedKey_";
    vpnBean->askpass_ = "swanctlConf_";
    vpnBean->swanctlConf_ = "ipsecPreSharedKey_";
    vpnBean->optionsL2tpdClient_ = "optionsL2tpdClient_";
    vpnBean->ipsecSecrets_ = "ipsecSecrets_";
    vpnBean->userId_ = 100;
    vpnBean->vpnType_ = 1;
    vpnBean->vpnName_ = "name";
    vpnBean->vpnAddress_ = "1.1.1.1";
    vpnBean->isLegacy_ = 1;
    vpnBean->saveLogin_ = 1;
    
    EXPECT_EQ(vpnDataHelper_.EncryptData(vpnBean), NETMANAGER_EXT_ERR_INTERNAL);
    EXPECT_EQ(vpnDataHelper_.DecryptData(vpnBean), NETMANAGER_EXT_ERR_INTERNAL);
}
} // namespace NetManagerStandard
} // namespace OHOS