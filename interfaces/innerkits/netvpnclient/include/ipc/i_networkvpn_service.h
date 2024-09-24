/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef I_NETWORK_VPN_SERVICE_H
#define I_NETWORK_VPN_SERVICE_H

#include <string>
#include <vector>

#include "iremote_broker.h"
#include "iremote_object.h"

#include "i_vpn_event_callback.h"
#include "net_manager_ext_constants.h"
#ifdef SUPPORT_SYSVPN
#include "sysvpn_config.h"
#endif // SUPPORT_SYSVPN
#include "vpn_config.h"

namespace OHOS {
namespace NetManagerStandard {
class INetworkVpnService : public IRemoteBroker {
public:
    enum class MessageCode {
        CMD_START_INTERNAL_VPN, // for start internal vpn
        CMD_PREPARE,
        CMD_START_VPN, // for start extended vpn
        CMD_PROTECT,
        CMD_STOP_VPN,
        CMD_REGISTER_EVENT_CALLBACK,
        CMD_UNREGISTER_EVENT_CALLBACK,
        CMD_CREATE_VPN_CONNECTION,
        CMD_FACTORYRESET_VPN,
#ifdef SUPPORT_SYSVPN
        CMD_ADD_SYS_VPN_CONFIG,
        CMD_DELETE_SYS_VPN_CONFIG,
        CMD_GET_SYS_VPN_CONFIG_LIST,
        CMD_GET_SYS_VPN_CONFIG,
        CMD_GET_CONNECTED_SYS_VPN_CONFIG,
        CMD_NOTIFY_CONNECT_STAGE,
        CMD_SETUP_SYS_VPN,
        CMD_GET_SYS_VPN_CERT_URI,
#endif // SUPPORT_SYSVPN
        CMD_START_VPN_EXT, // for start extension extended vpn
        CMD_PROTECT_EXT,
        CMD_STOP_VPN_EXT,
        CMD_CREATE_VPN_CONNECTION_EXT,
        CMD_REGISTER_BUNDLENAME,
        CMD_GET_SELF_APP_NAME,
    };

public:
    virtual int32_t Prepare(bool &isExistVpn, bool &isRun, std::string &pkg) = 0;
    virtual int32_t SetUpVpn(const sptr<VpnConfig> &config, bool isVpnExtCall = false) = 0;
    virtual int32_t Protect(bool isVpnExtCall = false) = 0;
    virtual int32_t DestroyVpn(bool isVpnExtCall = false) = 0;
    virtual int32_t RegisterVpnEvent(const sptr<IVpnEventCallback> callback) = 0;
    virtual int32_t UnregisterVpnEvent(const sptr<IVpnEventCallback> callback) = 0;
    virtual int32_t CreateVpnConnection(bool isVpnExtCall = false) = 0;
    virtual int32_t FactoryResetVpn() = 0;
    virtual int32_t RegisterBundleName(const std::string &bundleName) = 0;
    virtual int32_t GetSelfAppName(std::string &selfAppName) = 0;
#ifdef SUPPORT_SYSVPN
    virtual int32_t SetUpVpn(const sptr<SysVpnConfig> &config) = 0;
    virtual int32_t AddSysVpnConfig(sptr<SysVpnConfig> &config) = 0;
    virtual int32_t DeleteSysVpnConfig(const std::string &vpnId) = 0;
    virtual int32_t GetSysVpnConfigList(std::vector<SysVpnConfig> &vpnList) = 0;
    virtual int32_t GetSysVpnConfig(sptr<SysVpnConfig> &config, const std::string &vpnId) = 0;
    virtual int32_t GetConnectedSysVpnConfig(sptr<SysVpnConfig> &config) = 0;
    virtual int32_t NotifyConnectStage(const std::string &stage, const int32_t &result) = 0;
    virtual int32_t GetSysVpnCertUri(const int32_t certType, std::string &certUri) = 0;
#endif // SUPPORT_SYSVPN

    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.INetworkVpnService");
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // I_NETWORK_VPN_SERVICE_H
