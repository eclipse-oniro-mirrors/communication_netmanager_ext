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

#include "ethernet_service.h"

#include <sys/time.h>
#include <unistd.h>
#include <cinttypes>

#include "netmgr_ext_log_wrapper.h"
#include "ethernet_constants.h"
#include "net_manager_center.h"

namespace OHOS {
namespace NetManagerStandard {
const bool REGISTER_LOCAL_RESULT_ETH = SystemAbility::MakeAndRegisterAbility(
    DelayedSingleton<EthernetService>::GetInstance().get());

EthernetService::EthernetService()
    : SystemAbility(COMM_ETHERNET_MANAGER_SYS_ABILITY_ID, true)
{
    ethManagement_ = std::make_unique<EthernetManagement>();
}

EthernetService::~EthernetService() {}

void EthernetService::OnStart()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    NETMGR_EXT_LOG_D("EthernetService::OnStart begin timestamp [%{public}" PRId64 ".%{public}" PRId64 "]",
        tv.tv_sec, tv.tv_usec);
    if (state_ == STATE_RUNNING) {
        NETMGR_EXT_LOG_D("EthernetService the state is already running");
        return;
    }
    if (!Init()) {
        NETMGR_EXT_LOG_E("EthernetService init failed");
        return;
    }
    nlkRtnl_.Init();
    if (ethManagement_ != nullptr) {
        ethManagement_->Init();
        ethManagement_->RegisterNlk(nlkRtnl_);
    }
    state_ = STATE_RUNNING;
    gettimeofday(&tv, NULL);
    NETMGR_EXT_LOG_D("EthernetService::OnStart end timestamp [%{public}" PRId64 ".%{public}" PRId64 "]",
        tv.tv_sec, tv.tv_usec);
}

void EthernetService::OnStop()
{
    state_ = STATE_STOPPED;
    registerToService_ = false;
}

bool EthernetService::Init()
{
    if (!REGISTER_LOCAL_RESULT_ETH) {
        NETMGR_EXT_LOG_E("EthernetService Register to local sa manager failed");
        return false;
    }
    if (!registerToService_) {
        if (!Publish(DelayedSingleton<EthernetService>::GetInstance().get())) {
            NETMGR_EXT_LOG_E("EthernetService Register to sa manager failed");
            return false;
        }
        registerToService_ = true;
    }
    serviceComm_ = (std::make_unique<EthernetServiceCommon>()).release();
    NetManagerCenter::GetInstance().RegisterEthernetService(serviceComm_);
    NETMGR_EXT_LOG_D("GetEthernetServer suc");
    return true;
}

int32_t EthernetService::SetIfaceConfig(const std::string &iface, sptr<InterfaceConfiguration> &ic)
{
    NETMGR_EXT_LOG_D("EthernetService SetIfaceConfig processing");
    if (ethManagement_ != nullptr) {
        return ethManagement_->UpdateDevInterfaceState(iface, ic);
    } else {
        return ETHERNET_ERROR;
    }
}

sptr<InterfaceConfiguration> EthernetService::GetIfaceConfig(const std::string &iface)
{
    NETMGR_EXT_LOG_D("EthernetService GetIfaceConfig processing");
    if (ethManagement_ != nullptr) {
        return ethManagement_->GetDevInterfaceCfg(iface);
    } else {
        return nullptr;
    }
}

int32_t EthernetService::IsIfaceActive(const std::string &iface)
{
    NETMGR_EXT_LOG_D("EthernetService IsIfaceActive processing");
    if (ethManagement_ != nullptr) {
        return ethManagement_->IsIfaceActive(iface);
    } else {
        return ETHERNET_ERROR;
    }
}

std::vector<std::string> EthernetService::GetAllActiveIfaces()
{
    NETMGR_EXT_LOG_D("EthernetService GetAllActiveIfaces processing");
    if (ethManagement_ != nullptr) {
        return ethManagement_->GetAllActiveIfaces();
    } else {
        return {};
    }
}

int32_t EthernetService::ResetFactory()
{
    NETMGR_EXT_LOG_D("EthernetService ResetFactory processing");
    if (ethManagement_ != nullptr) {
        return ethManagement_->ResetFactory();
    } else {
        return ETHERNET_ERROR;
    }
}
} // namespace NetManagerStandard
} // namespace OHOS