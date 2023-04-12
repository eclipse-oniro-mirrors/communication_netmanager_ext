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

#ifndef INTERFACE_OBSERVER_WRAPPER_H
#define INTERFACE_OBSERVER_WRAPPER_H

#include <initializer_list>
#include <string>

#include <napi/native_api.h>

#include "refbase.h"
#include "singleton.h"

#include "event_manager.h"
#include "interface_state_observer.h"

namespace OHOS {
namespace NetManagerStandard {
class InterfaceStateObserverWrapper {
    DISALLOW_COPY_AND_MOVE(InterfaceStateObserverWrapper);

public:
    napi_value On(napi_env env, napi_callback_info info, const std::initializer_list<std::string> &events,
                  bool asyncCallback);
    napi_value Off(napi_env env, napi_callback_info info, const std::initializer_list<std::string> &events,
                   bool asyncCallback);

    static InterfaceStateObserverWrapper &GetInstance();

    EventManager *GetEventManager() const
    {
        return manager_;
    }

private:
    InterfaceStateObserverWrapper();
    ~InterfaceStateObserverWrapper();

    sptr<InterfaceStateObserver> observer_ = nullptr;
    EventManager *manager_ = nullptr;
    bool registered_ = false;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // statistics_observer_wrapper_H
