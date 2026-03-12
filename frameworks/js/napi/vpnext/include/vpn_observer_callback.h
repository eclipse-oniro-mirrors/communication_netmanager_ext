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

#ifndef NETMANAGER_EXT_VPN_OBSERVER_CALLBACK_H
#define NETMANAGER_EXT_VPN_OBSERVER_CALLBACK_H

#include <condition_variable>
#include <mutex>
#include <string>

#include "event_manager.h"
#include "iremote_stub.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_utils.h"
#include "netmanager_ext_log.h"

namespace OHOS {
namespace NetManagerStandard {
class VpnObserver : public RefBase {
public:
    VpnObserver() = default;
    ~VpnObserver() = default;
    int32_t HandleAuthorizeResult(bool isAuthorized);
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // NETMANAGER_EXT_VPN_OBSERVER_CALLBACK_H
