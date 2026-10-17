/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef ETH_EAP_STATE_INFO_H
#define ETH_EAP_STATE_INFO_H

#include <string>
#include "parcel.h"

namespace OHOS {
namespace NetManagerStandard {

enum class EthEapState {
    STATE_IDLE = 0,
    STATE_AUTHENTICATING = 1,
    STATE_AUTHENTICATED = 2,
    STATE_FAILED = 3,
    STATE_RETRYING = 4,
};

struct EthEapStateInfo final : public Parcelable {
    EthEapState state = EthEapState::STATE_IDLE;
    int32_t retryCount = 0;
    std::string message;

    bool Marshalling(Parcel &parcel) const override
    {
        if (!parcel.WriteInt32(static_cast<int32_t>(state))) {
            return false;
        }
        if (!parcel.WriteInt32(retryCount)) {
            return false;
        }
        if (!parcel.WriteString(message)) {
            return false;
        }
        return true;
    }

    static EthEapStateInfo *Unmarshalling(Parcel &parcel)
    {
        EthEapStateInfo *info = new (std::nothrow) EthEapStateInfo();
        if (info == nullptr) {
            return nullptr;
        }
        int32_t stateVal = 0;
        if (!parcel.ReadInt32(stateVal)) {
            delete info;
            return nullptr;
        }
        info->state = static_cast<EthEapState>(stateVal);
        if (!parcel.ReadInt32(info->retryCount)) {
            delete info;
            return nullptr;
        }
        if (!parcel.ReadString(info->message)) {
            delete info;
            return nullptr;
        }
        return info;
    }
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETH_EAP_STATE_INFO_H
