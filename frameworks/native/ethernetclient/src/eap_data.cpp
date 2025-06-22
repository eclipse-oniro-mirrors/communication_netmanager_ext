/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
 
#include "eap_data.h"
#include <sstream>
#include "netmanager_base_log.h"
#include "refbase.h"
 
namespace OHOS {
namespace NetManagerStandard {
 
EapData::EapData()
{
}
 
EapData::~EapData()
{
}
 
bool EapData::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(eapCode)) {
        return false;
    }
    if (!parcel.WriteInt32(eapType)) {
        return false;
    }
    if (!parcel.WriteInt32(msgId)) {
        return false;
    }
    if (!parcel.WriteInt32(bufferLen)) {
        return false;
    }
    if (!parcel.WriteUInt8Vector(eapBuffer)) {
        return false;
    }
    return true;
}
 
EapData* EapData::Unmarshalling(Parcel &parcel)
{
    std::unique_ptr<EapData> ptr = std::make_unique<EapData>();
    if (ptr == nullptr) {
        return nullptr;
    }
    bool allOk = parcel.ReadInt32(ptr->eapCode) && parcel.ReadInt32(ptr->eapType) && parcel.ReadInt32(ptr->msgId) &&
        parcel.ReadInt32(ptr->bufferLen) && parcel.ReadInt32(&ptr->eapBuffer);
    return allOk ? ptr.release() : nullptr;
}
 
std::string EapData::PrintLogInfo()
{
    std::ostringstream ss;
    ss << "code:" << eapCode << " ";
    ss << "type:" << eapType << " ";
    ss << "msgId:" << msgId << " ";
    ss << "bufferLen:" << bufferLen;
    return ss.str();
}
 
} // namespace NetManagerStandard
} // namespace OHOS