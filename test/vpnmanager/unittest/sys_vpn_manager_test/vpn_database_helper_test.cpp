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
    static inline auto &vpnDataHelper_ = VpnDatabaseHelper::GetInstance();
};

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
    EXPECT_EQ(vpnDataHelper_.InsertData(vpnBean), NETMANAGER_EXT_ERR_OPERATION_FAILED);
}

HWTEST_F(VpnDatabaseHelperTest, InsertOrUpdateData001, TestSize.Level1)
{
    sptr<VpnDataBean> vpnBean = new (std::nothrow) VpnDataBean();
    if (vpnBean == nullptr) {
        return;
    }
    EXPECT_EQ(vpnDataHelper_.InsertOrUpdateData(vpnBean), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VpnDatabaseHelperTest, QueryVpnData001, TestSize.Level1)
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
    EXPECT_EQ(vpnDataHelper_.QueryVpnData(vpnBean, vpnId), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VpnDatabaseHelperTest, QueryAllData001, TestSize.Level1)
{
    std::vector<SysVpnConfig> list;
    int32_t userId = 100;
    EXPECT_EQ(vpnDataHelper_.QueryAllData(list, userId), NETMANAGER_EXT_SUCCESS);
}

HWTEST_F(VpnDatabaseHelperTest, DeleteVpnData001, TestSize.Level1)
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

    sptr<VpnDataBean> updateVpnBean = new (std::nothrow) VpnDataBean();
    vpnBean->vpnId_ = "1234";
    vpnBean->userId_ = 100;
    vpnBean->vpnType_ = 1;
    vpnBean->vpnName_ = "name_update";
    vpnBean->vpnAddress_ = "2.2.2.2";
    vpnBean->isLegacy_ = 1;
    vpnBean->saveLogin_ = 1;
    EXPECT_EQ(vpnDataHelper_.UpdateData(updateVpnBean), NETMANAGER_EXT_SUCCESS);
}
} // namespace NetManagerStandard
} // namespace OHOS