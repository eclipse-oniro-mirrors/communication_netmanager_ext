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
 
#ifndef OHOS_EAP_HDI_WPA_MANAGER_H
#define OHOS_EAP_HDI_WPA_MANAGER_H
 
#ifdef NET_EXTENSIBLE_AUTHENTICATION
 
#include <vector>
#include <mutex>
#include <pthread.h>
 
#include "eap_data.h"
#include "eth_eap_profile.h"
#include "devmgr_hdi.h"
#include "hdf_remote_service.h"
#include "servmgr_hdi.h"
#include "v1_0/iethernet.h"
#include "v1_0/iethernet_callback.h"
 
namespace OHOS {
namespace NetManagerStandard {
 
class EapHdiWpaManager : public std::enable_shared_from_this<EapHdiWpaManager> {
public:
    EapHdiWpaManager();
    ~EapHdiWpaManager() = default;
 
    int32_t LoadEthernetHdiService();
    int32_t StartEap(const std::string& ifName, const EthEapProfile& profile);
    int32_t StopEap(const std::string& ifName);
    int32_t RegisterCustomEapCallback(const std::string &ifName, const std::string &regCmd);
    int32_t ReplyCustomEapData(const std::string &ifName, int32_t result, const sptr<EapData> &eapData);
 
private:
    int32_t SetEapConfig(const EthEapProfile& config, const std::string& ifName);
    int32_t EapShellCmd(const std::string& ifName, const std::string& cmd);
    int32_t RegisterEapEventCallback(const std::string& ifName);
    int32_t UnregisterEapEventCallback(const std::string& ifName);
    static int32_t OnEapEventReport(IEthernetCallback *self, const char *ifName, const char *value);
    
    bool WriteEapConfigToFile(const std::string &fileContext);
    std::string Phase2MethodToStr(EapMethod eap, Phase2Method method);
    void RemoveHistoryCtrl();
    void UnloadDeviceManager();
    static bool ConvertStrToInt(const std::string &str, int32_t &value);
 
private:
    struct IEthernet *iEthernet_ = nullptr;
    struct HDIDeviceManager *devMgr_ = nullptr;
    std::mutex wpaMutex_;
    struct IEthernetCallback ethCallback_;
};
}  // namespace NetManagerStandard
}  // namespace OHOS
#endif // NET_EXTENSIBLE_AUTHENTICATION
#endif // OHOS_EAP_HDI_WPA_CALLBACK_H
