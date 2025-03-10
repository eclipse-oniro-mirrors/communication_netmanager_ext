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

#include "networkslice_service_base.h"
#include "networkslice_loop_manager.h"
#include "networkslicemsgcenter.h"

namespace OHOS {
namespace NetManagerStandard {

NetworkSliceServiceBase::NetworkSliceServiceBase(NetworkSliceSubModule moduleId)
    : AppExecFwk::EventHandler(
    DelayedSingleton<NetworkSlice_Loop_Manager>::GetInstance()->getLoop(OLLIE_MESSAGE_THREAD)), moduleId_(moduleId) {}

NetworkSliceServiceBase::~NetworkSliceServiceBase() = default;

void NetworkSliceServiceBase::Init()
{
    NETMGR_EXT_LOG_I("NetworkSliceServiceBase init start.");
    Singleton<NetworkSliceMsgCenter>::GetInstance().registHandler(moduleId_, shared_from_this());
    OnInit();
    NETMGR_EXT_LOG_I("NetworkSliceServiceBase init end.");
}

void NetworkSliceServiceBase::Subscribe(NetworkSliceEvent event)
{
    Singleton<NetworkSliceMsgCenter>::GetInstance().Subscribe(event, moduleId_);
}

void NetworkSliceServiceBase::UnSubscribe(NetworkSliceEvent event)
{
    Singleton<NetworkSliceMsgCenter>::GetInstance().UnSubscribe(event, moduleId_);
}

void NetworkSliceServiceBase::RecvKernelData(void *rcvMsg, int32_t dataLen)
{
    return;
}

}
}
