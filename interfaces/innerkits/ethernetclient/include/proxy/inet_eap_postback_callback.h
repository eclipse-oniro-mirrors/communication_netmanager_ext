/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
 
#ifndef I_NET_EAP_POSTBACK_CALLBACK_H
#define I_NET_EAP_POSTBACK_CALLBACK_H
 
#include "iremote_broker.h"
#include "eap_data.h"
#include "net_manager_constants.h"
#include "refbase.h"
 
namespace OHOS {
namespace NetManagerStandard {
class INetEapPostbackCallback : public IRemoteBroker {
public:
    virtual ~INetEapPostbackCallback() = default;
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.EthernetService.INetEapPostbackCallback");
 
public:
    virtual int32_t OnEapSupplicantPostback(NetType netType, const sptr<EapData> &eapData) = 0;
};
 
} // namespace NetManagerStandard
} // namespace OHOS
#endif // I_NET_EAP_POSTBACK_CALLBACK_H