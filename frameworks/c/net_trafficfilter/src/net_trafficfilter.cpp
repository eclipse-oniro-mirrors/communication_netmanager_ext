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
    const OH_TrafficFilter_Config* config, OH_TrafficFilter_Redirector** redirector)
{
    if (config == nullptr) {
        NETMGR_EXT_LOG_E("CreateRedirector: config parameter is NULL");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }
    return RedirectorAdapterManager::GetInstance().CreateRedirector(group_id, priority, config, redirector);
}

void OH_TrafficFilter_DestroyRedirector(OH_TrafficFilter_Redirector* redirector)
{
    if (redirector == nullptr) {
        NETMGR_EXT_LOG_E("DestroyRedirector: redirector is NULL");
        return;
    }
    RedirectorAdapterManager::GetInstance().DestroyRedirector(redirector);
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

int32_t OH_TrafficFilter_QueryProcess(const OH_TrafficFilter_ConnectionInfo* connection_info,
    OH_TrafficFilter_ProcessInfo* process_info)
{
    if (connection_info == nullptr || process_info == nullptr) {
        NETMGR_EXT_LOG_E("QueryProcess: invalid parameters, connection_info or process_info is null");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    if (connection_info->src_ip == nullptr || connection_info->dst_ip == nullptr) {
        NETMGR_EXT_LOG_E("QueryProcess: invalid parameters, src_ip or dst_ip is null");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    if (connection_info->src_ip[0] == '\0' || connection_info->dst_ip[0] == '\0') {
        NETMGR_EXT_LOG_E("QueryProcess: invalid parameters, src_ip or dst_ip is empty");
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    if (connection_info->protocol != OH_TRAFFICFILTER_PROTO_TCP &&
        connection_info->protocol != OH_TRAFFICFILTER_PROTO_UDP) {
        NETMGR_EXT_LOG_E("QueryProcess: invalid protocol %{public}u", connection_info->protocol);
        return OH_TRAFFICFILTER_ERROR_INVALID_PARAM;
    }

    return RedirectorAdapterManager::GetInstance().QueryProcess(
        connection_info->src_ip, connection_info->src_port, connection_info->dst_ip,
        connection_info->dst_port, connection_info->protocol, process_info);
}
