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

#ifndef MOCK_I_NET_CONN_SERVICE_H
#define MOCK_I_NET_CONN_SERVICE_H

#include <gmock/gmock.h>
#include "proxy/i_net_conn_service.h"

namespace OHOS {
namespace NetManagerStandard {

class MockINetConnService : public INetConnService {
public:
    MOCK_METHOD(sptr<IRemoteObject>, AsObject, ());
    MOCK_METHOD(int32_t, SystemReady, (), (override));
    MOCK_METHOD(int32_t, SetInternetPermission, (uint32_t uid, uint8_t allow), (override));
    MOCK_METHOD(int32_t, RegisterNetSupplier, (NetBearType bearerType, const std::string &ident,
                                 const std::set<NetCap> &netCaps, uint32_t &supplierId), (override));
    MOCK_METHOD(int32_t, UnregisterNetSupplier, (uint32_t supplierId), (override));
    MOCK_METHOD(int32_t, RegisterNetSupplierCallback, (uint32_t supplierId, const sptr<INetSupplierCallback> &callback), (override));
    MOCK_METHOD(int32_t, RegisterNetConnCallback, (const sptr<INetConnCallback> callback), (override));
    MOCK_METHOD(int32_t, RegisterNetConnCallback, (const sptr<NetSpecifier> &netSpecifier,
                                                 const sptr<INetConnCallback> callback, const uint32_t &timeoutMS), (override));
    MOCK_METHOD(int32_t, RequestNetConnection, (const sptr<NetSpecifier> netSpecifier,
                                              const sptr<INetConnCallback> callback, const uint32_t timeoutMS), (override));
    MOCK_METHOD(int32_t, UnregisterNetConnCallback, (const sptr<INetConnCallback> &callback), (override));
    MOCK_METHOD(int32_t, UpdateNetCaps, (const std::set<NetCap> &netCaps, const uint32_t supplierId), (override));
    MOCK_METHOD(int32_t, UpdateNetStateForTest, (const sptr<NetSpecifier> &netSpecifier, int32_t netState), (override));
    MOCK_METHOD(int32_t, UpdateNetSupplierInfo, (uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo), (override));
    MOCK_METHOD(int32_t, UpdateNetLinkInfo, (uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo), (override));
    MOCK_METHOD(int32_t, GetIfaceNames, (NetBearType bearerType, std::list<std::string> &ifaceNames), (override));
    MOCK_METHOD(int32_t, GetIfaceNameByType, (NetBearType bearerType, const std::string &ident, std::string &ifaceName), (override));
    MOCK_METHOD(int32_t, GetIfaceNameIdentMaps, (NetBearType bearerType,
                                              (SafeMap<std::string, std::string> &ifaceNameIdentMaps)), (override));
    MOCK_METHOD(int32_t, RegisterNetDetectionCallback, (int32_t netId, const sptr<INetDetectionCallback> &callback), (override));
    MOCK_METHOD(int32_t, UnRegisterNetDetectionCallback, (int32_t netId, const sptr<INetDetectionCallback> &callback), (override));
    MOCK_METHOD(int32_t, NetDetection, (int32_t netId), (override));
    MOCK_METHOD(int32_t, NetDetection, (const std::string &rawUrl, PortalResponse &resp), (override));
    MOCK_METHOD(int32_t, GetDefaultNet, (int32_t &netId), (override));
    MOCK_METHOD(int32_t, HasDefaultNet, (bool &flag), (override));
    MOCK_METHOD(int32_t, GetSpecificNet, (NetBearType bearerType, std::list<int32_t> &netIdList), (override));
    MOCK_METHOD(int32_t, GetSpecificNetByIdent, (NetBearType bearerType, const std::string &ident,
                                               std::list<int32_t> &netIdList), (override));
    MOCK_METHOD(int32_t, GetAllNets, (std::list<int32_t> &netIdList), (override));
    MOCK_METHOD(int32_t, GetSpecificUidNet, (int32_t uid, int32_t &netId), (override));
    MOCK_METHOD(int32_t, GetConnectionProperties, (int32_t netId, NetLinkInfo &info), (override));
    MOCK_METHOD(int32_t, GetNetCapabilities, (int32_t netId, NetAllCapabilities &netAllCap), (override));
    MOCK_METHOD(int32_t, SetAirplaneMode, (bool state), (override));
    MOCK_METHOD(int32_t, IsDefaultNetMetered, (bool &isMetered), (override));
    MOCK_METHOD(int32_t, SetGlobalHttpProxy, (const HttpProxy &httpProxy), (override));
    MOCK_METHOD(int32_t, GetGlobalHttpProxy, (HttpProxy &httpProxy), (override));
    MOCK_METHOD(int32_t, GetDefaultHttpProxy, (int32_t bindNetId, HttpProxy &httpProxy), (override));
    MOCK_METHOD(int32_t, GetNetIdByIdentifier, (const std::string &ident, std::list<int32_t> &netIdList), (override));
    MOCK_METHOD(int32_t, SetAppNet, (int32_t netId), (override));
    MOCK_METHOD(int32_t, RegisterNetInterfaceCallback, (const sptr<INetInterfaceStateCallback> &callback), (override));
    MOCK_METHOD(int32_t, UnregisterNetInterfaceCallback, (const sptr<INetInterfaceStateCallback> &callback), (override));
    MOCK_METHOD(int32_t, GetNetInterfaceConfiguration, (const std::string &iface, NetInterfaceConfiguration &config), (override));
    MOCK_METHOD(int32_t, SetNetInterfaceIpAddress, (const std::string &iface, const std::string &ipAddress), (override));
    MOCK_METHOD(int32_t, SetInterfaceUp, (const std::string &iface), (override));
    MOCK_METHOD(int32_t, SetInterfaceDown, (const std::string &iface), (override));
    MOCK_METHOD(int32_t, AddNetworkRoute, (int32_t netId, const std::string &ifName,
                                         const std::string &destination, const std::string &nextHop), (override));
    MOCK_METHOD(int32_t, RemoveNetworkRoute, (int32_t netId, const std::string &ifName,
                                         const std::string &destination, const std::string &nextHop), (override));
    MOCK_METHOD(int32_t, AddInterfaceAddress, (const std::string &ifName, const std::string &ipAddr,
                                             int32_t prefixLength), (override));
    MOCK_METHOD(int32_t, DelInterfaceAddress, (const std::string &ifName, const std::string &ipAddr,
                                             int32_t prefixLength), (override));
    MOCK_METHOD(int32_t, AddStaticArp, (const std::string &ipAddr, const std::string &macAddr,
                                      const std::string &ifName), (override));
    MOCK_METHOD(int32_t, DelStaticArp, (const std::string &ipAddr, const std::string &macAddr,
                                      const std::string &ifName), (override));
    MOCK_METHOD(int32_t, RegisterSlotType, (uint32_t supplierId, int32_t type), (override));
    MOCK_METHOD(int32_t, GetSlotType, (std::string &type), (override));

