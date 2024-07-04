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

#include "vpn_data_bean.h"
#include "ipsecvpn_config.h"
#include "l2tpvpn_config.h"
#include "rdb_common.h"
#include "rdb_errno.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_predicates.h"
#include "rdb_store.h"
#include "result_set.h"

namespace OHOS {
namespace NetManagerStandard {
class VpnDatabaseHelper {
public:
    VpnDatabaseHelper();
    ~VpnDatabaseHelper() = default;

    int32_t InsertData(const sptr<VpnDataBean> &vpnBean);
    int32_t InsertOrUpdateData(const sptr<VpnDataBean> &vpnBean);
    bool IsVpnInfoExists(std::string &vpnId);
    int32_t QueryVpnData(sptr<VpnDataBean> &vpnBean, const std::string &vpnUuid);
    int32_t QueryAllData(std::vector<SysVpnConfig> &infos, const int32_t userId);
    int32_t DeleteVpnData(const std::string &vpnUuid);
    int32_t UpdateData(const sptr<VpnDataBean> &vpnBean);

private:
    void getVpnDataFromResultSet(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &queryResultSet,
        sptr<VpnDataBean> &vpnBean);
    void bindVpnData(NativeRdb::ValuesBucket &values, const sptr<VpnDataBean> &info);
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store_;
};

class VpnDataBaseCallBack : public OHOS::NativeRdb::RdbOpenCallback {
public:
    int32_t OnCreate(OHOS::NativeRdb::RdbStore &rdbStore) override;

    int32_t OnUpgrade(OHOS::NativeRdb::RdbStore &rdbStore, int32_t oldVersion, int32_t newVersion) override;

    int32_t OnDowngrade(OHOS::NativeRdb::RdbStore &rdbStore, int32_t currentVersion, int32_t targetVersion) override;
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif // VPN_DATABASE_HELPER_H
