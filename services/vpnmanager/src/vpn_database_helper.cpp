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

#include "vpn_database_helper.h"

#include <cstdlib>
#include <filesystem>

#include "net_manager_constants.h"
#include "net_manager_ext_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "vpn_database_defines.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace VpnDatabaseDefines;

VpnDatabaseHelper &VpnDatabaseHelper::GetInstance()
{
    static VpnDatabaseHelper instance;
    return instance;
}

VpnDatabaseHelper::VpnDatabaseHelper()
{
    if (!std::filesystem::exists(VPN_DATABASE_PATH)) {
        std::error_code ec;
        if (std::filesystem::create_directories(VPN_DATABASE_PATH, ec)) {
            NETMGR_EXT_LOG_D("create_directories success :%{public}s", VPN_DATABASE_PATH.c_str());
        } else {
            NETMGR_EXT_LOG_E("create_directories error :%{public}s : %s", VPN_DATABASE_PATH.c_str(),
                ec.message().c_str());
        }
    }
    std::string vpnDatabaseName = VPN_DATABASE_PATH + VPN_DB_NAME;
    int32_t errCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbStoreConfig config(vpnDatabaseName);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    VpnDataBaseCallBack sqliteOpenHelperCallback;
    store_ = OHOS::NativeRdb::RdbHelper::GetRdbStore(config, DATABASE_OPEN_VERSION, sqliteOpenHelperCallback, errCode);
    if (errCode != OHOS::NativeRdb::E_OK && errCode != OHOS::NativeRdb::E_SQLITE_CORRUPT) {
        NETMGR_EXT_LOG_E("GetRdbStore failed. errCode :%{public}d", errCode);
    } else {
        NETMGR_EXT_LOG_I("GetRdbStore success");
    }
}

