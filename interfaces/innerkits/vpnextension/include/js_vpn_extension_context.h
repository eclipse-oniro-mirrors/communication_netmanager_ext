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

#ifndef OHOS_NETMANAGER_EXT_JS_VPN_EXTENSION_CONTEXT_H
#define OHOS_NETMANAGER_EXT_JS_VPN_EXTENSION_CONTEXT_H

#include <memory>

#include "ability_connect_callback.h"
#include "vpn_extension_context.h"
#include "event_handler.h"
#include "js_free_install_observer.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace NetManagerStandard {
using namespace AbilityRuntime;
napi_value CreateJsVpnExtensionContext(napi_env env, std::shared_ptr<VpnExtensionContext> context);

class JSVpnExtensionContext : public AbilityConnectCallback {
public:
    explicit JSVpnExtensionContext(napi_env env);
    ~JSVpnExtensionContext();
private:
    napi_env env_;
    std::unique_ptr<NativeReference> jsConnectionObject_ = nullptr;
};

struct ConnectionKey {
    AAFwk::Want want;
    int64_t id;
};

struct key_compare {
    bool operator()(const ConnectionKey &key1, const ConnectionKey &key2) const
    {
        if (key1.id < key2.id) {
            return true;
        }
        return false;
    }
};

static std::map<ConnectionKey, sptr<JSVpnExtensionContext>, key_compare> connects_;
static int64_t serialNumber_ = 0;
static std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;
}  // namespace NetManagerStandard
}  // namespace OHOS
#endif  // OHOS_NETMANAGER_EXT_JS_VPN_EXTENSION_CONTEXT_H
