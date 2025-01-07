/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "networkshare_notification.h"

#include "ability_manager_ipc_interface_code.h"
#include "extension_manager_client.h"
#include "iservice_registry.h"
#include "message_parcel.h"
#include "system_ability_definition.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {

NetworkShareNotification &NetworkShareNotification::GetInstance()
{
    static NetworkShareNotification instance;
    return instance;
}

void NetworkShareNotification::PublishNetworkShareNotification(NotificationId notificationId)
{
    NETMGR_EXT_LOG_I("Publishing notification, id [%{public}d]", static_cast<int>(notificationId));
    AAFwk::Want want;
    want.SetElementName("com.ohos.locationdialog", "HotSpotServiceAbility");
    want.SetParam("operateType", static_cast<int>(NotificationOpetationType::PUBLISH));
    want.SetParam("notificationId", static_cast<int>(notificationId));
    auto result = StartAbility(want);
    isNtfPublished = true;
    NETMGR_EXT_LOG_I("Publishing notification End, result = %{public}d", result);
}

void NetworkShareNotification::CancelNetworkShareNotification(NotificationId notificationId)
{
    NETMGR_EXT_LOG_I("Cancel notification, id [%{public}d]", static_cast<int>(notificationId));
    if (!isNtfPublished) {
        return;
    }
    AAFwk::Want want;
    want.SetElementName("com.ohos.locationdialog", "HotSpotServiceAbility");
    want.SetParam("operateType", static_cast<int>(NotificationOpetationType::CANCEL));
    want.SetParam("notificationId", static_cast<int>(notificationId));
    auto result = StartAbility(want);
    isNtfPublished = false;
    NETMGR_EXT_LOG_I("Cancel notification End, result = %{public}d", result);
}

int32_t NetworkShareNotification::StartAbility(OHOS::AAFwk::Want& want)
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityManager == nullptr) {
        NETMGR_EXT_LOG_E("systemAbilityManager is nullptr");
        return -1;
    }
    sptr<IRemoteObject> remote = systemAbilityManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    if (remote == nullptr) {
        NETMGR_EXT_LOG_E("remote is nullptr");
        return -1;
    }

    int error;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
 
    if (!data.WriteInterfaceToken(ABILITY_MGR_DESCRIPTOR)) {
        return -1;
    }
    if (!data.WriteParcelable(&want)) {
        NETMGR_EXT_LOG_E("want write failed.");
        return -1;
    }
 
    if (!data.WriteInt32(DEFAULT_INVAL_VALUE)) {
        NETMGR_EXT_LOG_E("userId write failed.");
        return -1;
    }
 
    if (!data.WriteInt32(DEFAULT_INVAL_VALUE)) {
        NETMGR_EXT_LOG_E("requestCode write failed.");
        return -1;
    }
    uint32_t task =  static_cast<uint32_t>(AAFwk::AbilityManagerInterfaceCode::START_ABILITY);
    error = remote->SendRequest(task, data, reply, option);
    if (error != NO_ERROR) {
        NETMGR_EXT_LOG_E("Send request error: %{public}d", error);
        return error;
    }
    return reply.ReadInt32();
}
} // namespace NetManagerStandard
} // namespace OHOS