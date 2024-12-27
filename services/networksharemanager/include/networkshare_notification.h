/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef NETWORKSHARE_NOTIFICATION_H
#define NETWORKSHARE_NOTIFICATION_H

#include <atomic>
#include <string>
#include "ability_connect_callback_stub.h"
#include "ipc_skeleton.h"
#include "want.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {

inline const std::u16string ABILITY_MGR_DESCRIPTOR = u"ohos.aafwk.AbilityManager";
constexpr int DEFAULT_INVAL_VALUE = -1;
enum NotificationId {
    WIFI_PORTAL_NOTIFICATION_ID = 101000,
    HOTSPOT_IDLE_NOTIFICATION_ID
};

enum NotificationOpetationType {
    CANCEL = 0,
    PUBLISH = 1
};

class NetworkShareNotification {
public:
    static NetworkShareNotification& GetInstance(void);

    void PublishNetworkShareNotification(NotificationId notificationId);
    void CancelNetworkShareNotification(NotificationId notificationId);
private:
    int32_t StartAbility(OHOS::AAFwk::Want& want);

private:
    std::atomic<bool> isNtfPublished {false};
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETWORKSHARE_SERVICE_H