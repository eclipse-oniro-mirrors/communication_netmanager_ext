/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "nrunsolicitedmsgparser.h"
#include "cellular_data_client.h"
#include "core_service_client.h"
#include "networkslicemanager.h"
#include "state_utils.h"

namespace OHOS {
namespace NetManagerStandard {
static std::string DEFAULT_PLMN = "00101";
NrUnsolicitedMsgParser& NrUnsolicitedMsgParser::GetInstance()
{
    static NrUnsolicitedMsgParser instance;
    return instance;
}

NrUnsolicitedMsgParser::NrUnsolicitedMsgParser()
{
}

std::string NrUnsolicitedMsgParser::GetHplmn()
{
    return (mEhplmns.size() > 0) ? mEhplmns[0] : DEFAULT_PLMN;
}

} // namespace NetManagerStandard
} // namespace OHOS
