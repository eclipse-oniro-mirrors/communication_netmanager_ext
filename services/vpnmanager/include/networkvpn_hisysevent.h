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

#ifndef NETWORK_VPN_HISYSEVENT_H
#define NETWORK_VPN_HISYSEVENT_H

#include <string>

#include "net_event_report.h"
#include "net_manager_ext_constants.h"

namespace OHOS {
namespace NetManagerStandard {

class VpnHisysEvent {
public:
    // param isLegacy: 0: unknow, 1:legacy vpn, 2:extend vpn
    static void SendFaultEvent(const VpnEventType &isLegacy, const VpnEventOperator &operatorType,
                               const VpnEventErrorType &errorCode, const std::string &errorMsg);

    static void SendFaultEventConnSetting(const VpnEventType &isLegacy, const VpnEventErrorType &errorCode,
                                          const std::string &errorMsg);

    static void SendFaultEventConnDestroy(const VpnEventType &isLegacy, const VpnEventErrorType &errorCode,
                                          const std::string &errorMsg);
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORK_VPN_HISYSEVENT_H