int32_t VpnDataBaseCallBack::OnCreate(OHOS::NativeRdb::RdbStore &store)
{
    NETMGR_EXT_LOG_I("DB OnCreate Enter");
    std::string sql =
        "CREATE TABLE IF NOT EXISTS " + VPN_CONFIG_TABLE + "(" + std::string(VPN_CONFIG_TABLE_CREATE_PARAM) + ");";
    int32_t ret = store.ExecuteSql(sql);
    if (ret != OHOS::NativeRdb::E_OK) {
        NETMGR_EXT_LOG_E("Create table failed: %{public}d", ret);
        return NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t VpnDataBaseCallBack::OnUpgrade(OHOS::NativeRdb::RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    NETMGR_EXT_LOG_I("DB OnUpgrade Enter");
    return NETMANAGER_EXT_SUCCESS;
}

int32_t VpnDataBaseCallBack::OnDowngrade(OHOS::NativeRdb::RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    NETMGR_EXT_LOG_I("DB OnDowngrade Enter");
    return NETMANAGER_EXT_SUCCESS;
}

int32_t VpnDatabaseHelper::InsertOrUpdateData(const sptr<VpnDataBean> &vpnBean)
{
    if (vpnBean == nullptr) {
        NETMGR_EXT_LOG_E("InsertOrUpdateData vpnBean is nullptr");
        return NETMANAGER_EXT_ERR_INVALID_PARAMETER;
    }

    if (IsVpnInfoExists(vpnBean->vpnId_)) {
        return UpdateData(vpnBean);
    }
    return InsertData(vpnBean);
}

bool VpnDatabaseHelper::IsVpnInfoExists(const std::string &vpnId)
{
    if (vpnId.empty()) {
        NETMGR_EXT_LOG_E("IsVpnInfoExists vpnId is empty");
        return false;
    }
    if (store_ == nullptr) {
        NETMGR_EXT_LOG_E("IsVpnInfoExists store_ is nullptr");
        return false;
    }

    std::vector<std::string> columns;
    OHOS::NativeRdb::RdbPredicates rdbPredicate{ VPN_CONFIG_TABLE };
    rdbPredicate.EqualTo(VPN_ID, vpnId);
    auto queryResultSet = store_->Query(rdbPredicate, columns);
    if (queryResultSet == nullptr) {
        NETMGR_EXT_LOG_E("Query error");
        return false;
    }

    int32_t rowCount = 0;
    int32_t ret = queryResultSet->GetRowCount(rowCount);
    if (ret != OHOS::NativeRdb::E_OK) {
        NETMGR_EXT_LOG_E("get row count failed, ret:%{public}d", ret);
        return false;
    }
    return rowCount == 1;
}

void VpnDatabaseHelper::BindVpnData(NativeRdb::ValuesBucket &values, const sptr<VpnDataBean> &info)
{
    if (info == nullptr) {
        NETMGR_EXT_LOG_E("BindVpnData info is nullptr");
        return;
    }
    values.PutString(VPN_ID, info->vpnId_);
    values.PutString(VPN_NAME, info->vpnName_);
    values.PutInt(VPN_TYPE, info->vpnType_);
    values.PutString(VPN_ADDRESS, info->vpnAddress_);
    values.PutString(USER_NAME, info->userName_);
    values.PutString(PASSWORD, info->password_);
    values.PutInt(USER_ID, info->userId_);
    values.PutInt(VPN_IS_LEGACY, info->isLegacy_);
    values.PutInt(VPN_SAVE_LOGIN, info->saveLogin_);
    values.PutString(VPN_FORWARDED_ROUTES, info->forwardingRoutes_);
    values.PutString(VPN_DNS_ADDRESSES, info->dnsAddresses_);
    values.PutString(VPN_SEARCH_DOMAINS, info->searchDomains_);

    values.PutString(OPEN_VPN_PORT, info->ovpnPort_);
    values.PutInt(OPEN_VPN_PROTOCOL, info->ovpnProtocol_);
    values.PutString(OPEN_VPN_CFG, info->ovpnConfig_);
    values.PutInt(OPEN_VPN_AUTH_TYPE, info->ovpnAuthType_);
    values.PutString(OPEN_VPN_ASKPASS, info->askpass_);
    values.PutString(OPEN_VPN_CFG_FILE_PATH, info->ovpnConfigFilePath_);
    values.PutString(OPEN_VPN_CA_CERT_FILE_PATH, info->ovpnCaCertFilePath_);
    values.PutString(OPEN_VPN_USER_CERT_FILE_PATH, info->ovpnUserCertFilePath_);
    values.PutString(OPEN_VPN_PRIVATE_KEY_FILE_PATH, info->ovpnPrivateKeyFilePath_);

    values.PutString(IPSEC_PRE_SHARE_KEY, info->ipsecPreSharedKey_);
    values.PutString(IPSEC_IDENTIFIER, info->ipsecIdentifier_);
    values.PutString(SWANCTL_CONF, info->swanctlConf_);
    values.PutString(STRONGSWAN_CONF, info->strongswanConf_);
    values.PutString(IPSEC_CA_CERT_CONF, info->ipsecCaCertConf_);
    values.PutString(IPSEC_PRIVATE_USER_CERT_CONF, info->ipsecPrivateUserCertConf_);
    values.PutString(IPSEC_PUBLIC_USER_CERT_CONF, info->ipsecPublicUserCertConf_);
    values.PutString(IPSEC_PRIVATE_SERVER_CERT_CONF, info->ipsecPrivateServerCertConf_);
    values.PutString(IPSEC_PUBLIC_SERVER_CERT_CONF, info->ipsecPublicServerCertConf_);
    values.PutString(IPSEC_CA_CERT_FILE_PATH, info->ipsecCaCertFilePath_);
    values.PutString(IPSEC_PRIVATE_USER_CERT_FILE_PATH, info->ipsecPrivateUserCertFilePath_);
    values.PutString(IPSEC_PUBLIC_USER_CERT_FILE_PATH, info->ipsecPublicUserCertFilePath_);
    values.PutString(IPSEC_PRIVATE_SERVER_CERT_FILE_PATH, info->ipsecPrivateServerCertFilePath_);
    values.PutString(IPSEC_PUBLIC_SERVER_CERT_FILE_PATH, info->ipsecPublicServerCertFilePath_);
    values.PutString(IPSEC_CONF, info->ipsecConf_);
    values.PutString(IPSEC_SECRETS, info->ipsecSecrets_);
    values.PutString(OPTIONS_L2TPD_CLIENT, info->optionsL2tpdClient_);
    values.PutString(XL2TPD_CONF, info->xl2tpdConf_);
    values.PutString(L2TP_SHARED_KEY, info->l2tpSharedKey_);
}

int32_t VpnDatabaseHelper::InsertData(const sptr<VpnDataBean> &vpnBean)
{
    NETMGR_EXT_LOG_I("InsertData");
    if (vpnBean == nullptr) {
        NETMGR_EXT_LOG_E("InsertData vpnBean is nullptr");
        return NETMANAGER_EXT_ERR_INVALID_PARAMETER;
    }
    if (store_ == nullptr) {
        NETMGR_EXT_LOG_E("InsertData store_ is nullptr");
        return NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }

    NativeRdb::ValuesBucket values;
    BindVpnData(values, vpnBean);
    int64_t rowId = 0;
    int ret = store_->Insert(rowId, VPN_CONFIG_TABLE, values);
    if (ret != NativeRdb::E_OK) {
        NETMGR_EXT_LOG_E("InsertData failed, result is %{public}d", ret);
        return NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t VpnDatabaseHelper::UpdateData(const sptr<VpnDataBean> &vpnBean)
{
    if (store_ == nullptr) {
        NETMGR_EXT_LOG_E("UpdateData store_ is nullptr");
        return NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }
    if (vpnBean == nullptr) {
        NETMGR_EXT_LOG_E("UpdateData vpnBean is nullptr");
        return NETMANAGER_EXT_ERR_INVALID_PARAMETER;
    }
    NETMGR_EXT_LOG_I("UpdateData");
    OHOS::NativeRdb::RdbPredicates rdbPredicate{ VPN_CONFIG_TABLE };
    rdbPredicate.EqualTo(VPN_ID, vpnBean->vpnId_);
    NativeRdb::ValuesBucket values;
    BindVpnData(values, vpnBean);
    int32_t rowId = -1;
    int32_t ret = store_->Update(rowId, values, rdbPredicate);
    if (ret != OHOS::NativeRdb::E_OK) {
        NETMGR_EXT_LOG_E("UpdateData ret :%{public}d", ret);
        return NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }
    return NETMANAGER_EXT_SUCCESS;
}

void VpnDatabaseHelper::GetVpnDataFromResultSet(const std::shared_ptr<OHOS::NativeRdb::ResultSet> &queryResultSet,
    sptr<VpnDataBean> &vpnBean)
{
    queryResultSet->GetString(INDEX_VPN_ID, vpnBean->vpnId_);
    queryResultSet->GetString(INDEX_VPN_NAME, vpnBean->vpnName_);
    queryResultSet->GetInt(INDEX_VPN_TYPE, vpnBean->vpnType_);
    queryResultSet->GetString(INDEX_VPN_ADDRESS, vpnBean->vpnAddress_);
    queryResultSet->GetString(INDEX_USER_NAME, vpnBean->userName_);
    queryResultSet->GetString(INDEX_PASSWORD, vpnBean->password_);
    queryResultSet->GetInt(INDEX_USER_ID, vpnBean->userId_);
    queryResultSet->GetInt(INDEX_VPN_IS_LEGACY, vpnBean->isLegacy_);
    queryResultSet->GetInt(INDEX_VPN_SAVE_LOGIN, vpnBean->saveLogin_);
    queryResultSet->GetString(INDEX_VPN_FORWARDED_ROUTES, vpnBean->forwardingRoutes_);
    queryResultSet->GetString(INDEX_VPN_DNS_ADDRESSES, vpnBean->dnsAddresses_);
    queryResultSet->GetString(INDEX_VPN_SEARCH_DOMAINS, vpnBean->searchDomains_);
    queryResultSet->GetString(INDEX_OPEN_VPN_PORT, vpnBean->ovpnPort_);
    queryResultSet->GetInt(INDEX_OPEN_VPN_PROTOCOL, vpnBean->ovpnProtocol_);
    queryResultSet->GetString(INDEX_OPEN_VPN_CFG, vpnBean->ovpnConfig_);
    queryResultSet->GetInt(INDEX_OPEN_VPN_AUTH_TYPE, vpnBean->ovpnAuthType_);
    queryResultSet->GetString(INDEX_OPEN_VPN_ASKPASS, vpnBean->askpass_);
    queryResultSet->GetString(INDEX_OPEN_VPN_CFG_FILE_PATH, vpnBean->ovpnConfigFilePath_);
    queryResultSet->GetString(INDEX_OPEN_VPN_CA_CERT_FILE_PATH, vpnBean->ovpnCaCertFilePath_);
    queryResultSet->GetString(INDEX_OPEN_VPN_USER_CERT_FILE_PATH, vpnBean->ovpnUserCertFilePath_);
    queryResultSet->GetString(INDEX_OPEN_VPN_PRIVATE_KEY_FILE_PATH, vpnBean->ovpnPrivateKeyFilePath_);

    queryResultSet->GetString(INDEX_IPSEC_PRE_SHARE_KEY, vpnBean->ipsecPreSharedKey_);
    queryResultSet->GetString(INDEX_IPSEC_IDENTIFIER, vpnBean->ipsecIdentifier_);
    queryResultSet->GetString(INDEX_SWANCTL_CONF, vpnBean->swanctlConf_);
    queryResultSet->GetString(INDEX_STRONGSWAN_CONF, vpnBean->strongswanConf_);
    queryResultSet->GetString(INDEX_IPSEC_CA_CERT_CONF, vpnBean->ipsecCaCertConf_);
    queryResultSet->GetString(INDEX_IPSEC_PRIVATE_USER_CERT_CONF, vpnBean->ipsecPrivateUserCertConf_);
    queryResultSet->GetString(INDEX_IPSEC_PUBLIC_USER_CERT_CONF, vpnBean->ipsecPublicUserCertConf_);
    queryResultSet->GetString(INDEX_IPSEC_PRIVATE_SERVER_CERT_CONF, vpnBean->ipsecPrivateServerCertConf_);
    queryResultSet->GetString(INDEX_IPSEC_PUBLIC_SERVER_CERT_CONF, vpnBean->ipsecPublicServerCertConf_);
    queryResultSet->GetString(INDEX_IPSEC_CA_CERT_FILE_PATH, vpnBean->ipsecCaCertFilePath_);
    queryResultSet->GetString(INDEX_IPSEC_PRIVATE_USER_CERT_FILE_PATH, vpnBean->ipsecPrivateUserCertFilePath_);
    queryResultSet->GetString(INDEX_IPSEC_PUBLIC_USER_CERT_FILE_PATH, vpnBean->ipsecPublicUserCertFilePath_);
    queryResultSet->GetString(INDEX_IPSEC_PRIVATE_SERVER_CERT_FILE_PATH, vpnBean->ipsecPrivateServerCertFilePath_);
    queryResultSet->GetString(INDEX_IPSEC_PUBLIC_SERVER_CERT_FILE_PATH, vpnBean->ipsecPublicServerCertFilePath_);
    queryResultSet->GetString(INDEX_IPSEC_CONF, vpnBean->ipsecConf_);
    queryResultSet->GetString(INDEX_IPSEC_SECRETS, vpnBean->ipsecSecrets_);
    queryResultSet->GetString(INDEX_OPTIONS_L2TPD_CLIENT, vpnBean->optionsL2tpdClient_);
    queryResultSet->GetString(INDEX_XL2TPD_CONF, vpnBean->xl2tpdConf_);
    queryResultSet->GetString(INDEX_L2TP_SHARED_KEY, vpnBean->l2tpSharedKey_);
}

int32_t VpnDatabaseHelper::QueryVpnData(sptr<VpnDataBean> &vpnBean, const std::string &vpnUuid)
{
    if (vpnBean == nullptr) {
        NETMGR_EXT_LOG_E("QueryVpnData vpnBean is nullptr");
        return NETMANAGER_EXT_ERR_INVALID_PARAMETER;
    }
    if (vpnUuid.empty()) {
        NETMGR_EXT_LOG_E("QueryVpnData vpnUuid is empty");
        return NETMANAGER_EXT_ERR_INVALID_PARAMETER;
    }

    NETMGR_EXT_LOG_I("QueryVpnData vpnUuid=%{public}s", vpnUuid.c_str());
    if (store_ == nullptr) {
        NETMGR_EXT_LOG_E("QueryVpnData store_ is nullptr");
        return NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }

    std::vector<std::string> columns;
    OHOS::NativeRdb::RdbPredicates rdbPredicate{ VPN_CONFIG_TABLE };
    rdbPredicate.EqualTo(VPN_ID, vpnUuid);
    auto queryResultSet = store_->Query(rdbPredicate, columns);
    if (queryResultSet == nullptr) {
        NETMGR_EXT_LOG_E("QueryVpnData error");
        return NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }
    int32_t rowCount = 0;
    int ret = queryResultSet->GetRowCount(rowCount);
    if (ret != OHOS::NativeRdb::E_OK) {
        NETMGR_EXT_LOG_E("QueryVpnData failed, get row count failed, ret:%{public}d", ret);
        return ret;
    }
    if (rowCount == 0) {
        NETMGR_EXT_LOG_E("QueryVpnData result num is 0");
        return NETMANAGER_EXT_SUCCESS;
    }
    while (!queryResultSet->GoToNextRow()) {
        GetVpnDataFromResultSet(queryResultSet, vpnBean);
        if (vpnBean->vpnId_ == vpnUuid) {
            return NETMANAGER_SUCCESS;
        }
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t VpnDatabaseHelper::QueryAllData(std::vector<SysVpnConfig> &infos, const int32_t userId)
{
    NETMGR_EXT_LOG_I("QueryAllData");
    if (store_ == nullptr) {
        NETMGR_EXT_LOG_E("QueryAllData store_ is nullptr");
        return NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }
    infos.clear();
    std::vector<std::string> columns;
    OHOS::NativeRdb::RdbPredicates rdbPredicate{ VPN_CONFIG_TABLE };
    rdbPredicate.EqualTo(USER_ID, userId);
    auto queryResultSet = store_->Query(rdbPredicate, columns);
    if (queryResultSet == nullptr) {
        NETMGR_EXT_LOG_E("QueryAllData error");
        return NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }
    int32_t rowCount = 0;
    int ret = queryResultSet->GetRowCount(rowCount);
    if (ret != OHOS::NativeRdb::E_OK) {
        NETMGR_EXT_LOG_E("QueryAllData failed, get row count failed, ret:%{public}d", ret);
        return ret;
    }
    if (rowCount == 0) {
        NETMGR_EXT_LOG_E("QueryAllData result num is 0");
        return NETMANAGER_EXT_SUCCESS;
    }
    while (!queryResultSet->GoToNextRow()) {
        SysVpnConfig info;
        queryResultSet->GetString(INDEX_VPN_ID, info.vpnId_);
        queryResultSet->GetString(INDEX_VPN_NAME, info.vpnName_);
        infos.emplace_back(info);
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t VpnDatabaseHelper::DeleteVpnData(const std::string &vpnUuid)
{
    NETMGR_EXT_LOG_I("DeleteVpnData");
    if (vpnUuid.empty()) {
        NETMGR_EXT_LOG_E("DeleteVpnData vpnUuid is empty");
        return NETMANAGER_EXT_ERR_INVALID_PARAMETER;
    }

    if (store_ == nullptr) {
        NETMGR_EXT_LOG_E("DeleteVpnData store_ is nullptr");
        return NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }
    int32_t deletedRows = -1;
    OHOS::NativeRdb::RdbPredicates rdbPredicate{ VPN_CONFIG_TABLE };
    rdbPredicate.EqualTo(VPN_ID, vpnUuid);
    int32_t result = store_->Delete(deletedRows, rdbPredicate);
    if (result != NativeRdb::E_OK) {
        NETMGR_EXT_LOG_E("DeleteVpnData failed, result is %{public}d", result);
        return NETMANAGER_EXT_ERR_OPERATION_FAILED;
    }
    return NETMANAGER_EXT_SUCCESS;
}
} // namespace NetManagerStandard
} // namespace OHOS
