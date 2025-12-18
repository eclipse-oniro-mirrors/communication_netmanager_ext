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

#ifndef I_NET_INTERCEPT_RECORD_CALLBACK_H
#define I_NET_INTERCEPT_RECORD_CALLBACK_H

#include <string>
#include <vector>
#include <iremote_broker.h>
#include "netfirewall_parcel.h"

namespace OHOS {
namespace NetManagerStandard {
class INetInterceptRecordCallback : public IRemoteBroker {
public:
    virtual ~INetInterceptRecordCallback() = default;
    virtual int32_t OnInterceptRecord(const sptr<NetManagerStandard::InterceptRecord> &record) = 0;
    enum {
        ON_INTERCEPT_RECORD,
    };
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.NetManagerStandard.INetInterceptRecordCallback");
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // I_NET_INTERCEPT_RECORD_CALLBACK_H
