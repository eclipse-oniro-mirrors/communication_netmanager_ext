/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef ETHERNET_MANAGEMENT_H
#define ETHERNET_MANAGEMENT_H

#include <map>
#include <mutex>
#include <dirent.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#include "netd_controller_callback.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "dev_interface_state.h"
#include "nlk_event_handle.h"
#include "netLink_rtnl.h"

namespace OHOS {
namespace NetManagerStandard {
class EthernetManagement : public NlkEventHandle {
    class NotifyCallback : public NetdControllerCallback {
    public:
        NotifyCallback(EthernetManagement &ethernetManagement);
        ~NotifyCallback() override;
        int32_t OnInterfaceAddressUpdated(const std::string &, const std::string &, int, int) override;
        int32_t OnInterfaceAddressRemoved(const std::string &, const std::string &, int, int) override;
        int32_t OnInterfaceAdded(const std::string &) override;
        int32_t OnInterfaceRemoved(const std::string &) override;
        int32_t OnInterfaceChanged(const std::string &, bool) override;
        int32_t OnInterfaceLinkStateChanged(const std::string &, bool) override;
        int32_t OnRouteChanged(bool, const std::string &, const std::string &, const std::string &) override;
        int32_t OnDhcpSuccess(NetdControllerCallback::DhcpResult &dhcpResult) override;
    private:
        EthernetManagement &ethernetManagement_;
    };
public:
    EthernetManagement();
    ~EthernetManagement();
    void Init();
    void UpdateInterfaceState(const std::string &dev, bool up, bool lowerUp);
    int32_t UpdateDevInterfaceState(const std::string &iface, sptr<InterfaceConfiguration> cfg);
    int32_t UpdateDevInterfaceLinkInfo(NetdControllerCallback::DhcpResult &dhcpResult);
    sptr<InterfaceConfiguration> GetDevInterfaceCfg(const std::string &iface);
    int32_t IsIfaceActive(const std::string &iface);
    std::vector<std::string> GetAllActiveIfaces();
    int32_t ResetFactory();
    void RegisterNlk(NetLinkRtnl &nlk);
    void Handle(const struct NlkEventInfo &info) override;

private:
    bool IsDirExist(const std::string &dirPath);
    bool CreateDir(const std::string &dirPath);
    bool DelDir(const std::string &dirPath);
    bool IsFileExist(const std::string &filePath);
    bool ReadFile(const std::string &filePath, std::string &fileContent);
    bool WriteFile(const std::string &filePath, const std::string &fileContent);
    void ReadFileList(const std::string &dirPath, std::map<std::string, sptr<InterfaceConfiguration>> &devCfgs);
    void ParserFileConfig(const std::string &fileContent, std::string &iface, sptr<InterfaceConfiguration> cfg);
    void GenCfgContent(const std::string &iface, sptr<InterfaceConfiguration> cfg, std::string &fileContent);
    bool IsValidIPV4(const std::string &ip);
    bool IsValidIPV6(const std::string &ip);
    int8_t GetAddrFamily(const std::string &ip);
    int32_t Ipv4PrefixLen(const std::string &ip);
    void StartDhcpClient(const std::string &dev, sptr<DevInterfaceState> &devState);
    void StopDhcpClient(const std::string &dev, sptr<DevInterfaceState> &devState);
    void SetDevState(sptr<DevInterfaceState> &devState, const std::string &devName,
        const std::vector<uint8_t> &hwAddr, bool up, bool lowerUp);

private:
    sptr<NotifyCallback> notifyCallback_ = nullptr;
    std::map<std::string, sptr<DevInterfaceState>> devs_;
    std::string configDir_ = "/data/ethernet/";
    std::mutex mutex_;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // ETHERNET_MANAGEMENT_H