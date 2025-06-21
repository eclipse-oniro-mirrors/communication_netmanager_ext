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
 
#ifndef EAP_DATA_H
#define EAP_DATA_H
 
#include "parcel.h"
#include "refbase.h"
 
namespace OHOS {
namespace NetManagerStandard {
 
#define EAP_CODE_MIN 1
#define EAP_CODE_MAX 4
#define EAP_TYPE_MIN 0
#define EAP_TYPE_MAX 255
 
enum class NetEapIpcCode {
    NET_EAP_POSTBACK = 0,
    NET_REGISTER_CUSTOM_EAP_CALLBACK = 1,
    NET_REPLY_CUSTOM_EAPDATA = 2,
};
 
enum class CustomResult {
    RESULT_FAIL = 0,
    RESULT_NEXT = 1,
    RESULT_FINISH = 2,
};
 
enum class NetType {
    WLAN0 = 1,
    ETH0 = 2,
    INVALID
};
 
enum class RegTriggerMode {
    SA_LAUNCH = 0,
    USER_REGISTER = 1,
    UNREGISTER =2,
    INVALID
};
 
#define NET_SYMBOL_VISIBLE __attribute__ ((visibility("default")))
struct NET_SYMBOL_VISIBLE EapData final : public Parcelable {
    int32_t eapCode = -1;
    int32_t eapType = -1;
    int32_t msgId = -1;
    int32_t bufferLen = -1;
    std::vector<uint8_t> eapBuffer;
 
    EapData();
    ~EapData();
    bool Marshalling(Parcel &parcel) const override;
    static sptr<EapData> Unmarshalling(Parcel &parcel);
    std::string PrintLogInfo();
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // EAP_DATA_H