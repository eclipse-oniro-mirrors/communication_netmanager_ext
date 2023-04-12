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

#ifndef INTERFACE_STATE_CALLBACK_H
#define INTERFACE_STATE_CALLBACK_H

#include <string>

#include "iremote_broker.h"

namespace OHOS {
namespace NetManagerStandard {
class InterfaceStateCallback : public IRemoteBroker {
public:
    virtual ~InterfaceStateCallback() = default;
    virtual int32_t OnInterfaceAdded(const std::string &ifName) = 0;
    virtual int32_t OnInterfaceRemoved(const std::string &ifName) = 0;
    virtual int32_t OnInterfaceChanged(const std::string &ifName, bool up) = 0;
    enum class Message {
        INTERFACE_STATE_ADD,
        INTERFACE_STATE_REMOVE,
        INTERFACE_STATE_CHANGE,
    };

    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.EthernetService.InterfaceStateCallback");
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // INTERFACE_STATE_CALLBACK_H
