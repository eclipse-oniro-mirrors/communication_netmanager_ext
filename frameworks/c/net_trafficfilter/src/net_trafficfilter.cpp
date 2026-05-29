/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "net_trafficfilter.h"
#include "net_trafficfilter_adapter.h"
#include "netmgr_ext_log_wrapper.h"

using namespace OHOS::NetManagerStandard;

int32_t OH_TrafficFilter_CreateRedirector(uint32_t group_id, uint32_t priority,
    OH_TrafficFilter_Redirector** redirector)
{
    return RedirectorAdapterManager::GetInstance().CreateRedirector(group_id, priority, redirector);
}

int32_t OH_TrafficFilter_DestroyRedirector(OH_TrafficFilter_Redirector* redirector)
{
    if (redirector == nullptr) {
        NETMGR_EXT_LOG_E("DestroyRedirector: redirector is NULL");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    return RedirectorAdapterManager::GetInstance().DestroyRedirector(redirector);
}

int32_t OH_TrafficFilter_AddRedirectRule(OH_TrafficFilter_Redirector* redirector,
    const OH_TrafficFilter_RedirectRule* rule)
{
    if (redirector == nullptr) {
        NETMGR_EXT_LOG_E("AddRedirectRule: redirector is NULL");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    if (rule == nullptr) {
        NETMGR_EXT_LOG_E("AddRedirectRule: rule is NULL");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    if (rule->size < REDIRECT_RULE_MIN_SIZE) {
        NETMGR_EXT_LOG_E("AddRedirectRule: invalid rule size=%{public}u, min=%{public}u",
            rule->size, REDIRECT_RULE_MIN_SIZE);
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    return RedirectorAdapterManager::GetInstance().AddRedirectRule(redirector, rule);
}

int32_t OH_TrafficFilter_ClearRedirectRule(OH_TrafficFilter_Redirector* redirector)
{
    if (redirector == nullptr) {
        NETMGR_EXT_LOG_E("ClearRedirectRule: redirector is NULL");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    return RedirectorAdapterManager::GetInstance().ClearRedirectRule(redirector);
}

int32_t OH_TrafficFilter_QueryProcess(const OH_TrafficFilter_ConnectionInfo* connectionInfo,
    OH_TrafficFilter_ProcessInfo* processInfo)
{
    if (connectionInfo == nullptr || processInfo == nullptr) {
        NETMGR_EXT_LOG_E("QueryProcess: connectionInfo or processInfo is null");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    if (connectionInfo->size < CONNECTION_INFO_MIN_SIZE) {
        NETMGR_EXT_LOG_E("QueryProcess: invalid connection size=%{public}u, min=%{public}u",
            connectionInfo->size, CONNECTION_INFO_MIN_SIZE);
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    if (processInfo->size < PROCESS_INFO_MIN_SIZE) {
        NETMGR_EXT_LOG_E("QueryProcess: invalid processInfo size=%{public}u, min=%{public}u",
            processInfo->size, PROCESS_INFO_MIN_SIZE);
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    OH_TrafficFilter_IPFamily srcFamily = connectionInfo->src_ip.family;
    OH_TrafficFilter_IPFamily dstFamily = connectionInfo->dst_ip.family;
    if (static_cast<int32_t>(srcFamily) == 0) {
        srcFamily = OH_TRAFFICFILTER_IP_FAMILY_V4;
    }
    if (static_cast<int32_t>(dstFamily) == 0) {
        dstFamily = OH_TRAFFICFILTER_IP_FAMILY_V4;
    }

    if ((srcFamily != OH_TRAFFICFILTER_IP_FAMILY_V4 && srcFamily != OH_TRAFFICFILTER_IP_FAMILY_V6) ||
        (dstFamily != OH_TRAFFICFILTER_IP_FAMILY_V4 && dstFamily != OH_TRAFFICFILTER_IP_FAMILY_V6)) {
        NETMGR_EXT_LOG_E("QueryProcess: invalid family, src=%{public}d, dst=%{public}d",
            static_cast<int32_t>(connectionInfo->src_ip.family),
            static_cast<int32_t>(connectionInfo->dst_ip.family));
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    if (connectionInfo->protocol != OH_TRAFFICFILTER_PROTO_TCP &&
        connectionInfo->protocol != OH_TRAFFICFILTER_PROTO_UDP) {
        NETMGR_EXT_LOG_E("QueryProcess: invalid protocol=%{public}u", connectionInfo->protocol);
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    return RedirectorAdapterManager::GetInstance().QueryProcess(connectionInfo, processInfo);
}
