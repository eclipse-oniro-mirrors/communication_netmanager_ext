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

#ifndef VPN_CONN_STATE_CALLBACK_H
#define VPN_CONN_STATE_CALLBACK_H

#include "net_manager_ext_constants.h"

namespace OHOS {
namespace NetManagerStandard {

class IVpnConnStateCb {
public:
    virtual void OnVpnConnStateChanged(const VpnConnectState &state) = 0;
    virtual void OnMultiVpnConnStateChanged(const VpnConnectState &state, const std::string &vpnId) {};
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // VPN_CONN_STATE_CALLBACK_H
