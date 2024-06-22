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

#include "vpn_sqlite_statement.h"

#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include <gtest/gtest.h>
#include "net_stats_constants.h"

#ifndef USE_SQLITE_SYMBOLS
#include "sqlite3.h"
#else
#include "sqlite3sym.h"
#endif

namespace OHOS {
namespace NetManagerStandard {

using namespace testing::ext;

class VpnSqliteStatementTest : public testing::Test {
public:
    static inline std::unique_ptr<VpnSqliteStatement> vpnSqliteStatement_ = nullptr;
    static void SetUpTestSuite();
};

void VpnSqliteStatementTest::SetUpTestSuite()
{
    vpnSqliteStatement_ = std::make_unique<VpnSqliteStatement>();
}

HWTEST_F(VpnSqliteStatementTest, Prepare, TestSize.Level1)
{
    sqlite3 *sqlite_ = nullptr;
    std::string sql = "SELECT COUNT(*) FROM T_vpn_config";
    int32_t ret = sqlite3_open_v2("/data/service/el1/public/netmanager/vpn_data.db", &sqlite_,
    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
    EXPECT_EQ(vpnSqliteStatement_->Prepare(sqlite_, sql), SQLITE_OK);
}

HWTEST_F(VpnSqliteStatementTest, BindInt32, TestSize.Level1)
{
    int index = 1;
    int value = 100;
    EXPECT_NE(vpnSqliteStatement_->BindInt32(index, value), SQLITE_OK);
}

HWTEST_F(VpnSqliteStatementTest, BindText, TestSize.Level1)
{
    int index = 1;
    std::string value = "test";
    EXPECT_NE(vpnSqliteStatement_->BindText(index, value), SQLITE_OK);
}

HWTEST_F(VpnSqliteStatementTest, Finalize, TestSize.Level1)
{
    vpnSqliteStatement_->Finalize();
}

HWTEST_F(VpnSqliteStatementTest, ResetStatementAndClearBindings, TestSize.Level1)
{
    vpnSqliteStatement_->ResetStatementAndClearBindings();
}

HWTEST_F(VpnSqliteStatementTest, Step, TestSize.Level1)
{
    EXPECT_NE(vpnSqliteStatement_->Step(), SQLITE_OK);
}

HWTEST_F(VpnSqliteStatementTest, GetColumnString, TestSize.Level1)
{
    int index = 1;
    std::string value = "test";
    EXPECT_NE(vpnSqliteStatement_->GetColumnString(index, value), SQLITE_OK);
}

HWTEST_F(VpnSqliteStatementTest, GetColumnLong, TestSize.Level1)
{
    int index = 1;
    uint64_t value = 100;
    EXPECT_NE(vpnSqliteStatement_->GetColumnLong(index, value), SQLITE_OK);
}

HWTEST_F(VpnSqliteStatementTest, GetColumnInt, TestSize.Level1)
{
    int index = 1;
    int32_t value = 100;
    EXPECT_NE(vpnSqliteStatement_->GetColumnInt(index, value), SQLITE_OK);
}

HWTEST_F(VpnSqliteStatementTest, GetColumnCount, TestSize.Level1)
{
    EXPECT_NE(vpnSqliteStatement_->GetColumnCount(), SQLITE_OK);
}
}
}