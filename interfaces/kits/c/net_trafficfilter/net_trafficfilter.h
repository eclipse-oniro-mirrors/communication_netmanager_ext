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

/**
 * @addtogroup TrafficFilter
 * @{
 *
 * @brief Defines the APIs for traffic filtering.
 *
 * @since 26.0.0
 */

/**
 * @file net_trafficfilter.h
 * @brief Defines the APIs for traffic filtering.
 *
 * @library libnet_trafficfilter.so
 * @kit NetworkKit
 * @permission ohos.permission.kernel.TRAFFIC_FILTER
 * @syscap SystemCapability.Communication.NetManager.NetFirewall
 * @since 26.0.0
 */

#ifndef NET_TRAFFICFILTER_H
#define NET_TRAFFICFILTER_H

#include "net_trafficfilter_type.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Creates a packet controller instance.
 *     Creates a packet controller for intercepting and filtering network packets
 *     Resource Management: This instance occupies system resources.
 *     You must call {@link OH_TrafficFilter_DestroyPacketController} to release resources.
 *
 * @param group_id Filter chain identifier.
 *                  This is the logical grouping ID within the application.
 *                  Multiple controllers within the same application can use different group_id.
 *                  The same group_id from different applications will be automatically isolated.
 * @param priority Priority (determines execution order between different group_id chain, smaller number executes first)
 * @param config Configuration parameters (can be NULL to use default configuration)
 * @param controller Output parameter, returns the packet controller handle on success.
 * @return Returns {@link OH_TRAFFICFILTER_OK} on success,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_PERMISSION_DENIED} if permission is denied,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_GROUP_ID_IN_USE} when group_id already exists,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_INVALID_PARAM} if priority is invalid,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_NFQUEUE_ERROR} if NFQueue initialization fails.
 *
 * @permission ohos.permission.kernel.TRAFFIC_FILTER
 * @since 26.0.0
 */
int32_t OH_TrafficFilter_CreatePacketController(
    uint32_t group_id,
    uint32_t priority,
    const OH_TrafficFilter_Config* config,
    OH_TrafficFilter_PacketController** controller
);

/**
 * @brief Destroys a packet controller instance.
 *     Destroys the controller and releases related resources, including rules and callbacks.
 *     After calling this function, the handle is invalid. Do not use it again.
 *
 * @param controller OH_TrafficFilter_PacketController handle
 *
 * @permission ohos.permission.kernel.TRAFFIC_FILTER
 * @since 26.0.0
 */
void OH_TrafficFilter_DestroyPacketController(OH_TrafficFilter_PacketController* controller);

/**
 * @brief Register a packet callback function.
 *     Register a callback function to handle intercepted packets.
 *     The callback will be triggered when packets match the filter rule.
 *
 * @note To release the callback, you need to call {@link OH_TrafficFilter_UnregisterPacketCallback}.
 * @param controller OH_TrafficFilter_PacketController handle
 * @param callback Callback function pointer. Cannot be NULL.
 * @param user_data User data (will be passed back in callback).
 * @return Returns {@link OH_TRAFFICFILTER_OK} on success,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_PERMISSION_DENIED} if permission is denied,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_INVALID_PARAM} if controller or callback is NULL.
 *
 * @permission ohos.permission.kernel.TRAFFIC_FILTER
 * @since 26.0.0
 */
int32_t OH_TrafficFilter_RegisterPacketCallback(
    OH_TrafficFilter_PacketController* controller,
    OH_TrafficFilter_PacketCallback callback,
    void* user_data
);

/**
 * @brief Unregister a packet callback function.
 *     Unregister the current packet callback function.
 *     After calling this, no more packets will be delivered to the callback.
 *
 * @param controller OH_TrafficFilter_PacketController handle
 * @return Returns {@link OH_TRAFFICFILTER_OK} on success,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_PERMISSION_DENIED} if permission is denied,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_INVALID_PARAM} if controller is NULL.
 *
 * @permission ohos.permission.kernel.TRAFFIC_FILTER
 * @since 26.0.0
 */
int32_t OH_TrafficFilter_UnregisterPacketCallback(OH_TrafficFilter_PacketController* controller);

/**
 * @brief Set packet filter rule
 *     Add a packet filter rule to controller chain.
 *     only packets matching the rule will be intercepted and sent to callback function.
 * @note Logical relationship:
 *     - Conditions within a single OH_TrafficFilter_FilterRule structure are combined with logical AND.
 *     - Multiple rules added to the same OH_TrafficFilter_PacketController are combined with logical OR.
 *     To clear filter rules, you need to call {@link OH_TrafficFilter_ClearPacketRule}.
 * @param controller OH_TrafficFilter_PacketController handle
 * @param rule Filter rule. Cannot be NULL.
 * @return Returns {@link OH_TRAFFICFILTER_OK} on success,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_PERMISSION_DENIED} if permission is denied,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_INVALID_PARAM} if controller or rule is NULL,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_TOO_MANY_RULES} if too many rules added.
 *         If the function fails, the rule structure memory needs to be released by the caller.
 *
 * @permission ohos.permission.kernel.TRAFFIC_FILTER
 * @since 26.0.0
 */
