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

#include "netfirewall_client_fuzzer.h"

#include "http_proxy.h"
#include "inet_addr.h"
#include "net_manager_constants.h"
#include "netmanager_ext_test_security.h"
#include "netmgr_ext_log_wrapper.h"
#include "refbase.h"
#include "singleton.h"
#include "netfirewall_client.h"
#include "netfirewall_proxy.h"
#include "netfirewall_service.h"
#include "netfirewall_database.h"
#include "netfirewall_db_helper.h"
#include "net_firewall_param_check.h"
#include "net_firewall_rule_parse.h"
#include "i_netfirewall_service.h"
#include "mock_i_net_intercept_record_callback_test.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
const uint8_t *g_baseFuzzData = nullptr;
size_t g_baseFuzzSize = 0;
size_t g_baseFuzzPos;
constexpr size_t IFACE_LEN = 5;
constexpr int32_t OLD_VERSION = 1;
constexpr uint8_t IFACE_NAME_BOUNDARY_EXTRA = 2;
}

template <class T> T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (g_baseFuzzData == nullptr || objectSize > g_baseFuzzSize - g_baseFuzzPos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, g_baseFuzzData + g_baseFuzzPos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_baseFuzzPos += objectSize;
    return object;
}

std::string GetStringFromData(int strlen)
{
    char cstr[strlen];
    cstr[strlen - 1] = '\0';
    for (int i = 0; i < strlen - 1; i++) {
        cstr[i] = GetData<char>();
    }
    std::string str(cstr);
    return str;
}

bool WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(NetFirewallStub::GetDescriptor())) {
        return false;
    }
    return true;
}

bool IsDataAndWriteVaild(const uint8_t *data, size_t size, MessageParcel &parcel)
{
    if ((data == nullptr) || (size == 0)) {
        return false;
    }

    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    std::string iface = GetStringFromData(IFACE_LEN);
    WriteInterfaceToken(parcel);
    if (!parcel.WriteString(iface)) {
        return false;
    }

    return true;
}

int32_t OnRemoteRequest(uint32_t code, MessageParcel &data)
{
    MessageParcel reply;
    MessageOption option;
    return DelayedSingleton<NetFirewallService>::GetInstance()->OnRemoteRequest(code, data, reply, option);
}

void SetNetFirewallPolicyTest(const uint8_t *data, size_t size)
{
    MessageParcel parcel;
    if (!IsDataAndWriteVaild(data, size, parcel)) {
        return;
    }
    auto status = std::make_unique<NetFirewallPolicy>();
    if (!status->Marshalling(parcel)) {
        return;
    }
    int32_t userId = GetData<int32_t>();
    parcel.WriteInt32(userId);
    OnRemoteRequest(static_cast<uint32_t>(INetFirewallService::SET_NET_FIREWALL_STATUS), parcel);
}

void RegisterInterceptRecordsCallbackTest(const uint8_t *data, size_t size)
{
    MessageParcel parcel;
    if (!IsDataAndWriteVaild(data, size, parcel)) {
        return;
    }
    auto status = std::make_unique<NetFirewallPolicy>();
    if (!status->Marshalling(parcel)) {
        return;
    }
    sptr<INetInterceptRecordCallback> callback = new (std::nothrow) MockINetInterceptRecordCallbackTest();
    if (callback == nullptr) {
        return;
    }
    parcel.WriteRemoteObject(callback->AsObject());
    OnRemoteRequest(static_cast<uint32_t>(INetFirewallService::REGISTER_INTERCEPT_RECORDS_CALLBACK), parcel);
}

void UnregisterInterceptRecordsCallbackTest(const uint8_t *data, size_t size)
{
    MessageParcel parcel;
    if (!IsDataAndWriteVaild(data, size, parcel)) {
        return;
    }
    auto status = std::make_unique<NetFirewallPolicy>();
    if (!status->Marshalling(parcel)) {
        return;
    }
    sptr<INetInterceptRecordCallback> callback = new (std::nothrow) MockINetInterceptRecordCallbackTest();
    if (callback == nullptr) {
        return;
    }
    parcel.WriteRemoteObject(callback->AsObject());
    OnRemoteRequest(static_cast<uint32_t>(INetFirewallService::UNREGISTER_INTERCEPT_RECORDS_CALLBACK), parcel);
}

