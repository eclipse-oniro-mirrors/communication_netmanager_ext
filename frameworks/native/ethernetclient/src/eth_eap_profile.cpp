/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "eth_eap_profile.h"
 
namespace OHOS {
namespace NetManagerStandard {
 
bool EthEapProfile::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(static_cast<int32_t>(eapMethod)) ||
        !parcel.WriteInt32(static_cast<int32_t>(phase2Method))) {
        return false;
    }
    if (!parcel.WriteString(identity) || !parcel.WriteString(anonymousIdentity) ||
        !parcel.WriteString(password) || !parcel.WriteString(caCertAliases) ||
        !parcel.WriteString(caPath) || !parcel.WriteString(clientCertAliases) ||
        !parcel.WriteString(certPassword) || !parcel.WriteString(altSubjectMatch) ||
        !parcel.WriteString(domainSuffixMatch) || !parcel.WriteString(realm) ||
        !parcel.WriteString(plmn)) {
        return false;
    }
    if (!parcel.WriteInt32(eapSubId)) {
        return false;
    }
    if (!parcel.WriteInt32(certEntry.size())) {
        return false;
    }
    for (const auto entry : certEntry) {
        if (!parcel.WriteUint8(entry)) {
            return false;
        }
    }
    return true;
}
 
EthEapProfile* EthEapProfile::Unmarshalling(Parcel &parcel)
{
    std::unique_ptr<EthEapProfile> ptr = std::make_unique<EthEapProfile>();
    int32_t eapMeth;
    int32_t phase2Meth;
    if (!parcel.ReadInt32(eapMeth) || !parcel.ReadInt32(phase2Meth)) {
        return nullptr;
    }
    ptr->eapMethod = static_cast<EapMethod>(eapMeth);
    ptr->phase2Method = static_cast<Phase2Method>(phase2Meth);
    if (!parcel.ReadString(ptr->identity) || !parcel.ReadString(ptr->anonymousIdentity) ||
        !parcel.ReadString(ptr->password) || !parcel.ReadString(ptr->caCertAliases) ||
        !parcel.ReadString(ptr->caPath) || !parcel.ReadString(ptr->clientCertAliases) ||
        !parcel.ReadString(ptr->certPassword) || !parcel.ReadString(ptr->altSubjectMatch) ||
        !parcel.ReadString(ptr->domainSuffixMatch) || !parcel.ReadString(ptr->realm) ||
        !parcel.ReadString(ptr->plmn)) {
        return nullptr;
    }
    if (!parcel.ReadInt32(ptr->eapSubId)) {
        return nullptr;
    }
    int32_t size = 0;
    if (!parcel.ReadInt32(size)) {
        return nullptr;
    }
    uint8_t entry;
    for (int32_t i = 0; i < size; i++) {
        if (!parcel.ReadUint8(entry)) {
            return nullptr;
        }
        ptr->certEntry.emplace_back(entry);
    }
    return ptr.release();
}
 
} // namespace NetManagerStandard
} // namespace OHOS