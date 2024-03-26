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

#include "interface_state_observer.h"

#include "constant.h"
#include "interface_state_observer_wrapper.h"
#include "netmanager_ext_log.h"

static constexpr const char *KEY_IFACE = "iface";
static constexpr const char *KEY_ACTIVE = "active";
static constexpr const char *EVENT_STATS_CHANGE = "interfaceStateChange";

namespace OHOS {
namespace NetManagerStandard {
int32_t InterfaceStateObserver::OnInterfaceAdded(const std::string &iface)
{
    if (!InterfaceStateObserverWrapper::GetInstance().GetEventManager()->HasEventListener(EVENT_STATS_CHANGE)) {
        NETMANAGER_EXT_LOGE("no event listener find %{public}s", EVENT_STATS_CHANGE);
        return 0;
    }
    auto pair = new std::pair<std::string, bool>(iface, true);
    InterfaceStateObserverWrapper::GetInstance().GetEventManager()->EmitByUv(EVENT_STATS_CHANGE, pair,
                                                                             IfaceChangedCallback);
    return 0;
}

int32_t InterfaceStateObserver::OnInterfaceRemoved(const std::string &iface)
{
    if (!InterfaceStateObserverWrapper::GetInstance().GetEventManager()->HasEventListener(EVENT_STATS_CHANGE)) {
        NETMANAGER_EXT_LOGE("no event listener find %{public}s", EVENT_STATS_CHANGE);
        return -1;
    }
    auto pair = new std::pair<std::string, bool>(iface, false);
    InterfaceStateObserverWrapper::GetInstance().GetEventManager()->EmitByUv(EVENT_STATS_CHANGE, pair,
                                                                             IfaceChangedCallback);
    return 0;
}

int32_t InterfaceStateObserver::OnInterfaceChanged(const std::string &iface, bool up)
{
    if (!InterfaceStateObserverWrapper::GetInstance().GetEventManager()->HasEventListener(EVENT_STATS_CHANGE)) {
        NETMANAGER_EXT_LOGE("no event listener find %{public}s", EVENT_STATS_CHANGE);
        return -1;
    }
    auto pair = new std::pair<std::string, bool>(iface, up);
    InterfaceStateObserverWrapper::GetInstance().GetEventManager()->EmitByUv(EVENT_STATS_CHANGE, pair,
                                                                             IfaceChangedCallback);
    return 0;
}

napi_value InterfaceStateObserver::CreateIfaceChangedParam(napi_env env, void *data)
{
    auto pair(reinterpret_cast<std::pair<std::string, bool> *>(data));
    napi_value obj = NapiUtils::CreateObject(env);
    if (pair != nullptr) {
        NapiUtils::SetStringPropertyUtf8(env, obj, KEY_IFACE, pair->first);
        NapiUtils::SetBooleanProperty(env, obj, KEY_ACTIVE, pair->second);
    }
    delete pair;
    return obj;
}

void InterfaceStateObserver::IfaceChangedCallback(uv_work_t *work, int status)
{
    CallbackTemplate<CreateIfaceChangedParam>(work, status);
}
} // namespace NetManagerStandard
} // namespace OHOS