void CreateRedirectorTest(const uint8_t *data, size_t size)
{
    MessageParcel parcel;
    if (!WriteInterfaceToken(parcel)) {
        return;
    }
    uint32_t groupId = GetData<uint32_t>();
    uint32_t priority = GetData<uint32_t>();
    parcel.WriteUint32(groupId);
    parcel.WriteUint32(priority);
    OnRemoteRequest(static_cast<uint32_t>(INetFirewallService::CREATE_REDIRECTOR), parcel);
}

void DestroyRedirectorTest(const uint8_t *data, size_t size)
{
    MessageParcel parcel;
    if (!WriteInterfaceToken(parcel)) {
        return;
    }
    std::string redirectorId = GetStringFromData(IFACE_LEN);
    if (!parcel.WriteString(redirectorId)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(INetFirewallService::DESTROY_REDIRECTOR), parcel);
}

void AddRedirectRuleTest(const uint8_t *data, size_t size)
{
    MessageParcel parcel;
    if (!WriteInterfaceToken(parcel)) {
        return;
    }
    std::string redirectorId = GetStringFromData(IFACE_LEN);
    if (!parcel.WriteString(redirectorId)) {
        return;
    }
    auto rule = std::make_unique<TrafficFilterRedirectRule>();
    if (!rule->Marshalling(parcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(INetFirewallService::ADD_REDIRECT_RULE), parcel);
}

void ClearRedirectRuleTest(const uint8_t *data, size_t size)
{
    MessageParcel parcel;
    if (!WriteInterfaceToken(parcel)) {
        return;
    }
    std::string redirectorId = GetStringFromData(IFACE_LEN);
    if (!parcel.WriteString(redirectorId)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(INetFirewallService::CLEAR_REDIRECT_RULE), parcel);
}

void GlobalEnableTrafficFilterTest(const uint8_t *data, size_t size)
{
    (void)size;
    MessageParcel parcel;
    if (!WriteInterfaceToken(parcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(INetFirewallService::GLOBAL_ENABLE_TRAFFIC_FILTER), parcel);
}

void GlobalDisableTrafficFilterTest(const uint8_t *data, size_t size)
{
    (void)size;
    MessageParcel parcel;
    if (!WriteInterfaceToken(parcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(INetFirewallService::GLOBAL_DISABLE_TRAFFIC_FILTER), parcel);
}

void GetTrafficFilterGlobalStatusTest(const uint8_t *data, size_t size)
{
    (void)size;
    MessageParcel parcel;
    if (!WriteInterfaceToken(parcel)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(INetFirewallService::GET_TRAFFIC_FILTER_GLOBAL_STATUS), parcel);
}

void QueryProcessTest(const uint8_t *data, size_t size)
{
    MessageParcel parcel;
    if (!WriteInterfaceToken(parcel)) {
        return;
    }
    std::string srcIp = GetStringFromData(IFACE_LEN);
    uint16_t srcPort = GetData<uint16_t>();
    std::string dstIp = GetStringFromData(IFACE_LEN);
    uint16_t dstPort = GetData<uint16_t>();
    uint8_t protocol = GetData<uint8_t>();

    if (!parcel.WriteString(srcIp)) {
        return;
    }
    if (!parcel.WriteUint16(srcPort)) {
        return;
    }
    if (!parcel.WriteString(dstIp)) {
        return;
    }
    if (!parcel.WriteUint16(dstPort)) {
        return;
    }
    if (!parcel.WriteUint8(protocol)) {
        return;
    }
    OnRemoteRequest(static_cast<uint32_t>(INetFirewallService::QUERY_PROCESS), parcel);
}

void CheckRuleStringParam001FuzzTest(const uint8_t *data, size_t size)
{
    (void)data;
    (void)size;
    napi_env env = nullptr;
    napi_value object = nullptr;
    NetFirewallParamCheck::CheckRuleStringParam(env, object);
}

void CheckRuleStringParam002FuzzTest(const uint8_t *data, size_t size)
{
    (void)data;
    (void)size;
    napi_env env = nullptr;
    napi_value object = nullptr;
    NetFirewallParamCheck::CheckRuleStringParam(env, object);
}

void CheckInterfaceName001FuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    uint8_t strLen = GetData<uint8_t>();
    strLen = strLen % (MAX_INTERFACE_NAME_LEN + 1) + 1;
    std::string iface = GetStringFromData(strLen);
    NetFirewallParamCheck::CheckInterfaceName(iface);
}

void CheckInterfaceName002FuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    uint8_t strLen = GetData<uint8_t>();
    strLen = strLen % (MAX_INTERFACE_NAME_LEN + 1) + 1;
    std::string iface = GetStringFromData(strLen);
    NetFirewallParamCheck::CheckInterfaceName(iface);
}

void CheckInterfaceName003FuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    uint8_t strLen = GetData<uint8_t>();
    strLen = strLen % (MAX_INTERFACE_NAME_LEN + 1) + 1;
    std::string iface = GetStringFromData(strLen);
    NetFirewallParamCheck::CheckInterfaceName(iface);
}

