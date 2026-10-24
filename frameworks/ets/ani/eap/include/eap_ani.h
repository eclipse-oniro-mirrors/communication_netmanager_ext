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

#ifndef EAP_ANI_H
#define EAP_ANI_H

#include <cstdint>
#include <string>
#include <vector>

#include "cxx.h"
#include "net_eap_callback_stub.h"
#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
#include "eth_eap_state_callback_stub.h"
#endif

namespace OHOS {
namespace NetManagerAni {

int32_t RegCustomEapHandler(int32_t net_type, int32_t eap_code, int32_t eap_type);
int32_t UnregCustomEapHandler(int32_t net_type, int32_t eap_code, int32_t eap_type);
int32_t ReplyCustomEapData(int32_t result, int32_t msg_id, int32_t buffer_len, const rust::Vec<uint8_t> &eap_buffer);
struct EthEapConfig;
int32_t StartEthEap(int32_t net_id, EthEapConfig config);
int32_t LogOffEthEap(int32_t net_id);

struct EthEapConfigAni;
struct EthEapStateInfoAni;
int32_t SetEthEapConfig(EthEapConfigAni config);
int32_t GetEthEapConfig(EthEapConfigAni &config);
int32_t DeleteEthEapConfig();
int32_t GetEthEapStateInfo(EthEapStateInfoAni &state_info);
int32_t RegisterEthEapStateCallback();
int32_t UnregisterEthEapStateCallback();
int32_t ConnectEthEap(bool connect);

struct EapAniData;

#ifdef NETMANAGER_EXT_ETHERNET_ENABLE_DISABLE
class EthEapStateCallbackAni : public NetManagerStandard::EthEapStateCallbackStub {
public:
    int32_t OnEthEapStateChanged(const NetManagerStandard::EthEapStateInfo &stateInfo) override;
};
#endif

class EapCallbackObserverAni : public NetManagerStandard::NetEapPostbackCallbackStub {
public:
    int32_t OnEapSupplicantPostback(NetManagerStandard::NetType netType,
                                     const sptr<NetManagerStandard::EapData> &eapData) override;
};

} // namespace NetManagerAni
} // namespace OHOS

#endif // EAP_ANI_H
