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

#ifndef VPN_DATABASE_HELPER_H
#define VPN_DATABASE_HELPER_H

#include <climits>
#include <functional>
#include <string>

#ifndef USE_SQLITE_SYMBOLS
#include "sqlite3.h"
#else
#include "sqlite3sym.h"
#endif

#include "vpn_sqlite_statement.h"
#include "vpn_data_bean.h"
#include "ipsecvpn_config.h"
#include "l2tpvpn_config.h"

namespace OHOS {
namespace NetManagerStandard {
class VpnDatabaseHelper {
public:
    using SqlCallback = sqlite3_callback;
    explicit VpnDatabaseHelper(const std::string &path);
    VpnDatabaseHelper() = delete;
    ~VpnDatabaseHelper();

    int32_t CreateTable();
    int32_t InsertData(const sptr<VpnDataBean> &vpnBean);
    int32_t InsertOrUpdateData(const sptr<VpnDataBean> &vpnBean);
    bool IsVpnInfoExists(std::string &vpnId, int32_t &userId);
    int32_t QueryVpnData(sptr<VpnDataBean> &vpnBean, const std::string &vpnUuid, const int32_t userId);
    int32_t QueryAllData(std::vector<SysVpnConfig> &infos, const int32_t userId);
    int32_t DeleteVpnData(const std::string &vpnUuid, const int32_t userId);
    int32_t UpdateData(const sptr<VpnDataBean> &vpnBean);
    int32_t Step(std::vector<VpnDataBean> &vpnBean);
    int32_t ExecSql(const std::string &sql, void *recv, SqlCallback callback);
    void initStatementQuery(std::vector<VpnDataBean> &infos);
    void bindUpdateData(const sptr<VpnDataBean> &info);
    void bindInsertData(const sptr<VpnDataBean> &info);

private:
    int32_t Open(const std::string &path);
    int32_t Close();
    sqlite3 *sqlite_ = nullptr;
    VpnSqliteStatement statement_;
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif // VPN_DATABASE_HELPER_H
