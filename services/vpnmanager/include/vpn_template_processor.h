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

#ifndef NET_VPN_TEMPLATE_PROCESSOR_H
#define NET_VPN_TEMPLATE_PROCESSOR_H

#include <unordered_map>
#include "ipsecvpn_config.h"
#include "l2tpvpn_config.h"
#include "net_vpn_impl.h"

namespace OHOS {
namespace NetManagerStandard {
class VpnTemplateProcessor {
public:
    int32_t BuildConfig(std::shared_ptr<NetVpnImpl> &vpnObj,
        std::map<std::string, std::shared_ptr<NetVpnImpl>> &vpnObjMap);

private:
    void GenSwanctlOrIpsecConf(sptr<IpsecVpnConfig> &ipsecConfig, sptr<L2tpVpnConfig> &l2tpConfig,
        int32_t ifNameId, std::map<std::string, std::shared_ptr<NetVpnImpl>> &vpnObjMap);
    void GenXl2tpdConf(sptr<L2tpVpnConfig> &config, int32_t ifNameId,
        std::map<std::string, std::shared_ptr<NetVpnImpl>> &vpnObjMap);
    void GenOptionsL2tpdClient(sptr<L2tpVpnConfig> &config);
    void GenIpsecSecrets(sptr<L2tpVpnConfig> &config);
    void GetSecret(sptr<IpsecVpnConfig> &ipsecConfig, int32_t ifNameId, std::string &outSecret);
    void GetConnect(sptr<IpsecVpnConfig> &ipsecConfig, int32_t ifNameId, std::string &outConnect);
    void CreateConnectAndSecret(sptr<IpsecVpnConfig> &ipsecConfig, sptr<L2tpVpnConfig> &l2tpConfig,
        int32_t ifNameId, std::string &outConnect, std::string &outSecret);
    void CreateXl2tpdConf(sptr<L2tpVpnConfig> &config, int32_t ifNameId, std::string &outConf);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_VPN_TEMPLATE_PROCESSOR_H
