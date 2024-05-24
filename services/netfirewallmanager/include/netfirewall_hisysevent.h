/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef NETFIREWALL_HISYSEVENT_H
#define NETFIREWALL_HISYSEVENT_H

#include <string>

#include "netfirewall_parcel.h"
#include "net_event_report.h"
#include "net_manager_ext_constants.h"

namespace OHOS {
namespace NetManagerStandard {
struct NetFirewallEvent {
    int32_t userId = 100;
    int32_t errorType = 0;
    std::string errorMsg;
};

class NetFirewallHisysEvent {
public:
    ~NetFirewallHisysEvent() = default;

    static NetFirewallHisysEvent &GetInstance();

    /**
     * Send firewall configuration management information
     *
     * @param rule Firewall configuration information
     * @param errorCode
     */
    static void SendFirewallConfigReport(sptr<NetFirewallRule> rule, int32_t &errorCode);

    /**
     * Send and obtain firewall management information
     *
     * @param userId User id
     * @param info Query input information
     * @param errorCode Errorcode such as FIREWALL_SUCCESS = 0,
     *     FIREWALL_ERR_PERMISSION_DENIED = 201 and so on
     */
    static void SendFirewallRequestReport(const int32_t userId, const std::string &info, int32_t &errorCode);

    /**
     * Sending and obtaining interception records and tracking information
     *
     * @param userId User id
     * @param errorCode Errorcode such as FIREWALL_SUCCESS = 0,
     *     FIREWALL_ERR_PERMISSION_DENIED = 201 and so on
     */
    static void SendRecordRequestReport(const int32_t userId, int32_t &errorCode);

    /**
     * Send initialization default rule information
     *
     * @param userId User id
     * @param errorCode Errorcode such as FIREWALL_SUCCESS = 0,
     *     FIREWALL_ERR_PERMISSION_DENIED = 201 and so on
     */
    static void SendInitDefaultRequestReport(const int32_t userId, int32_t &errorCode);

private:
    NetFirewallHisysEvent() = default;

    void SendNetFirewallRuleFault(const NetFirewallEvent &event, const std::string &info, const std::string &eventName);

    void SendNetFirewallRuleBehavior(const int32_t userId, const std::string &info, const std::string &eventName);

    void SendNetFirewallFault(const NetFirewallEvent &event, const std::string &eventName);

    void SendNetFirewallBehavior(const int32_t userId, const std::string &eventName);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETFIREWALL_HISYSEVENT_H