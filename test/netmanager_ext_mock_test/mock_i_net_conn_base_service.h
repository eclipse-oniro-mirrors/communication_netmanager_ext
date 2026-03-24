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

#ifndef MOCK_I_NET_CONN_BASE_SERVICE_H
#define MOCK_I_NET_CONN_BASE_SERVICE_H

#include <gmock/gmock.h>
#include "net_conn_base_service.h"

namespace OHOS {
namespace NetManagerStandard {

class MockNetConnBaseService : public NetConnBaseService {
public:
    MOCK_METHOD(int32_t, GetIfaceNames, (NetBearType bearerType, std::list<std::string> &ifaceNames), (override));
    MOCK_METHOD(int32_t, GetIfaceNameByType, (NetBearType bearerType, const std::string &ident, std::string &ifaceName), (override));
    MOCK_METHOD(int32_t, RegisterNetSupplier, (NetBearType bearerType, const std::string &ident,
        const std::set<NetCap> &netCaps, uint32_t &supplierId), (override));
    MOCK_METHOD(int32_t, UnregisterNetSupplier, (uint32_t supplierId), (override));
    MOCK_METHOD(int32_t, UpdateNetLinkInfo, (uint32_t supplierId, const sptr<NetLinkInfo> &netLinkInfo), (override));
    MOCK_METHOD(int32_t, UpdateNetSupplierInfo, (uint32_t supplierId, const sptr<NetSupplierInfo> &netSupplierInfo), (override));
    MOCK_METHOD(int32_t, RestrictBackgroundChanged, (bool isRestrictBackground), (override));
    MOCK_METHOD(int32_t, RegisterNetConnCallback, (const sptr<INetConnCallback> &callback), (override));
    MOCK_METHOD(int32_t, RegisterNetFactoryResetCallback, (const sptr<INetFactoryResetCallback> &callback), (override));
    MOCK_METHOD(int32_t, SetReuseSupplierId, (uint32_t supplierId, uint32_t reuseSupplierId, bool isReused), (override));
    MOCK_METHOD(int32_t, UpdateUidLostDelay, (const std::set<uint32_t> &uidLostDelaySet), (override));
    MOCK_METHOD(int32_t, GetConnectionProperties, (int32_t netId, NetLinkInfo &info), (override));
    MOCK_METHOD(int32_t, RegisterDualStackProbeCallback, (int32_t netId,
        std::shared_ptr<IDualStackProbeCallback>& callback), (override));
    MOCK_METHOD(int32_t, UnRegisterDualStackProbeCallback, (int32_t netId,
        std::shared_ptr<IDualStackProbeCallback>& callback), (override));
    MOCK_METHOD(int32_t, DualStackProbe, (int32_t netId), (override));
    MOCK_METHOD(int32_t, UpdateDualStackProbeTime, (int32_t dualStackProbeTimeOut), (override));
    MOCK_METHOD(ProbeUrls, GetDataShareUrl, (), (override));
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // MOCK_I_NET_CONN_BASE_SERVICE_H
