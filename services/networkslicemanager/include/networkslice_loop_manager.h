/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef NETWORKSLICE_LOOP_MANAGER
#define NETWORKSLICE_LOOP_MANAGER
#pragma once
#include <string>
#include <memory>
#include <unordered_map>

#include "singleton.h"
#include "event_runner.h"
#include "netmgr_ext_log_wrapper.h"

namespace OHOS {
namespace NetManagerStandard {

enum LooperType {
    OLLIE_MESSAGE_THREAD
};

class NetworkSlice_Loop_Manager final {
    DECLARE_DELAYED_SINGLETON(NetworkSlice_Loop_Manager);
public:
    void init();

    bool submitTask();
    std::shared_ptr<AppExecFwk::EventRunner> getLoop(LooperType type);

private:
    void initLoopMap();
    std::unordered_map<LooperType, std::shared_ptr<AppExecFwk::EventRunner>> loopMap_;
};
} // namespace NetManagerStandard
} // namespace OHOS

#endif  // NETWORKSLICE_LOOP_MANAGER
