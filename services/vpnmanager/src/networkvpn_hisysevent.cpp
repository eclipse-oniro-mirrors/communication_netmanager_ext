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

#include "networkvpn_hisysevent.h"
#include "net_event_report.h"

namespace OHOS {
namespace NetManagerStandard {

void VpnHisysEvent::SendFaultEvent(const VpnEventType &isLegacy, const VpnEventOperator &operatorType,
                                   const VpnEventErrorType &errorCode, const std::string &errorMsg)
{
    VpnEventInfo eventInfo;
    eventInfo.legacy = static_cast<int32_t>(isLegacy);
    eventInfo.operatorType = static_cast<int32_t>(operatorType);
    eventInfo.errorType = static_cast<int32_t>(errorCode);
    eventInfo.errorMsg = errorMsg;
    NetEventReport::SendVpnConnectEvent(eventInfo);
}

void VpnHisysEvent::SendFaultEventConnSetting(const VpnEventType &isLegacy, const VpnEventErrorType &errorCode,
                                              const std::string &errorMsg)
{
    VpnEventInfo eventInfo;
    eventInfo.legacy = static_cast<int32_t>(isLegacy);
    eventInfo.operatorType = static_cast<int32_t>(VpnEventOperator::OPERATION_CONNECT_SETTING);
    eventInfo.errorType = static_cast<int32_t>(errorCode);
    eventInfo.errorMsg = errorMsg;
    NetEventReport::SendVpnConnectEvent(eventInfo);
}

void VpnHisysEvent::SendFaultEventConnDestroy(const VpnEventType &isLegacy, const VpnEventErrorType &errorCode,
                                              const std::string &errorMsg)
{
    VpnEventInfo eventInfo;
    eventInfo.legacy = static_cast<int32_t>(isLegacy);
    eventInfo.operatorType = static_cast<int32_t>(VpnEventOperator::OPERATION_CONNECT_DESTROY);
    eventInfo.errorType = static_cast<int32_t>(errorCode);
    eventInfo.errorMsg = errorMsg;
    NetEventReport::SendVpnConnectEvent(eventInfo);
}

} // namespace NetManagerStandard
} // namespace OHOS