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

#ifndef NETWORKVPN_CLIENT_H
#define NETWORKVPN_CLIENT_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#include <parcel.h>
#include <refbase.h>
#include <unistd.h>

#include "i_networkvpn_service.h"
#include "i_vpn_event_callback.h"
#include "vpn_event_callback_stub.h"
#include "vpn_interface.h"

namespace OHOS {
namespace NetManagerStandard {

class VpnSetUpEventCallback : public VpnEventCallbackStub {
public:
    void OnVpnStateChanged(const bool &isConnected) override{};
    void OnVpnMultiUserSetUp() override;
};

class NetworkVpnClient {
private:
    NetworkVpnClient() = default;
    ~NetworkVpnClient() = default;
    NetworkVpnClient(const NetworkVpnClient &) = delete;
    NetworkVpnClient &operator=(const NetworkVpnClient &) = delete;

public:
    static NetworkVpnClient &GetInstance();

public:
    /**
     * start internal vpn
     *
     * @param isExistVpn check whether exist vpn connection
     * @param isRun if isExistVpn=true, check the vpn is running or not
     * @param pkg Indicates which application the current vpn belongs to
     * @return NETMANAGER_EXT_SUCCESS(0) if process normal, others is error
     * @permission ohos.permission.MANAGE_VPN
     * @systemapi Hide this for inner system use.
     */
    int32_t Prepare(bool &isExistVpn, bool &isRun, std::string &pkg);

    /**
     * extended vpn need always communication with remote vpn server, the data is send/receive by default network but
     * not vpn network.
     *
     * @param socketFd extended vpn opened soecket fd
     * @return NETMANAGER_EXT_SUCCESS(0) if process normal, others is error
     * @permission ohos.permission.MANAGE_VPN
     * @systemapi Hide this for inner system use.
     */
    int32_t Protect(int32_t socketFd, bool isVpnExtCall = false);

    /**
     * after extended vpn's negotiation over, need system create a VPN interface using the config parameters.
     *
     * @param config VPN interface parameters
     * @param tunFd the virtual interface fd(out param)
     * @return the interface node's file descriptor(>0) if process normal, others is error
     * @permission ohos.permission.MANAGE_VPN
     * @systemapi Hide this for inner system use.
     */
    int32_t SetUpVpn(sptr<VpnConfig> config, int32_t &tunFd, bool isVpnExtCall = false);

    /**
     * stop the vpn connection, system will destroy the vpn network.
     *
     * @return NETMANAGER_EXT_SUCCESS(0) if process normal, others is error
     * @permission ohos.permission.MANAGE_VPN
     * @systemapi Hide this for inner system use.
     */
    int32_t DestroyVpn(bool isVpnExtCall = false);

#ifdef SUPPORT_SYSVPN
    /**
     * setup system vpn.
     *
     * @param config system VPN interface parameters
     * @return NETMANAGER_EXT_SUCCESS(0) if process normal, others is error
     * @permission ohos.permission.MANAGE_VPN
     * @systemapi Hide this for inner system use.
     */
    int32_t SetUpVpn(const sptr<SysVpnConfig> &config);

    /**
     * save vpn
     *
     * @param config vpn config
     * @return NETMANAGER_EXT_SUCCESS(0) if process normal, others is error
     * @permission ohos.permission.MANAGE_VPN
     * @systemapi Hide this for inner system use.
     */
    int32_t AddSysVpnConfig(sptr<SysVpnConfig> &config);

    /**
     * delete vpn
     *
     * @param vpnId vpn vpnId
     * @return NETMANAGER_EXT_SUCCESS(0) if process normal, others is error
     * @permission ohos.permission.MANAGE_VPN
     * @systemapi Hide this for inner system use.
     */
    int32_t DeleteSysVpnConfig(const std::string &vpnId);

