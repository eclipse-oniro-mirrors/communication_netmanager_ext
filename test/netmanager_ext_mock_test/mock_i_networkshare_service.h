/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef MOCK_I_NETWORKSHARE_SERVICE_H
#define MOCK_I_NETWORKSHARE_SERVICE_H

#include "inetwork_share_service.h"
#include <gmock/gmock.h>

namespace OHOS {
namespace NetManagerStandard {

// Mock of the (IDL generated) INetworkShareService interface. Injected into a local
// NetworkShareClient so that client-side unit tests do not depend on the real
// networkshare service process (no real IPC, no device permission environment).
class MockINetworkShareService : public INetworkShareService {
public:
    MOCK_METHOD(sptr<IRemoteObject>, AsObject, ());

    MOCK_METHOD(ErrCode, IsNetworkSharingSupported, (int32_t & supported), (override));
    MOCK_METHOD(ErrCode, IsSharing, (int32_t & sharingStatus), (override));
    MOCK_METHOD(ErrCode, StartNetworkSharing, (int32_t type), (override));
    MOCK_METHOD(ErrCode, StopNetworkSharing, (int32_t type), (override));
    MOCK_METHOD(ErrCode, GetSharableRegexs, (int32_t type, std::vector<std::string> & ifaceRegexs), (override));
    MOCK_METHOD(ErrCode, GetSharingState, (int32_t type, int32_t & state), (override));
    MOCK_METHOD(ErrCode, GetNetSharingIfaces, (int32_t state, std::vector<std::string> & ifaces), (override));
    MOCK_METHOD(ErrCode, RegisterSharingEvent, (const sptr<ISharingEventCallback> & cb), (override));
    MOCK_METHOD(ErrCode, UnregisterSharingEvent, (const sptr<ISharingEventCallback> & cb), (override));
    MOCK_METHOD(ErrCode, GetStatsRxBytes, (int32_t & bytes), (override));
    MOCK_METHOD(ErrCode, GetStatsTxBytes, (int32_t & bytes), (override));
    MOCK_METHOD(ErrCode, GetStatsTotalBytes, (int32_t & bytes), (override));
    MOCK_METHOD(ErrCode, SetConfigureForShare, (bool enabled), (override));
};

} // namespace NetManagerStandard
} // namespace OHOS
#endif // MOCK_I_NETWORKSHARE_SERVICE_H