void CheckInterfaceName004FuzzTest(const uint8_t *data, size_t size)
{
    (void)data;
    (void)size;
    NetFirewallParamCheck::CheckInterfaceName("");
}

void CheckInterfaceName005FuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    uint8_t strLen = GetData<uint8_t>();
    strLen = strLen % (MAX_INTERFACE_NAME_LEN + 1) + 1;
    std::string iface = GetStringFromData(strLen);
    NetFirewallParamCheck::CheckInterfaceName(iface);
}

void CheckInterfaceName006FuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    uint8_t strLen = GetData<uint8_t>();
    strLen = strLen % (MAX_INTERFACE_NAME_LEN + 1) + 1;
    std::string iface = GetStringFromData(strLen);
    NetFirewallParamCheck::CheckInterfaceName(iface);
}

void CheckInterfaceName007FuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    uint8_t strLen = GetData<uint8_t>();
    strLen = strLen % (MAX_INTERFACE_NAME_LEN + IFACE_NAME_BOUNDARY_EXTRA) + 1;
    std::string iface = GetStringFromData(strLen);
    NetFirewallParamCheck::CheckInterfaceName(iface);
}

void CheckInterfaceName008FuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    uint8_t strLen = GetData<uint8_t>();
    strLen = strLen % (MAX_INTERFACE_NAME_LEN + 1) + 1;
    std::string iface = GetStringFromData(strLen);
    NetFirewallParamCheck::CheckInterfaceName(iface);
}

void CheckInterface001FuzzTest(const uint8_t *data, size_t size)
{
    (void)data;
    (void)size;
    napi_env env = nullptr;
    napi_value object = nullptr;
    NetFirewallParamCheck::CheckInterface(env, object);
}

void CheckInterface002FuzzTest(const uint8_t *data, size_t size)
{
    (void)data;
    (void)size;
    napi_env env = nullptr;
    napi_value object = nullptr;
    NetFirewallParamCheck::CheckInterface(env, object);
}

void CheckInterface003FuzzTest(const uint8_t *data, size_t size)
{
    (void)data;
    (void)size;
    napi_env env = nullptr;
    napi_value object = nullptr;
    NetFirewallParamCheck::CheckInterface(env, object);
}

void CheckInterface004FuzzTest(const uint8_t *data, size_t size)
{
    (void)data;
    (void)size;
    napi_env env = nullptr;
    napi_value object = nullptr;
    NetFirewallParamCheck::CheckInterface(env, object);
}

void ParseInterface001FuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    napi_env env = nullptr;
    napi_value value = nullptr;
    uint8_t strLen = GetData<uint8_t>();
    strLen = strLen % (MAX_INTERFACE_NAME_LEN + 1) + 1;
    std::string interfaceName = GetStringFromData(strLen);
    NetFirewallRuleParse::ParseInterface(env, value, interfaceName);
}

void ParseInterface002FuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    napi_env env = nullptr;
    napi_value value = nullptr;
    uint8_t strLen = GetData<uint8_t>();
    strLen = strLen % (MAX_INTERFACE_NAME_LEN + 1) + 1;
    std::string interfaceName = GetStringFromData(strLen);
    NetFirewallRuleParse::ParseInterface(env, value, interfaceName);
}

void GetParamRuleInfoFormResultSetInterface001FuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    std::string columnName = GetStringFromData(IFACE_LEN);
    int32_t index = GetData<int32_t>();
    NetFirewallRuleInfo table;
    table.interfaceIndex = -1;
    NetFirewallDbHelper::GetInstance().GetParamRuleInfoFormResultSet(columnName, index, table);
}

