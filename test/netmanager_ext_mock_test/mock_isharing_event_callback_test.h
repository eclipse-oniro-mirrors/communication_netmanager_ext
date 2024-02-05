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

#ifndef COMMON_I_SHARING_EVENT_CALLBACK_TEST_H
#define COMMON_I_SHARING_EVENT_CALLBACK_TEST_H

#include "i_sharing_event_callback.h"
#include "iremote_object.h"
#include "networkshare_service.h"

namespace OHOS {
namespace NetManagerStandard {
class MockISharingEventCallback : public IRemoteStub<ISharingEventCallback> {
public:
    void OnSharingStateChanged(const bool &isRunning)
    {
        return;
    }

    void OnInterfaceSharingStateChanged(
        const SharingIfaceType &type, const std::string &iface, const SharingIfaceState &state)
    {
        return;
    }

    void OnSharingUpstreamChanged(const sptr<NetHandle> netHandle)
    {
        return;
    }
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // COMMON_I_SHARING_EVENT_CALLBACK_TEST_H
