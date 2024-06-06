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

#include "errorcode_convertor.h"
#include "net_event_report.h"
#include "net_manager_constants.h"
#include "netfirewall_hisysevent.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *NET_FIREWALL_CONF_FAULT = "NET_FIREWALL_CONF_FAULT";
constexpr const char *NET_FIREWALL_REQ_FAULT = "NET_FIREWALL_REQ_FAULT";
constexpr const char *NET_FIREWALL_LOG_REQ_FAULT = "NET_FIREWALL_LOG_REQ_FAULT";
constexpr const char *NET_FIREWALL_INIT_FAULT = "NET_FIREWALL_INIT_FAULT";
constexpr const char *NET_FIREWALL_CONF_BEHAVIOR = "NET_FIREWALL_CONF_BEHAVIOR";
constexpr const char *NET_FIREWALL_REQ_BEHAVIOR = "NET_FIREWALL_REQ_BEHAVIOR";
constexpr const char *NET_FIREWALL_LOG_REQ_BEHAVIOR = "NET_FIREWALL_LOG_REQ_BEHAVIOR";

constexpr const char *EVENT_KEY_FIREWALL_USER_ID = "userId";
constexpr const char *EVENT_KEY_FIREWALL_RULE_INFO = "ruleInfo";
constexpr const char *EVENT_KEY_FIREWALL_ERROR_TYPE = "errorType";
constexpr const char *EVENT_KEY_FIREWALL_ERROR_MSG = "errorMsg";
}

NetFirewallHisysEvent &NetFirewallHisysEvent::GetInstance()
{
    static NetFirewallHisysEvent instance;
    return instance;
}

void NetFirewallHisysEvent::SendFirewallConfigReport(sptr<NetFirewallRule> rule, int32_t &errorCode)
{
    NetFirewallHisysEvent instance = GetInstance();
    if (errorCode == FIREWALL_SUCCESS) {
        instance.SendNetFirewallRuleBehavior(rule->userId, rule->ToString(), NET_FIREWALL_CONF_BEHAVIOR);
    } else {
        NetFirewallEvent eventInfo;
        eventInfo.userId = rule->userId;
        eventInfo.errorType = errorCode;
        NetBaseErrorCodeConvertor convertor;
        eventInfo.errorMsg = convertor.ConvertErrorCode(errorCode);
        instance.SendNetFirewallRuleFault(eventInfo, rule->ToString(), NET_FIREWALL_CONF_FAULT);
    }
}

void NetFirewallHisysEvent::SendFirewallRequestReport(const int32_t userId, const std::string &info, int32_t &errorCode)
{
    NetFirewallHisysEvent instance = GetInstance();
    if (errorCode == FIREWALL_SUCCESS) {
        instance.SendNetFirewallRuleBehavior(userId, info, NET_FIREWALL_REQ_BEHAVIOR);
    } else {
        NetFirewallEvent eventInfo;
        eventInfo.userId = userId;
        eventInfo.errorType = errorCode;
        NetBaseErrorCodeConvertor convertor;
        eventInfo.errorMsg = convertor.ConvertErrorCode(errorCode);
        instance.SendNetFirewallRuleFault(eventInfo, info, NET_FIREWALL_REQ_FAULT);
    }
}

void NetFirewallHisysEvent::SendRecordRequestReport(const int32_t userId, int32_t &errorCode)
{
    NetFirewallHisysEvent instance = GetInstance();
    if (errorCode == FIREWALL_SUCCESS) {
        instance.SendNetFirewallBehavior(userId, NET_FIREWALL_LOG_REQ_BEHAVIOR);
    } else {
        NetFirewallEvent eventInfo;
        eventInfo.userId = userId;
        eventInfo.errorType = errorCode;
        NetBaseErrorCodeConvertor convertor;
        eventInfo.errorMsg = convertor.ConvertErrorCode(errorCode);
        instance.SendNetFirewallFault(eventInfo, NET_FIREWALL_LOG_REQ_FAULT);
    }
}

void NetFirewallHisysEvent::SendInitDefaultRequestReport(const int32_t userId, int32_t &errorCode)
{
    NetFirewallEvent eventInfo;
    eventInfo.userId = userId;
    eventInfo.errorType = errorCode;
    NetBaseErrorCodeConvertor convertor;
    eventInfo.errorMsg = convertor.ConvertErrorCode(errorCode);
    GetInstance().SendNetFirewallFault(eventInfo, NET_FIREWALL_INIT_FAULT);
}

void NetFirewallHisysEvent::SendNetFirewallRuleFault(const NetFirewallEvent &event, const std::string &info,
    const std::string &eventName)
{
    HiSysEventWrite(HiSysEvent::Domain::NETMANAGER_STANDARD, eventName, HiSysEvent::EventType::FAULT,
        EVENT_KEY_FIREWALL_USER_ID, event.userId, EVENT_KEY_FIREWALL_RULE_INFO, info, EVENT_KEY_FIREWALL_ERROR_TYPE,
        event.errorType, EVENT_KEY_FIREWALL_ERROR_MSG, event.errorMsg);
}

void NetFirewallHisysEvent::SendNetFirewallRuleBehavior(const int32_t userId, const std::string &info,
    const std::string &eventName)
{
    HiSysEventWrite(HiSysEvent::Domain::NETMANAGER_STANDARD, eventName, HiSysEvent::EventType::BEHAVIOR,
        EVENT_KEY_FIREWALL_USER_ID, userId, EVENT_KEY_FIREWALL_RULE_INFO, info);
}

void NetFirewallHisysEvent::SendNetFirewallFault(const NetFirewallEvent &event, const std::string &eventName)
{
    HiSysEventWrite(HiSysEvent::Domain::NETMANAGER_STANDARD, eventName, HiSysEvent::EventType::FAULT,
        EVENT_KEY_FIREWALL_USER_ID, event.userId, EVENT_KEY_FIREWALL_ERROR_TYPE, event.errorType,
        EVENT_KEY_FIREWALL_ERROR_MSG, event.errorMsg);
}

void NetFirewallHisysEvent::SendNetFirewallBehavior(const int32_t userId, const std::string &eventName)
{
    HiSysEventWrite(HiSysEvent::Domain::NETMANAGER_STANDARD, eventName, HiSysEvent::EventType::BEHAVIOR,
        EVENT_KEY_FIREWALL_USER_ID, userId);
}
} // namespace NetManagerStandard
} // namespace OHOS