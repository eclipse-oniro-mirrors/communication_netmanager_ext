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

#include "ipsecvpn_config.h"
#include "l2tpvpn_config.h"

namespace OHOS {
namespace NetManagerStandard {
class VpnTemplateProcessor {
public:
    int32_t BuildConfig(sptr<L2tpVpnConfig> &l2tpConfig);
    int32_t BuildConfig(sptr<IpsecVpnConfig> &ipsecConfig);

private:
    void GenSwanctlConf(sptr<IpsecVpnConfig> &config);
    void GenXl2tpdConf(sptr<L2tpVpnConfig> &config);
    void GenOptionsL2tpdClient(sptr<L2tpVpnConfig> &config);
    void GenIpsecConf(sptr<L2tpVpnConfig> &config);
    void GenIpsecSecrets(sptr<L2tpVpnConfig> &config);
    void GenStrongSwanConf(int32_t vpnType, std::string &outConf);
    void InflateConf(std::string &conf,
        const std::unordered_map<std::string, std::string>& params);
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_VPN_TEMPLATE_PROCESSOR_H
