/*
 * Copyright (C) 2023-2024 Huawei Device Co., Ltd.
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
 
#include "net_datashare_utils_iface.h"
#include "net_datashare_utils.h"
#include "parameters.h"
 
namespace OHOS {
namespace NetManagerStandard {
std::unique_ptr<NetDataShareHelperUtils> NetDataShareHelperUtilsIface::dataShareHelperUtils_ =
    std::make_unique<NetDataShareHelperUtils>();
constexpr const char* MOCK_DATA_SHARE_PROP = "persist.edm.mock_data_share_prop";
 
int32_t NetDataShareHelperUtilsIface::Query(const std::string &strUri, const std::string &key, std::string &value)
{
    value = system::GetParameter(MOCK_DATA_SHARE_PROP, "");
    return 0;
}
 
int32_t NetDataShareHelperUtilsIface::Insert(const std::string &strUri, const std::string &key,
                                             const std::string &value)
{
    return 0;
}
 
int32_t NetDataShareHelperUtilsIface::Update(const std::string &strUri, const std::string &key,
                                             const std::string &value)
{
    system::SetParameter(MOCK_DATA_SHARE_PROP, value);
    return 0;
}
 
int32_t NetDataShareHelperUtilsIface::Delete(const std::string &strUri, const std::string &key)
{
    return 0;
}
 
int32_t NetDataShareHelperUtilsIface::RegisterObserver(const std::string &strUri, const std::function<void()> &onChange)
{
    return 0;
}
 
int32_t NetDataShareHelperUtilsIface::UnregisterObserver(const std::string &strUri, int32_t callbackId)
{
    return 0;
}
} // namespace NetManagerStandard
} // namespace OHOS