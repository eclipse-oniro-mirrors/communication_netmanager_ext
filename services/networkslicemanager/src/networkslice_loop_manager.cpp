/*
 * Copyright (C) 2024-2025 Huawei Device Co., Ltd.
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

#include "networkslice_loop_manager.h"
#include "networkslice_submodule.h"
#include "parameters.h"

namespace OHOS {
namespace NetManagerStandard {
const std::string NETMANAGER_EXT_NETWORKSLICE_ABILITY = "persist.netmgr_ext.networkslice";
NetworkSlice_Loop_Manager::NetworkSlice_Loop_Manager()
{
    NETMGR_EXT_LOG_I("NetworkSlice_Loop_Manager start");
    bool isSupportNrSlice = OHOS::system::GetBoolParameter(NETMANAGER_EXT_NETWORKSLICE_ABILITY, false);
    if (isSupportNrSlice) {
        initLoopMap();
        NETMGR_EXT_LOG_I("NetworkSlice_Loop_Manager end");
    }
}
NetworkSlice_Loop_Manager::~NetworkSlice_Loop_Manager() = default;

void NetworkSlice_Loop_Manager::initLoopMap()
{
    NETMGR_EXT_LOG_I("initLoopMap start");
    std::unordered_map<LooperType, std::string> loopTypeToString = {
        {OLLIE_MESSAGE_THREAD, "OllieMsgThread"}
    };

    for (auto it = loopTypeToString.begin(); it != loopTypeToString.end(); it++) {
        auto loop = AppExecFwk::EventRunner::Create(it->second);
        if (loop == nullptr) {
            NETMGR_EXT_LOG_E("Lopper Create failed. %{public}s", it->second.c_str());
            continue;
        }
        loopMap_.insert(std::make_pair(it->first, loop));
    }
    NETMGR_EXT_LOG_I("initLoopMap end");
}

std::shared_ptr<AppExecFwk::EventRunner> NetworkSlice_Loop_Manager::getLoop(LooperType type)
{
    NETMGR_EXT_LOG_I("getLoop start");
    auto it = loopMap_.find(type);
    if (it != loopMap_.end()) {
        return it->second;
    }
    NETMGR_EXT_LOG_I("getLoop null");
    return nullptr;
}
} // namespace NetManagerStandard
} // namespace OHOS
