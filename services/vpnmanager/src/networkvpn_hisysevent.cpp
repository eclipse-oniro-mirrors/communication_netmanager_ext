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
constexpr const int32_t MAIN_USER_ID = 100;
constexpr const int32_t ONE_HOUR = 3600;
std::chrono::system_clock::time_point VpnHisysEvent::sendEventTime = std::chrono::system_clock::time_point::min();

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

void VpnHisysEvent::SetFaultVpnEvent(const int32_t userId, const std::string &bundleName,
                                     const VpnOperatorType &operatorType, const VpnOperatorErrorType &errorCode,
                                     const int32_t vpnType)
{
    auto currentTime = std::chrono::system_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - sendEventTime);
    if (elapsedTime.count() < ONE_HOUR && sendEventTime != std::chrono::system_clock::time_point::min()) {
        return;
    }
    sendEventTime = currentTime;
    MultiVpnEvent event;
    event.isMainUser = userId == MAIN_USER_ID;
    event.bundleName = bundleName;
    event.vpnType = vpnType;
    event.operatorType = static_cast<int32_t>(operatorType);
    event.errorCode = static_cast<int32_t>(errorCode);
    NetEventReport::SendVpnFault(event);
}
} // namespace NetManagerStandard
} // namespace OHOS