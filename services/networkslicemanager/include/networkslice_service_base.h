/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef NETWORKSLICE_SERVICE_BASE_H
#define NETWORKSLICE_SERVICE_BASE_H

#include <vector>
#include <unordered_map>
#include <memory>
#include "networkslice_event.h"
#include "networkslice_submodule.h"
#include "event_handler.h"

namespace OHOS {
namespace NetManagerStandard {
class NetworkSliceServiceBase : public AppExecFwk::EventHandler {
public:
    NetworkSliceServiceBase(NetworkSliceSubModule moduleId);
    virtual ~NetworkSliceServiceBase();
    void Init();
    virtual void OnInit() {};
    virtual void RecvKernelData(void *rcvMsg, int32_t dataLen);

protected:
    void Subscribe(NetworkSliceEvent event);
    void UnSubscribe(NetworkSliceEvent event);
    NetworkSliceSubModule moduleId_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif
