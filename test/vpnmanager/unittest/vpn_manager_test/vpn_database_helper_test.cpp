/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <memory>

#include <gtest/gtest.h>

#ifdef GTEST_API_
#define private public
#endif
#include "vpn_database_helper.h"
#include "net_manager_constants.h"

namespace OHOS {
namespace NetManagerStandard {

using namespace testing::ext;

class VpnDatabaseHelperTest : public testing::Test {
public:
    static inline std::shared_ptr<VpnDatabaseHelper> vpnDataHelper_ = std::make_shared<VpnDatabaseHelper>("/data");  
};

VpnDatabaseHelper::SqlCallback sqlCallback = [](void *notUsed, int argc, char **argv, char **colName) {
    return 0;
};

HWTEST_F(VpnDatabaseHelperTest, CreateTable001, TestSize.Level1)
{
    EXPECT_EQ(vpnDataHelper_->CreateTable(), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VpnDatabaseHelperTest, InsertData001, TestSize.Level1)
{
    sptr<VpnDataBean> vpnBean = new(std::nothrow) VpnDataBean();
    EXPECT_EQ(vpnDataHelper_->InsertData(vpnBean), NETMANAGER_ERR_WRITE_DATA_FAIL);
}

HWTEST_F(VpnDatabaseHelperTest, InsertOrUpdateData001, TestSize.Level1)
{
    sptr<VpnDataBean> vpnBean = new(std::nothrow) VpnDataBean();
    EXPECT_EQ(vpnDataHelper_->InsertOrUpdateData(vpnBean), NETMANAGER_ERR_WRITE_DATA_FAIL);
}

HWTEST_F(VpnDatabaseHelperTest, QueryVpnData001, TestSize.Level1)
{
    sptr<VpnDataBean> vpnBean = new(std::nothrow) VpnDataBean();
    std::string vpnId = "1234";
    int32_t userId = 100;
    EXPECT_EQ(vpnDataHelper_->QueryVpnData(vpnBean, vpnId, userId), NETMANAGER_SUCCESS);
}
HWTEST_F(VpnDatabaseHelperTest, QueryAllData001, TestSize.Level1)
{
    std::vector<SysVpnConfig> list;
    int32_t userId = 100;
    EXPECT_EQ(vpnDataHelper_->QueryAllData(list, userId), NETMANAGER_SUCCESS);
}
HWTEST_F(VpnDatabaseHelperTest, DeleteVpnData001, TestSize.Level1)
{
    std::string vpnId = "1234";
    int32_t userId = 100;
    EXPECT_EQ(vpnDataHelper_->DeleteVpnData(vpnId, userId), NETMANAGER_SUCCESS);
}
HWTEST_F(VpnDatabaseHelperTest, UpdateData001, TestSize.Level1)
{
    sptr<VpnDataBean> vpnBean = new(std::nothrow) VpnDataBean();
    EXPECT_EQ(vpnDataHelper_->UpdateData(vpnBean), NETMANAGER_ERR_WRITE_DATA_FAIL);
}

HWTEST_F(VpnDatabaseHelperTest, ExecSql001, TestSize.Level1)
{
    std::string sql = "sql";
    EXPECT_EQ(vpnDataHelper_->ExecSql(sql, nullptr, sqlCallback), NETMANAGER_ERROR);
}
} // namespace NetManagerStandard
} // namespace OHOS