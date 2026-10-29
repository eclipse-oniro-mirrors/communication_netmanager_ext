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

#ifndef ETH_EAP_CONFIG_H
#define ETH_EAP_CONFIG_H

#include "eth_eap_profile.h"
#include "eth_eap_state_info.h"
#include "parcel.h"

namespace OHOS {
namespace NetManagerStandard {

struct EthEapConfig final : public Parcelable {
    EthEapProfile profile;
    bool enabled = false;
    bool autoAuth = true;
    bool locked = false;

    bool Marshalling(Parcel &parcel) const override
    {
        if (!profile.Marshalling(parcel)) {
            return false;
        }
        if (!parcel.WriteBool(enabled)) {
            return false;
        }
        if (!parcel.WriteBool(autoAuth)) {
            return false;
        }
        if (!parcel.WriteBool(locked)) {
            return false;
        }
        return true;
    }

    static EthEapConfig *Unmarshalling(Parcel &parcel)
    {
        EthEapConfig *config = new (std::nothrow) EthEapConfig();
        if (config == nullptr) {
            return nullptr;
        }
        EthEapProfile *profile = EthEapProfile::Unmarshalling(parcel);
        if (profile == nullptr) {
            delete config;
            return nullptr;
        }
        config->profile = *profile;
        delete profile;
        if (!parcel.ReadBool(config->enabled)) {
            delete config;
            return nullptr;
        }
        if (!parcel.ReadBool(config->autoAuth)) {
            delete config;
            return nullptr;
        }
        if (!parcel.ReadBool(config->locked)) {
            delete config;
            return nullptr;
        }
        return config;
    }
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETH_EAP_CONFIG_H
