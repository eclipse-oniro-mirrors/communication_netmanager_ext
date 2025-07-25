/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef NETMGR_EXT_INCLUDE_EVENT_REPORT_H
#define NETMGR_EXT_INCLUDE_EVENT_REPORT_H

#include <string>

#include "hisysevent.h"

using HiSysEventType = OHOS::HiviewDFX::HiSysEvent::EventType;
using HiSysEvent = OHOS::HiviewDFX::HiSysEvent;

namespace OHOS {
namespace NetManagerStandard {
struct EventInfo {
    int32_t sharingType = 0;
    int32_t operatorType = 0;
    int32_t sharingCount = 0;

    int32_t errorType = 0;
    std::string errorMsg;
    std::string apOpenErrorMsg;
    std::string apCloseErrorMsg;
};

enum class NetworkShareEventOperator {
    OPERATION_START_SA = 0,
    OPERATION_ENABLE_IFACE,
    OPERATION_TURNON_IP_FORWARD,
    OPERATION_GET_UPSTREAM,
    OPERATION_CONFIG_FORWARD,
    OPERATION_CHECK_SA = 10,
    OPERATION_DISABLE_IFACE,
    OPERATION_TURNOFF_IP_FORWARD,
    OPERATION_CANCEL_FORWARD,
};

enum class NetworkShareEventErrorType {
    ERROR_START_SA = -100,
    ERROR_CHECK_SA = -101,
    ERROR_ENABLE_IFACE = -110,
    ERROR_DISABLE_IFACE = -111,
    ERROR_TURNON_IP_FORWARD = -120,
    ERROR_TURNOFF_IP_FORWARD = -121,
    ERROR_CONFIG_FORWARD = -130,
    ERROR_CANCEL_FORWARD = -131,
    ERROR_GET_UPSTREAM = -140,
};

enum class NetworkShareEventType {
    SETUP_EVENT = 0,
    CANCEL_EVENT = 1,
    BEHAVIOR_EVENT = 2,
};

struct VpnEventInfo {
    int32_t legacy = 0;
    int32_t operatorType = 0;
    int32_t errorType = 0;
    std::string errorMsg;
};

enum class VpnEventType {
    TYPE_UNKNOWN = 0,
    TYPE_LEGACY,
    TYPE_EXTENDED,
};

enum class VpnEventOperator {
    OPERATION_START_SA = 0,
    OPERATION_PROTOCOL_NEGOTIATE,
    OPERATION_CONNECT_SETTING,
    OPERATION_CONNECT_DESTROY,
};

enum class VpnEventErrorType {
    ERROR_UNKNOWN_PROTOCOL_TYPE = -100,
    ERROR_CREATE_INTERFACE_ERROR = -101,
    ERROR_REG_NET_SUPPLIER_ERROR = -110,
    ERROR_UNREG_NET_SUPPLIER_ERROR = -111,
    ERROR_UPDATE_SUPPLIER_INFO_ERROR = -120,
    ERROR_UPDATE_NETLINK_INFO_ERROR = -121,
    ERROR_SET_APP_UID_RULE_ERROR = -130,
    ERROR_INTERNAL_ERROR = -140,
};

struct MultiVpnEvent {
    int32_t userId = 100;
    int32_t operatorType = 0;
    int32_t errorType = 0;
    std::string bundleName;
    std::string errorMsg;
};

enum class VpnOperatorType {
    OPERATION_SETUP_VPN = 0,
    OPERATION_ADD_VPN,
    OPERATION_DELETE_VPN,
    OPERATION_DESTROY_VPN,
};

enum class VpnOperatorErrorType {
    ERROR_CONFIG_WRONG = -100,
    ERROR_INTERNAL_DISCONNECT = -101,
};

class NetEventReport {
public:
    static int32_t SendSetupFaultEvent(const EventInfo &eventInfo);
    static int32_t SendCancleFaultEvent(const EventInfo &eventInfo);
    static int32_t SendTimeBehaviorEvent(const EventInfo &eventInfo);
    static int32_t SendVpnConnectEvent(const VpnEventInfo &eventInfo);
    static int32_t SendWifiSoftapEvent(const EventInfo &eventInfo);
    static int32_t SendVpnFault(const MultiVpnEvent &eventInfo);
    static int32_t SendVpnBehavior(const MultiVpnEvent &eventInfo);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETMGR_EXT_INCLUDE_EVENT_REPORT_H
