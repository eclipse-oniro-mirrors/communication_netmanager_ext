/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef ETHERNET_CLIENT_H
#define ETHERNET_CLIENT_H

#include <string>

#include "iethernet_service.h"
#include "interface_state_callback.h"
#include "net_eap_callback_stub.h"
#include "parcel.h"
#include "singleton.h"

namespace OHOS {
namespace NetManagerStandard {
class EthernetClient {
    DECLARE_DELAYED_SINGLETON(EthernetClient)

public:
    /**
     * Get the ethernet MAC address
     *
     * @param macAddrList MAC address info list
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.GET_ETHERNET_LOCAL_MAC
     * @systemapi Hide this for inner system use.
     */
    int32_t GetMacAddress(std::vector<MacAddressInfo> &macAddrList);

    /**
     *  Set the network interface configuration
     *
     * @param iface interface name
     * @param ic interface configuration
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t SetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ic);

    /**
     *  Gets the network interface configuration parameters
     *
     * @param iface interface name
     * @param ifaceConfig interface configuration
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ifaceConfig);

   /**
     *  check the network interface is active or not
     *
     * @param iface interface name
     * @param activeStatus active status
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t IsIfaceActive(const std::string &iface, int32_t &activeStatus);

    /**
     *  Gets the list of active devices
     *
     * @param activeIfaces list of active interface
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetAllActiveIfaces(std::vector<std::string> &activeIfaces);

    /**
     *  Reset all configuration information
     *
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t ResetFactory();

    /**
     *  Register the callback to monitor interface add/remove state
     *
     * @param callback use to receive interface add/remove event.
     * @return Returns NETMANAGER_EXT_SUCCESS as success, other values as failure
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t RegisterIfacesStateChanged(const sptr<InterfaceStateCallback> &callback);

    /**
     *  Cancel register the callback to monitor interface add/remove state
     *
     * @param callback use to receive interface add/remove event.
     * @return Returns NETMANAGER_EXT_SUCCESS as success, other values as failure
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t UnregisterIfacesStateChanged(const sptr<InterfaceStateCallback> &callback);

    /**
     *  Set the specified network port up
     *
     * @param iface interface name
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t SetInterfaceUp(const std::string &iface);

    /**
     *  Set the specified network port down
     *
     * @param iface interface name
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t SetInterfaceDown(const std::string &iface);

    /**
     *  Get the specified network port configuration
     *
     * @param iface interface name
     * @param cfg interface configuration
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t GetInterfaceConfig(const std::string &iface, OHOS::nmd::InterfaceConfigurationParcel &cfg);

    /**
     *  Set the specified network port configuration
     *
     * @param iface interface name
     * @param cfg interface configuration
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.CONNECTIVITY_INTERNAL
     * @systemapi Hide this for inner system use.
     */
    int32_t SetInterfaceConfig(const std::string &iface, OHOS::nmd::InterfaceConfigurationParcel &cfg);

    /**
     * Register custom eap info and callback
     * @param netType Indicates net type need to customize
     * @param regCmd Indicates some eapCode and eapType info. eg: 2:277:278
     * @param callback the func of post back eap data
     * @return Returns 0, successfully register custom eap callback, otherwise it will failed
     * @permission ohos.permission.ENTERPRISE_MANAGE_EAP
     * @systemapi Hide this for inner system use.
     */
    int32_t RegCustomEapHandler(NetType netType, const std::string &regCmd,
        const sptr<INetEapPostbackCallback> &callback);
 
    /**
     * Reply custom eap data of app
     *
     * @param result The result of app
     * @param eapData sptr of EapData
     * @return Returns 0, successfully replyl custom eap data, otherwise it will failed
     * @permission ohos.permission.ENTERPRISE_MANAGE_EAP
     * @systemapi Hide this for inner system use.
     */
    int32_t ReplyCustomEapData(int result, const sptr<EapData> &eapData);
 
    /**
     * Register module callback, such as wifi or eth
     *
     * @param netType Indicates net interface type need to customize
     * @param callback The callback of INetRegisterEapCallback interface
     * @return Returns 0, successfully register custom eap callback, otherwise it will failed
     * @systemapi Hide this for inner system use.
     */
    int32_t RegisterCustomEapCallback(const NetType netType, const sptr<INetRegisterEapCallback> &callback);
 
    /**
     * unRegister custom eap callback
     *
     * @param netType Indicates net interface type need to customize
     * @param callback The callback of INetRegisterEapCallback interface
     * @return Returns 0, successfully unregister custom eap callback, otherwise it will failed
     * @systemapi Hide this for inner system use.
     */
    int32_t UnRegisterCustomEapCallback(const NetType netType, const sptr<INetRegisterEapCallback> &callback);
 
    /**
     * Notify Wap Eap Intercept Info
     *
     * @param netType Indicates net interface type need to customize
     * @param eapData the sptr of eapData
     * @return Returns 0, successfully notity success, otherwise it will failed
     * @systemapi Hide this for inner system use.
     */
    int32_t NotifyWpaEapInterceptInfo(const NetType netType, const sptr<EapData> &eapData);
 
    /**
     * Get the ethernet device infomation
     *
     * @param deviceInfoList device information list
     * @return Return NETMANAGER_EXT_SUCCESS if process normal, others is error
     * @permission ohos.permission.GET_NETWORK_INFO
     * @systemapi Hide this for inner system use.
     */
    int32_t GetDeviceInformation(std::vector<EthernetDeviceInfo> &deviceInfoList);

private:
    class EthernetDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit EthernetDeathRecipient(EthernetClient &client) : client_(client) {}
        ~EthernetDeathRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override
        {
            client_.OnRemoteDied(remote);
        }

    private:
        EthernetClient &client_;
    };

private:
    sptr<IEthernetService> GetProxy();
    void RecoverCallback();
    void OnRemoteDied(const wptr<IRemoteObject> &remote);

private:
    std::mutex mutex_;
    sptr<IEthernetService> ethernetService_;
    sptr<IRemoteObject::DeathRecipient> deathRecipient_;
    sptr<InterfaceStateCallback> callback_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETHERNET_CLIENT_H