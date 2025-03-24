/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
 
#ifndef NETWORKSLICE_DNS_RESULT_CALLBACK_H
#define NETWORKSLICE_DNS_RESULT_CALLBACK_H
 
#include "netsys_native_client.h"
#include "net_conn_client.h"
 
namespace OHOS::NetManagerStandard {
constexpr int32_t MIN_CELLULAR_NETID = 100;
class DnsResultCallback : public NetsysDnsReportCallback {
public:
    DnsResultCallback();
    virtual ~DnsResultCallback();
    int32_t OnDnsResultReport(uint32_t size, const std::list<NetsysNative::NetDnsResultReport>) override;
 
private:
    bool IsValidNetId(int32_t netId, int32_t wifiNetId, int32_t cellularNetId);
    void HandleConnectivityChanged(int32_t &wifiNetId, int32_t &cellularNetId);
    int32_t GetDefaultNetId();
};
} // OHOS
#endif