void GetParamRuleInfoFormResultSetNonInterface001FuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    std::string columnName = GetStringFromData(IFACE_LEN);
    int32_t index = GetData<int32_t>();
    NetFirewallRuleInfo table;
    table.interfaceIndex = -1;
    NetFirewallDbHelper::GetInstance().GetParamRuleInfoFormResultSet(columnName, index, table);
}

void GetResultSetTableInfoRuleInfo001FuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    auto dbCallBack = new (std::nothrow) NetFirewallDataBaseCallBack();
    if (dbCallBack == nullptr) {
        return;
    }
    std::string firewallDatabaseName = FIREWALL_DB_PATH + FIREWALL_DB_NAME;
    int32_t errCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbStoreConfig config(firewallDatabaseName);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    NetFirewallDataBaseCallBack sqliteOpenHelperCallback;
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store_ =
        OHOS::NativeRdb::RdbHelper::GetRdbStore(config, OLD_VERSION, sqliteOpenHelperCallback, errCode);
    if (store_ == nullptr) {
        return;
    }
    int32_t onCreateRet = dbCallBack->OnCreate(*(store_));
    if (onCreateRet != FIREWALL_OK) {
        return;
    }

    auto resultSet = store_->QuerySql(std::string("SELECT * FROM firewallRule"), std::vector<std::string>{});
    if (resultSet == nullptr) {
        return;
    }

    NetFirewallRuleInfo table;
    NetFirewallDbHelper::GetInstance().GetResultSetTableInfo(resultSet, table);
}

void GetResultSetTableInfoRuleInfo002FuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    auto dbCallBack = new (std::nothrow) NetFirewallDataBaseCallBack();
    if (dbCallBack == nullptr) {
        return;
    }
    std::string firewallDatabaseName = FIREWALL_DB_PATH + FIREWALL_DB_NAME;
    int32_t errCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbStoreConfig config(firewallDatabaseName);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    NetFirewallDataBaseCallBack sqliteOpenHelperCallback;
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store_ =
        OHOS::NativeRdb::RdbHelper::GetRdbStore(config, OLD_VERSION, sqliteOpenHelperCallback, errCode);
    if (store_ == nullptr) {
        return;
    }
    int32_t onCreateRet = dbCallBack->OnCreate(*(store_));
    if (onCreateRet != FIREWALL_OK) {
        return;
    }

    auto resultSet = store_->QuerySql(std::string("SELECT * FROM firewallRule"), std::vector<std::string>{});
    if (resultSet == nullptr) {
        return;
    }
    resultSet->Close();

    NetFirewallRuleInfo table;
    NetFirewallDbHelper::GetInstance().GetResultSetTableInfo(resultSet, table);
}

void GetResultSetTableInfoInterceptRecord001FuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    auto dbCallBack = new (std::nothrow) NetFirewallDataBaseCallBack();
    if (dbCallBack == nullptr) {
        return;
    }
    std::string firewallDatabaseName = FIREWALL_DB_PATH + FIREWALL_DB_NAME;
    int32_t errCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbStoreConfig config(firewallDatabaseName);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    NetFirewallDataBaseCallBack sqliteOpenHelperCallback;
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store_ =
        OHOS::NativeRdb::RdbHelper::GetRdbStore(config, OLD_VERSION, sqliteOpenHelperCallback, errCode);
    if (store_ == nullptr) {
        return;
    }
    int32_t onCreateRet = dbCallBack->OnCreate(*(store_));
    if (onCreateRet != FIREWALL_OK) {
        return;
    }

    auto resultSet = store_->QuerySql(std::string("SELECT * FROM interceptRecord"), std::vector<std::string>{});
    if (resultSet == nullptr) {
        return;
    }

    NetInterceptRecordInfo table;
    NetFirewallDbHelper::GetInstance().GetResultSetTableInfo(resultSet, table);
}

