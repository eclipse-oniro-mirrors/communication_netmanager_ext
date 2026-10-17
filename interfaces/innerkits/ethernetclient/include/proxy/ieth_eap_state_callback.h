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

#ifndef IETH_EAP_STATE_CALLBACK_H
#define IETH_EAP_STATE_CALLBACK_H

#include "iremote_broker.h"
#include "eth_eap_config.h"

namespace OHOS {
namespace NetManagerStandard {

class IEthEapStateCallback : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.EthernetService.IEthEapStateCallback");

    virtual int32_t OnEthEapStateChanged(const EthEapStateInfo &stateInfo) = 0;
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // IETH_EAP_STATE_CALLBACK_H