    MOCK_METHOD(int32_t, FactoryResetNetwork, (), (override));
    MOCK_METHOD(int32_t, RegisterNetFactoryResetCallback, (const sptr<INetFactoryResetCallback> &callback), (override));
    MOCK_METHOD(int32_t, IsPreferCellularUrl, (const std::string& url, PreferCellularType& preferCellular), (override));
    MOCK_METHOD(int32_t, RegisterPreAirplaneCallback, (const sptr<IPreAirplaneCallback> callback), (override));
    MOCK_METHOD(int32_t, UnregisterPreAirplaneCallback, (const sptr<IPreAirplaneCallback> callback), (override));
    MOCK_METHOD(int32_t, GetDefaultSupplierId, (NetBearType bearerType, const std::string &ident,
                                              uint32_t& supplierId), (override));
    MOCK_METHOD(int32_t, UpdateSupplierScore, (uint32_t supplierId, uint32_t detectionStatus), (override));
    MOCK_METHOD(int32_t, EnableVnicNetwork, (const sptr<NetLinkInfo> &netLinkInfo, const std::set<int32_t> &uids), (override));
    MOCK_METHOD(int32_t, DisableVnicNetwork, (), (override));
    MOCK_METHOD(int32_t, EnableDistributedClientNet, (const std::string &virnicAddr, const std::string &iif), (override));
    MOCK_METHOD(int32_t, EnableDistributedServerNet, (const std::string &iif, const std::string &devIface,
                                                    const std::string &dstAddr, const std::string &gw), (override));
    MOCK_METHOD(int32_t, DisableDistributedNet, (bool isServer), (override));
    MOCK_METHOD(int32_t, CloseSocketsUid, (int32_t netId, uint32_t uid), (override));
    MOCK_METHOD(int32_t, SetPacUrl, (const std::string &pacUrl), (override));
    MOCK_METHOD(int32_t, GetPacUrl, (std::string &pacUrl), (override));
    MOCK_METHOD(int32_t, SetPacFileUrl, (const std::string &pacUrl), (override));
    MOCK_METHOD(int32_t, SetProxyMode, (const OHOS::NetManagerStandard::ProxyModeType mode), (override));
    MOCK_METHOD(int32_t, GetProxyMode, (OHOS::NetManagerStandard::ProxyModeType &mode), (override));
    MOCK_METHOD(int32_t, GetPacFileUrl, (std::string &pacUrl), (override));
    MOCK_METHOD(int32_t, FindProxyForURL, (const std::string &url, const std::string &host, std::string &proxy), (override));
    MOCK_METHOD(int32_t, QueryTraceRoute, (const std::string &destination,
        int32_t maxJumpNumber, int32_t packetsType, std::string &traceRouteInfo, bool isCallerNative), (override));
    MOCK_METHOD(int32_t, SetAppIsFrozened, (uint32_t uid, bool isFrozened), (override));
    MOCK_METHOD(int32_t, EnableAppFrozenedCallbackLimitation, (bool flag), (override));
    MOCK_METHOD(int32_t, SetReuseSupplierId, (uint32_t supplierId, uint32_t reuseSupplierId, bool isReused), (override));
    MOCK_METHOD(int32_t, SetNetExtAttribute, (int32_t netId, const std::string &netExtAttribute), (override));
    MOCK_METHOD(int32_t, GetNetExtAttribute, (int32_t netId, std::string &netExtAttribute), (override));
    MOCK_METHOD(int32_t, AddStaticIpv6Addr, (const std::string &ipv6Addr, const std::string &macAddr, const std::string &ifName), (override));
    MOCK_METHOD(int32_t, DelStaticIpv6Addr, (const std::string &ipv6Addr, const std::string &macAddr, const std::string &ifName), (override));
    MOCK_METHOD(int32_t, GetIpNeighTable, (std::vector<NetIpMacInfo> &ipMacInfo), (override));
    MOCK_METHOD(int32_t, CreateVlan, (const std::string &ifName, uint32_t vlanId), (override));
    MOCK_METHOD(int32_t, DestroyVlan, (const std::string &ifName, uint32_t vlanId), (override));
    MOCK_METHOD(int32_t, AddVlanIp, (const std::string &ifName, uint32_t vlanId, const std::string &ip, uint32_t mask), (override));
    MOCK_METHOD(int32_t, DeleteVlanIp, (const std::string &ifName, uint32_t vlanId, const std::string &ip, uint32_t mask), (override));
    MOCK_METHOD(int32_t, GetConnectOwnerUid, (const NetConnInfo &netConnInfo, int32_t &ownerUid), (override));
    MOCK_METHOD(int32_t, GetSystemNetPortStates, (NetPortStatesInfo &netPortStatesInfo), (override));
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // MOCK_I_NET_CONN_SERVICE_H