void GetResultSetTableInfoInterceptRecord002FuzzTest(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    g_baseFuzzData = data;
    g_baseFuzzSize = size;
    g_baseFuzzPos = 0;

    auto dbCallBack = new (std::nothrow) NetFirewallDataBaseCallBack();
    if (dbCallBack == nullptr) {
        return;
    }
    std::string firewallDatabaseName = FIREWALL_DB_PATH + FIREWALL_DB_NAME;
    int32_t errCode = OHOS::NativeRdb::E_OK;
    OHOS::NativeRdb::RdbStoreConfig config(firewallDatabaseName);
    config.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    NetFirewallDataBaseCallBack sqliteOpenHelperCallback;
    std::shared_ptr<OHOS::NativeRdb::RdbStore> store_ =
        OHOS::NativeRdb::RdbHelper::GetRdbStore(config, OLD_VERSION, sqliteOpenHelperCallback, errCode);
    if (store_ == nullptr) {
        return;
    }
    int32_t onCreateRet = dbCallBack->OnCreate(*(store_));
    if (onCreateRet != FIREWALL_OK) {
        return;
    }

    auto resultSet = store_->QuerySql(std::string("SELECT * FROM interceptRecord"), std::vector<std::string>{});
    if (resultSet == nullptr) {
        return;
    }
    resultSet->Close();

    NetInterceptRecordInfo table;
    NetFirewallDbHelper::GetInstance().GetResultSetTableInfo(resultSet, table);
}
} // namespace NetManagerStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::NetManagerStandard::SetNetFirewallPolicyTest(data, size);
    OHOS::NetManagerStandard::RegisterInterceptRecordsCallbackTest(data, size);
    OHOS::NetManagerStandard::UnregisterInterceptRecordsCallbackTest(data, size);
    OHOS::NetManagerStandard::CreateRedirectorTest(data, size);
    OHOS::NetManagerStandard::DestroyRedirectorTest(data, size);
    OHOS::NetManagerStandard::AddRedirectRuleTest(data, size);
    OHOS::NetManagerStandard::ClearRedirectRuleTest(data, size);
    OHOS::NetManagerStandard::GlobalEnableTrafficFilterTest(data, size);
    OHOS::NetManagerStandard::GlobalDisableTrafficFilterTest(data, size);
    OHOS::NetManagerStandard::GetTrafficFilterGlobalStatusTest(data, size);
    OHOS::NetManagerStandard::QueryProcessTest(data, size);
    OHOS::NetManagerStandard::CheckRuleStringParam001FuzzTest(data, size);
    OHOS::NetManagerStandard::CheckRuleStringParam002FuzzTest(data, size);
    OHOS::NetManagerStandard::CheckInterfaceName001FuzzTest(data, size);
    OHOS::NetManagerStandard::CheckInterfaceName002FuzzTest(data, size);
    OHOS::NetManagerStandard::CheckInterfaceName003FuzzTest(data, size);
    OHOS::NetManagerStandard::CheckInterfaceName004FuzzTest(data, size);
    OHOS::NetManagerStandard::CheckInterfaceName005FuzzTest(data, size);
    OHOS::NetManagerStandard::CheckInterfaceName006FuzzTest(data, size);
    OHOS::NetManagerStandard::CheckInterfaceName007FuzzTest(data, size);
    OHOS::NetManagerStandard::CheckInterfaceName008FuzzTest(data, size);
    OHOS::NetManagerStandard::CheckInterface001FuzzTest(data, size);
    OHOS::NetManagerStandard::CheckInterface002FuzzTest(data, size);
    OHOS::NetManagerStandard::CheckInterface003FuzzTest(data, size);
    OHOS::NetManagerStandard::CheckInterface004FuzzTest(data, size);
    OHOS::NetManagerStandard::ParseInterface001FuzzTest(data, size);
    OHOS::NetManagerStandard::ParseInterface002FuzzTest(data, size);
    OHOS::NetManagerStandard::GetParamRuleInfoFormResultSetInterface001FuzzTest(data, size);
    OHOS::NetManagerStandard::GetParamRuleInfoFormResultSetNonInterface001FuzzTest(data, size);
    OHOS::NetManagerStandard::GetResultSetTableInfoRuleInfo001FuzzTest(data, size);
    OHOS::NetManagerStandard::GetResultSetTableInfoRuleInfo002FuzzTest(data, size);
    OHOS::NetManagerStandard::GetResultSetTableInfoInterceptRecord001FuzzTest(data, size);
    OHOS::NetManagerStandard::GetResultSetTableInfoInterceptRecord002FuzzTest(data, size);
    return 0;
}
