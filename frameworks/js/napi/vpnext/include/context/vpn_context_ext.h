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

#ifndef VPN_CREATEVPN_CONTEXT_H
#define VPN_CREATEVPN_CONTEXT_H

#include <cstddef>
#include <napi/native_api.h>

#include "base_context.h"
#include "event_manager.h"
#include "refbase.h"
#include "vpn_config.h"

namespace OHOS {
namespace NetManagerStandard {
class VpnContext : public BaseContext {
public:
    VpnContext() = delete;
    VpnContext(napi_env env, EventManager *manager);

    void ParseParams(napi_value *params, size_t paramsCount);

public:
    sptr<VpnConfig> vpnConfig_ = nullptr;
    int fd_ = -1;

private:
    bool ParseVpnConfig(napi_value *params);
    bool ParseAddrRouteParams(napi_value config);
    bool ParseChoiceableParams(napi_value config);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // VPN_CREATEVPN_CONTEXT_H