int32_t OH_TrafficFilter_AddPacketRule(
    OH_TrafficFilter_PacketController* controller,
    const OH_TrafficFilter_FilterRule* rule
);

/**
 * @brief Clear packet filter rule
 *
 * Clear all packet filter rules in controller.
 *
 * @param controller OH_TrafficFilter_PacketController handle
 * @return Returns {@link OH_TRAFFICFILTER_OK} on success,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_PERMISSION_DENIED} if permission is denied,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_INVALID_PARAM} if controller is NULL.
 *
 * @permission ohos.permission.kernel.TRAFFIC_FILTER
 * @since 26.0.0
 */
int32_t OH_TrafficFilter_ClearPacketRule(OH_TrafficFilter_PacketController* controller);

/**
 * @brief Creates a traffic redirection instance
 *
 * Creates a traffic redirection instance for transparent TCP traffic redirection to proxy server
 *     Resource Management: You must call {@link OH_TrafficFilter_DestroyRedirector} to release resources.
 *
 * @param group_id Redirection chain identifier.
 *                  This is the logical grouping ID within the application.
 *                  Multiple redirectors within the same application can use different group_id.
 *                  The same group_id from different applications will be automatically isolated.
 * @param priority Priority
 *                  (determines execution order between different group_id chain, smaller number executes first).
 *                  Note: Redirector priority is higher than packet filter priority.
 * @param redirector Output parameter, returns the redirection handle on success.
 * @return Returns {@link OH_TRAFFICFILTER_OK} on success,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_PERMISSION_DENIED} if permission is denied,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_GROUP_ID_IN_USE} when group_id already exists,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_INVALID_PARAM} if priority is invalid,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_NFQUEUE_ERROR} if NFQueue initialization fails.
 *
 * @permission ohos.permission.kernel.TRAFFIC_FILTER
 * @since 26.0.0
 */
int32_t OH_TrafficFilter_CreateRedirector(
    uint32_t group_id,
    uint32_t priority,
    OH_TrafficFilter_Redirector** redirector
);

/**
 * @brief Destroys a traffic redirection instance.
 *     Destroys the redirection instance and releases related resources, including rules.
 *     The handle becomes invalid after this call.
 *
 * @param redirector OH_TrafficFilter_Redirector handle
 *
 * @permission ohos.permission.kernel.TRAFFIC_FILTER
 * @since 26.0.0
 */
void OH_TrafficFilter_DestroyRedirector(OH_TrafficFilter_Redirector* redirector);

/**
 * @brief Adds a redirection rule
 *     Adds a TCP traffic redirection rule to redirect matched traffic to specified proxy server
 *     To clear redirect rules, you need to call {@link OH_TrafficFilter_ClearRedirectRule}.
 *
 * @param redirector OH_TrafficFilter_Redirector handle
 * @param rule Redirection rule. Cannot be NULL.
 * @return Returns {@link OH_TRAFFICFILTER_OK} on success,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_PERMISSION_DENIED} if permission is denied,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_INVALID_PARAM} if redirector or rule is NULL,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_TOO_MANY_RULES} if too many rules added.
 *         If the function fails, the rule structure memory needs to be released by the caller.
 *
 * @permission ohos.permission.kernel.TRAFFIC_FILTER
 * @since 26.0.0
 */
int32_t OH_TrafficFilter_AddRedirectRule(
    OH_TrafficFilter_Redirector* redirector,
    const OH_TrafficFilter_RedirectRule* rule
);

/**
 * @brief Clear all redirection rule
 *
 * @param redirector OH_TrafficFilter_Redirector handle
 * @return Returns {@link OH_TRAFFICFILTER_OK} on success,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_PERMISSION_DENIED} if permission is denied,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_INVALID_PARAM} if redirector is NULL.
 *
 * @permission ohos.permission.kernel.TRAFFIC_FILTER
 * @since 26.0.0
 */
int32_t OH_TrafficFilter_ClearRedirectRule(OH_TrafficFilter_Redirector* redirector);

/**
 * @brief Queries corresponding process information based on connection information
 *
 * Queries corresponding process information based on five-tuple information
 *
 * @param connection_info Input connection information
 * @param process_info Output process information
 * @return Returns {@link OH_TRAFFICFILTER_OK} on success,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_PERMISSION_DENIED} if permission is denied,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_INVALID_PARAM} if input parameters are invalid,
 *         Returns {@link OH_TRAFFICFILTER_ERROR_NOT_FOUND} if process not found.
 *
 * @permission ohos.permission.kernel.TRAFFIC_FILTER
 * @since 26.0.0
 */
int32_t OH_TrafficFilter_QueryProcess(
    const OH_TrafficFilter_ConnectionInfo* connection_info,
    OH_TrafficFilter_ProcessInfo* process_info
);

#ifdef __cplusplus
}
#endif

#endif /* NET_TRAFFICFILTER_H */
/** @} */
