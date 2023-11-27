/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef MDNS_CLIENT_RESUME_H
#define MDNS_CLIENT_RESUME_H

#include <map>
#include <string>

#include "singleton.h"

#include "i_mdns_event.h"
#include "mdns_service_info.h"

namespace OHOS {
namespace NetManagerStandard {
struct MyCompareSmartPointer {
    bool operator()(const sptr<IRemoteBroker> &lhs, const sptr<IRemoteBroker> &rhs) const
    {
        if ((lhs == nullptr) || (rhs == nullptr)) {
            return false;
        }

        return lhs->AsObject().GetRefPtr() < rhs->AsObject().GetRefPtr();
    }
};

typedef std::map<sptr<IRegistrationCallback>, MDnsServiceInfo, MyCompareSmartPointer> RegisterServiceMap;
typedef std::map<sptr<IDiscoveryCallback>, std::string, MyCompareSmartPointer> DiscoverServiceMap;

class MDnsClientResume {
public:
    ~MDnsClientResume() = default;
    void Init();

    static MDnsClientResume &GetInstance();

    int32_t SaveRegisterService(const MDnsServiceInfo &serviceInfo, const sptr<IRegistrationCallback> &cb);

    int32_t RemoveRegisterService(const sptr<IRegistrationCallback> &cb);

    int32_t SaveStartDiscoverService(const std::string &serviceType, const sptr<IDiscoveryCallback> &cb);

    int32_t RemoveStopDiscoverService(const sptr<IDiscoveryCallback> &cb);

    RegisterServiceMap *GetRegisterServiceMap();
    DiscoverServiceMap *GetStartDiscoverServiceMap();

private:
    MDnsClientResume() = default;

    bool initFlag_ = false;

    RegisterServiceMap registerMap_;
    DiscoverServiceMap discoveryMap_;

    std::recursive_mutex registerMutex_;
    std::recursive_mutex discoveryMutex_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // MDNS_CLIENT_H