    /**
     * get vpn list
     *
     * @param vpnList vpn list (out param)
     * @return NETMANAGER_EXT_SUCCESS(0) if process normal, others is error
     * @permission ohos.permission.MANAGE_VPN
     * @systemapi Hide this for inner system use.
     */
    int32_t GetSysVpnConfigList(std::vector<SysVpnConfig> &vpnList);

    /**
     * get vpn detail
     *
     * @param config vpn config (out param)
     * @param vpnId vpn vpnId
     * @return NETMANAGER_EXT_SUCCESS(0) if process normal, others is error
     * @permission ohos.permission.MANAGE_VPN
     * @systemapi Hide this for inner system use.
     */
    int32_t GetSysVpnConfig(sptr<SysVpnConfig> &config, const std::string &vpnId);

    /**
     * get connected vpn
     *
     * @param config VpnConfig
     * @return VpnConnectState
     * @permission ohos.permission.MANAGE_VPN
     * @systemapi Hide this for inner system use.
     */
    int32_t GetConnectedSysVpnConfig(sptr<SysVpnConfig> &config);

    /**
     * nofytify the connect stage to fwk
     *
     * @param stage the connect stage
     * @param result the connect result
     * @return NETMANAGER_EXT_SUCCESS(0) if process normal, others is error
     * @systemapi Hide this for inner system use.
     */
    int32_t NotifyConnectStage(const std::string &stage, const int32_t &result);

    /**
     * get system vpn certificate uri
     *
     * @param certType the certificate type (ca certificate, user certificate or server certificate)
     * @param certUri the certificate uri (out param)
     * @return NETMANAGER_EXT_SUCCESS(0) if process normal, others is error
     * @systemapi Hide this for inner system use.
     */
    int32_t GetSysVpnCertUri(const int32_t certType, std::string &certUri);
#endif // SUPPORT_SYSVPN

    /**
     * register the vpn state callback
     *
     * @param callback if this fuction return NETMANAGER_EXT_SUCCESS(0), this callback will be called by service
     * @return NETMANAGER_EXT_SUCCESS(0) if process normal, others is error
     * @permission ohos.permission.MANAGE_VPN
     * @systemapi Hide this for inner system use.
     */
    int32_t RegisterVpnEvent(sptr<IVpnEventCallback> callback);

    /**
     * unregister the vpn state callback
     *
     * @param callback if this fuction return NETMANAGER_EXT_SUCCESS(0), this callback will not be called by service
     * @return NETMANAGER_EXT_SUCCESS(0) if process normal, others is error
     * @permission ohos.permission.MANAGE_VPN
     * @systemapi Hide this for inner system use.
     */
    int32_t UnregisterVpnEvent(sptr<IVpnEventCallback> callback);

    /**
     * create vpn connection.
     *
     * @return NETMANAGER_EXT_SUCCESS(0) if process normal, others is error
     * @permission ohos.permission.MANAGE_VPN
     * @systemapi Hide this for inner system use.
     */
    int32_t CreateVpnConnection(bool isVpnExtCall = false);

    /**
     * close the tunfd of vpn interface and unregister VpnEvent.
     */
    void multiUserSetUpEvent();
    int32_t RegisterBundleName(const std::string &bundleName);

    int32_t GetSelfAppName(std::string &selfAppName);

    int32_t SetSelfVpnPid();

private:
    class MonitorVpnServiceDead : public IRemoteObject::DeathRecipient {
    public:
        explicit MonitorVpnServiceDead(NetworkVpnClient &client) : client_(client) {}
        ~MonitorVpnServiceDead() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        NetworkVpnClient &client_;
    };

    sptr<INetworkVpnService> GetProxy();
    void RecoverCallback();
    void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    std::mutex mutex_;
    VpnInterface vpnInterface_;
    sptr<IVpnEventCallback> vpnEventCallback_ = nullptr;
    sptr<INetworkVpnService> networkVpnService_ = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_ = nullptr;
    std::pair<sptr<VpnConfig>, bool> clientVpnConfig_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORKVPN_CLIENT_H
