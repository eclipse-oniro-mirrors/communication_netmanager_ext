/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef MOCK_I_ETHERNET_SERVICE_H
#define MOCK_I_ETHERNET_SERVICE_H

#include "iethernet_service.h"
#include <gmock/gmock.h>

namespace OHOS {
namespace NetManagerStandard {

class MockIEthernetService : public IEthernetService {
public:
    MOCK_METHOD(sptr<IRemoteObject>, AsObject, ());

    MOCK_METHOD(int32_t, GetMacAddress, (std::vector<MacAddressInfo> & macAddrList), (override));
    MOCK_METHOD(
        int32_t, SetIfaceConfig, (const std::string& iface, const sptr<InterfaceConfiguration>& ic), (override));
    MOCK_METHOD(
        int32_t, GetIfaceConfig, (const std::string& iface, sptr<InterfaceConfiguration>& ifaceConfig), (override));
    MOCK_METHOD(int32_t, IsIfaceActive, (const std::string& iface, int32_t& activeStatus), (override));
    MOCK_METHOD(int32_t, GetAllActiveIfaces, (std::vector<std::string> & activeIfaces), (override));
#ifdef FEATURE_GET_IFACE_SUPPLIER_ID
    MOCK_METHOD(int32_t, GetIfaceSupplierId, (const std::string& iface, uint32_t& supplierId), (override));
#endif // FEATURE_GET_IFACE_SUPPLIER_ID
    MOCK_METHOD(int32_t, ResetFactory, (), (override));
    MOCK_METHOD(int32_t, RegisterIfacesStateChanged, (const sptr<InterfaceStateCallback>& callback), (override));
    MOCK_METHOD(int32_t, UnregisterIfacesStateChanged, (const sptr<InterfaceStateCallback>& callback), (override));
    MOCK_METHOD(int32_t, SetInterfaceUp, (const std::string& iface), (override));
    MOCK_METHOD(int32_t, SetInterfaceDown, (const std::string& iface), (override));
    MOCK_METHOD(int32_t, GetInterfaceConfig, (const std::string& iface, ConfigurationParcelIpc& cfgIpc), (override));
    MOCK_METHOD(
        int32_t, SetInterfaceConfig, (const std::string& iface, const ConfigurationParcelIpc& cfgIpc), (override));
    MOCK_METHOD(int32_t, GetDeviceInformation, (std::vector<EthernetDeviceInfo> & deviceInfoList), (override));
    MOCK_METHOD(int32_t, RegCustomEapHandler,
        (int32_t netType, const std::string& regCmd, const sptr<INetEapPostbackCallback>& callback), (override));
    MOCK_METHOD(int32_t, ReplyCustomEapData, (int32_t result, const sptr<EapData>& eapData), (override));
    MOCK_METHOD(int32_t, RegisterCustomEapCallback, (int32_t netType, const sptr<INetRegisterEapCallback>& callback),
        (override));
    MOCK_METHOD(int32_t, UnRegisterCustomEapCallback, (int32_t netType, const sptr<INetRegisterEapCallback>& callback),
        (override));
    MOCK_METHOD(int32_t, NotifyWpaEapInterceptInfo, (int32_t netType, const sptr<EapData>& eapData), (override));
    MOCK_METHOD(int32_t, StartEthEap, (int32_t netId, const EthEapProfile& profile), (override));
    MOCK_METHOD(int32_t, LogOffEthEap, (int32_t netId), (override));
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
    MOCK_METHOD(int32_t, SetEthEapConfig, (const EthEapConfig& config), (override));
    MOCK_METHOD(int32_t, GetEthEapConfig, (EthEapConfig & config), (override));
    MOCK_METHOD(int32_t, DeleteEthEapConfig, (), (override));
    MOCK_METHOD(int32_t, GetEthEapStateInfo, (EthEapStateInfo & stateInfo), (override));
    MOCK_METHOD(int32_t, RegisterEthEapStateCallback, (const sptr<IEthEapStateCallback>& callback), (override));
    MOCK_METHOD(int32_t, UnregisterEthEapStateCallback, (const sptr<IEthEapStateCallback>& callback), (override));
    MOCK_METHOD(int32_t, ConnectEthEap, (bool connect), (override));
    MOCK_METHOD(int32_t, EnableEthernetInterface, (), (override));
    MOCK_METHOD(int32_t, DisableEthernetInterface, (), (override));
    MOCK_METHOD(int32_t, IsEthernetEnabled, (int32_t & enabled), (override));
#endif // NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // MOCK_I_ETHERNET_SERVICE_H
