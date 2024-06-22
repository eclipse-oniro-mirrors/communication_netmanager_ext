/*
 * Copyright (c) 20224 Huawei Device Co., Ltd.
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

#include "vpn_database_helper.h"

#include <cstdlib>
#include <filesystem>

#include "net_manager_constants.h"
#include "netmgr_ext_log_wrapper.h"
#include "vpn_database_defines.h"
#include "net_manager_ext_constants.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace VpnDatabaseDefines;
namespace {
VpnDatabaseHelper::SqlCallback sqlCallback = [](void *notUsed, int argc, char **argv, char **colName) {
    std::string data;
    for (int i = 0; i < argc; i++) {
        data.append(colName[i]).append(" = ").append(argv[i] ? argv[i] : "nullptr\n");
    }
    NETMGR_EXT_LOG_D("Recv data: %{public}s", data.c_str());
    return 0;
};

bool CheckFilePath(const std::string &fileName)
{
    char tmpPath[PATH_MAX] = {0};
    const auto pos = fileName.find_last_of('/');
    const auto dir = fileName.substr(0, pos);
    if (!realpath(dir.c_str(), tmpPath)) {
        NETMGR_EXT_LOG_E("Get realPath failed error: %{public}d, %{public}s", errno, strerror(errno));
        return false;
    }
    if (strcmp(tmpPath, dir.c_str()) != 0) {
        NETMGR_EXT_LOG_E("file name is illegal fileName: %{public}s, tmpPath: %{public}s",
            fileName.c_str(), tmpPath);
        return false;
    }
    return true;
}
} // namespace

VpnDatabaseHelper::VpnDatabaseHelper(const std::string &path)
{
    if (!CheckFilePath(path)) {
        return;
    }
    Open(path);
}

VpnDatabaseHelper::~VpnDatabaseHelper()
{
    Close();
    sqlite_ = nullptr;
}

int32_t VpnDatabaseHelper::ExecSql(const std::string &sql, void *recv, SqlCallback callback)
{
    char *errMsg = nullptr;
    NETMGR_EXT_LOG_I("ExecSql sql = %{public}s", sql.c_str());
    int32_t ret = sqlite3_exec(sqlite_, sql.c_str(), callback, recv, &errMsg);
    if (errMsg != nullptr) {
        NETMGR_EXT_LOG_E("Exec sql failed err:%{public}s", errMsg);
        sqlite3_free(errMsg);
    }
    return ret == SQLITE_OK ? NETMANAGER_SUCCESS : NETMANAGER_ERROR;
}

int32_t VpnDatabaseHelper::CreateTable()
{
    std::string sql = "CREATE TABLE IF NOT EXISTS " + std::string(VPN_CONFIG_TABLE)
        + "(" + std::string(VPN_CONFIG_TABLE_CREATE_PARAM) + ");";
    int32_t ret = ExecSql(sql, nullptr, sqlCallback);
    if (ret != NETMANAGER_EXT_SUCCESS) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    return NETMANAGER_EXT_SUCCESS;
}

int32_t VpnDatabaseHelper::Open(const std::string &path)
{
    int32_t ret = sqlite3_open_v2(path.c_str(), &sqlite_,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
    return ret == SQLITE_OK ? NETMANAGER_SUCCESS : NETMANAGER_ERROR;
}

int32_t VpnDatabaseHelper::InsertOrUpdateData(const sptr<VpnDataBean> &vpnBean)
{
    if (IsVpnInfoExists(vpnBean->vpnId_, vpnBean->userId_)) {
        return UpdateData(vpnBean);
    }
    return InsertData(vpnBean);
}


bool VpnDatabaseHelper::IsVpnInfoExists(std::string &vpnId, int32_t &userId)
{
    std::string sql = "SELECT COUNT(*) FROM " + std::string(VPN_CONFIG_TABLE)
        + " WHERE " + "vpnId = ? AND userId = ?";
    int32_t ret = statement_.Prepare(sqlite_, sql);
    if (ret != SQLITE_OK) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    int exists = 0;
    int32_t idx = 1;
    statement_.BindText(idx, vpnId);
    statement_.BindInt32(++idx, userId);
    if (statement_.Step() == SQLITE_ROW) {
        statement_.GetColumnInt(0, exists);
    }
    statement_.ResetStatementAndClearBindings();
    return exists > 0;
}

int32_t VpnDatabaseHelper::InsertData(const sptr<VpnDataBean> &info)
{
    std::string paramList = VPN_CONFIG_TABLE_PARAM_LIST;
    std::string params;
    int32_t paramCount = count(paramList.begin(), paramList.end(), ',') + 1;
    for (int32_t i = 0; i < paramCount; ++i) {
        params += "?";
        if (i != paramCount - 1) {
            params += ",";
        }
    }
    std::string sql = "INSERT INTO " + std::string(VPN_CONFIG_TABLE)
        + " (" + paramList + ") " + "VALUES" + " (" + params + ") ";
    int32_t ret = statement_.Prepare(sqlite_, sql);
    if (ret != SQLITE_OK) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    int32_t idx = 1;
    statement_.BindText(idx, info->vpnId_);
    statement_.BindText(++idx, info->vpnName_);
    statement_.BindInt32(++idx, info->vpnType_);
    statement_.BindText(++idx, info->vpnAddress_);
    statement_.BindText(++idx, info->userName_);
    statement_.BindText(++idx, info->password_);
    statement_.BindInt32(++idx, info->userId_);
    statement_.BindInt32(++idx, info->isLegacy_);
    statement_.BindInt32(++idx, info->saveLogin_);
    statement_.BindText(++idx, info->forwardingRoutes_);
    statement_.BindText(++idx, info->dnsAddresses_);
    statement_.BindText(++idx, info->searchDomains_);
    statement_.BindText(++idx, info->ovpnPort_);
    statement_.BindInt32(++idx, info->ovpnProtocol_);
    statement_.BindText(++idx, info->ovpnConfig_);
    statement_.BindInt32(++idx, info->ovpnAuthType_);
    statement_.BindText(++idx, info->askpass_);
    statement_.BindText(++idx, info->ovpnConfigFilePath_);
    statement_.BindText(++idx, info->ovpnCaCertFilePath_);
    statement_.BindText(++idx, info->ovpnUserCertFilePath_);
    statement_.BindText(++idx, info->ovpnPrivateKeyFilePath_);
    statement_.BindText(++idx, info->ipsecPreSharedKey_);
    statement_.BindText(++idx, info->ipsecIdentifier_);
    statement_.BindText(++idx, info->swanctlConf_);
    statement_.BindText(++idx, info->strongswanConf_);
    statement_.BindText(++idx, info->ipsecCaCertConf_);
    statement_.BindText(++idx, info->ipsecPrivateUserCertConf_);
    statement_.BindText(++idx, info->ipsecPublicUserCertConf_);
    statement_.BindText(++idx, info->ipsecPrivateServerCertConf_);
    statement_.BindText(++idx, info->ipsecPublicServerCertConf_);
    statement_.BindText(++idx, info->ipsecCaCertFilePath_);
    statement_.BindText(++idx, info->ipsecPrivateUserCertFilePath_);
    statement_.BindText(++idx, info->ipsecPublicUserCertFilePath_);
    statement_.BindText(++idx, info->ipsecPrivateServerCertFilePath_);
    statement_.BindText(++idx, info->ipsecPublicServerCertFilePath_);
    statement_.BindText(++idx, info->ipsecConf_);
    statement_.BindText(++idx, info->ipsecSecrets_);
    statement_.BindText(++idx, info->optionsL2tpdClient_);
    statement_.BindText(++idx, info->xl2tpdConf_);
    statement_.BindText(++idx, info->l2tpSharedKey_);
    ret = statement_.Step();
    statement_.ResetStatementAndClearBindings();
    if (ret != SQLITE_DONE) {
        NETMGR_EXT_LOG_E("Step failed ret:%{public}d", ret);
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t VpnDatabaseHelper::UpdateData(const sptr<VpnDataBean> &info)
{
    std::string sql = "UPDATE " + std::string(VPN_CONFIG_TABLE)
        + " SET " + std::string(VPN_CONFIG_TABLE_UPDATE_SQL) + " WHERE "
        + "vpnId = ? AND userId = ?";
    int32_t ret = statement_.Prepare(sqlite_, sql);
    if (ret != SQLITE_OK) {
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    int32_t idx = 1;
    statement_.BindText(idx, info->vpnName_);
    statement_.BindInt32(++idx, info->vpnType_);
    statement_.BindText(++idx, info->vpnAddress_);
    statement_.BindText(++idx, info->userName_);
    statement_.BindText(++idx, info->password_);
    statement_.BindInt32(++idx, info->isLegacy_);
    statement_.BindInt32(++idx, info->saveLogin_);
    statement_.BindText(++idx, info->forwardingRoutes_);
    statement_.BindText(++idx, info->dnsAddresses_);
    statement_.BindText(++idx, info->searchDomains_);
    statement_.BindText(++idx, info->ovpnPort_);
    statement_.BindInt32(++idx, info->ovpnProtocol_);
    statement_.BindText(++idx, info->ovpnConfig_);
    statement_.BindInt32(++idx, info->ovpnAuthType_);
    statement_.BindText(++idx, info->askpass_);
    statement_.BindText(++idx, info->ovpnConfigFilePath_);
    statement_.BindText(++idx, info->ovpnCaCertFilePath_);
    statement_.BindText(++idx, info->ovpnUserCertFilePath_);
    statement_.BindText(++idx, info->ovpnPrivateKeyFilePath_);
    statement_.BindText(++idx, info->ipsecPreSharedKey_);
    statement_.BindText(++idx, info->ipsecIdentifier_);
    statement_.BindText(++idx, info->swanctlConf_);
    statement_.BindText(++idx, info->strongswanConf_);
    statement_.BindText(++idx, info->ipsecCaCertConf_);
    statement_.BindText(++idx, info->ipsecPrivateUserCertConf_);
    statement_.BindText(++idx, info->ipsecPublicUserCertConf_);
    statement_.BindText(++idx, info->ipsecPrivateServerCertConf_);
    statement_.BindText(++idx, info->ipsecPublicServerCertConf_);
    statement_.BindText(++idx, info->ipsecCaCertFilePath_);
    statement_.BindText(++idx, info->ipsecPrivateUserCertFilePath_);
    statement_.BindText(++idx, info->ipsecPublicUserCertFilePath_);
    statement_.BindText(++idx, info->ipsecPrivateServerCertFilePath_);
    statement_.BindText(++idx, info->ipsecPublicServerCertFilePath_);
    statement_.BindText(++idx, info->ipsecConf_);
    statement_.BindText(++idx, info->ipsecSecrets_);
    statement_.BindText(++idx, info->optionsL2tpdClient_);
    statement_.BindText(++idx, info->xl2tpdConf_);
    statement_.BindText(++idx, info->l2tpSharedKey_);
    statement_.BindText(++idx, info->vpnId_);
    statement_.BindInt32(++idx, info->userId_);
    ret = statement_.Step();
    statement_.ResetStatementAndClearBindings();
    if (ret != SQLITE_DONE) {
        NETMGR_EXT_LOG_E("Step failed ret:%{public}d", ret);
        return NETMANAGER_ERR_WRITE_DATA_FAIL;
    }
    return NETMANAGER_SUCCESS;
}

int32_t VpnDatabaseHelper::QueryVpnData(sptr<VpnDataBean> &vpnBean, const std::string &vpnUuid, const int32_t userId)
{
    std::string sql = "SELECT * FROM " + std::string(VPN_CONFIG_TABLE) + " WHERE vpnId = '" + vpnUuid + "'"
        + " AND userId = " + std::to_string(userId);
    int32_t ret = statement_.Prepare(sqlite_, sql);
    if (ret != SQLITE_OK) {
        NETMGR_EXT_LOG_E("Prepare failed ret:%{public}d", ret);
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    std::vector<VpnDataBean> infos;
    Step(infos);
    if (!infos.empty()) {
        vpnBean = sptr<VpnDataBean>::MakeSptr(infos[0]);
    }
    return NETMANAGER_SUCCESS;
}

int32_t VpnDatabaseHelper::QueryAllData(std::vector<SysVpnConfig> &infos, const int32_t userId)
{
    infos.clear();
    std::string sql = "SELECT vpnId, vpnName FROM " + std::string(VPN_CONFIG_TABLE)
        + " WHERE userId = " + std::to_string(userId);
    int32_t ret = statement_.Prepare(sqlite_, sql);
    if (ret != SQLITE_OK) {
        NETMGR_EXT_LOG_E("Prepare failed ret:%{public}d", ret);
        return NETMANAGER_ERR_READ_DATA_FAIL;
    }
    int32_t rc = statement_.Step();
    NETMGR_EXT_LOG_I("Step result:%{public}d", rc);
    while (rc != SQLITE_DONE) {
        int32_t idx = 0;
        SysVpnConfig info;
        statement_.GetColumnString(idx, info.vpnId_);
        statement_.GetColumnString(++idx, info.vpnName_);
        infos.emplace_back(info);
        rc = statement_.Step();
        NETMGR_EXT_LOG_I("Step result:%{public}d", rc);
    }
    statement_.ResetStatementAndClearBindings();
    return NETMANAGER_SUCCESS;
}

int32_t VpnDatabaseHelper::DeleteVpnData(const std::string &vpnUuid, const int32_t userId)
{
    std::string sql = "DELETE FROM " + std::string(VPN_CONFIG_TABLE) + " WHERE vpnId = '"
        + vpnUuid + "'" + " AND userId = " + std::to_string(userId);
    return ExecSql(sql, nullptr, sqlCallback);
}

int32_t VpnDatabaseHelper::Close()
{
    int32_t ret = sqlite3_close_v2(sqlite_);
    return ret == SQLITE_OK ? NETMANAGER_SUCCESS : NETMANAGER_ERROR;
}

int32_t VpnDatabaseHelper::Step(std::vector<VpnDataBean> &infos)
{
    int32_t rc = statement_.Step();
    NETMGR_EXT_LOG_I("Step result:%{public}d", rc);
    while (rc != SQLITE_DONE) {
        int32_t idx = 0;
        VpnDataBean info;
        statement_.GetColumnString(idx, info.vpnId_);
        statement_.GetColumnString(++idx, info.vpnName_);
        statement_.GetColumnInt(++idx, info.vpnType_);
        statement_.GetColumnString(++idx, info.vpnAddress_);
        statement_.GetColumnString(++idx, info.userName_);
        statement_.GetColumnString(++idx, info.password_);
        statement_.GetColumnInt(++idx, info.userId_);
        statement_.GetColumnInt(++idx, info.isLegacy_);
        statement_.GetColumnInt(++idx, info.saveLogin_);
        statement_.GetColumnString(++idx, info.forwardingRoutes_);
        statement_.GetColumnString(++idx, info.dnsAddresses_);
        statement_.GetColumnString(++idx, info.searchDomains_);
        statement_.GetColumnString(++idx, info.ovpnPort_);
        statement_.GetColumnInt(++idx, info.ovpnProtocol_);
        statement_.GetColumnString(++idx, info.ovpnConfig_);
        statement_.GetColumnInt(++idx, info.ovpnAuthType_);
        statement_.GetColumnString(++idx, info.askpass_);
        statement_.GetColumnString(++idx, info.ovpnConfigFilePath_);
        statement_.GetColumnString(++idx, info.ovpnCaCertFilePath_);
        statement_.GetColumnString(++idx, info.ovpnUserCertFilePath_);
        statement_.GetColumnString(++idx, info.ovpnPrivateKeyFilePath_);
        statement_.GetColumnString(++idx, info.ipsecPreSharedKey_);
        statement_.GetColumnString(++idx, info.ipsecIdentifier_);
        statement_.GetColumnString(++idx, info.swanctlConf_);
        statement_.GetColumnString(++idx, info.strongswanConf_);
        statement_.GetColumnString(++idx, info.ipsecCaCertConf_);
        statement_.GetColumnString(++idx, info.ipsecPrivateUserCertConf_);
        statement_.GetColumnString(++idx, info.ipsecPublicUserCertConf_);
        statement_.GetColumnString(++idx, info.ipsecPrivateServerCertConf_);
        statement_.GetColumnString(++idx, info.ipsecPublicServerCertConf_);
        statement_.GetColumnString(++idx, info.ipsecCaCertFilePath_);
        statement_.GetColumnString(++idx, info.ipsecPrivateUserCertFilePath_);
        statement_.GetColumnString(++idx, info.ipsecPublicUserCertFilePath_);
        statement_.GetColumnString(++idx, info.ipsecPrivateServerCertFilePath_);
        statement_.GetColumnString(++idx, info.ipsecPublicServerCertFilePath_);
        statement_.GetColumnString(++idx, info.ipsecConf_);
        statement_.GetColumnString(++idx, info.ipsecSecrets_);
        statement_.GetColumnString(++idx, info.optionsL2tpdClient_);
        statement_.GetColumnString(++idx, info.xl2tpdConf_);
        statement_.GetColumnString(++idx, info.l2tpSharedKey_);
        infos.emplace_back(info);
        rc = statement_.Step();
        NETMGR_EXT_LOG_I("Step result:%{public}d", rc);
    }
    statement_.ResetStatementAndClearBindings();
    return NETMANAGER_SUCCESS;
}

} // namespace NetManagerStandard
} // namespace OHOS
