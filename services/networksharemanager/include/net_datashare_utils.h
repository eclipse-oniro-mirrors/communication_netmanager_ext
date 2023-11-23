/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef NET_DATASHARE_UTILS_H
#define NET_DATASHARE_UTILS_H

#include <memory>
#include <utility>

#include "datashare_helper.h"
#include "datashare_predicates.h"
#include "datashare_result_set.h"
#include "datashare_values_bucket.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "uri.h"

namespace OHOS {
namespace NetManagerStandard {
namespace {
constexpr const char *SHARING_WIFI_URI = 
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=sharing_wifi";
constexpr const char *SHARING_USB_URI = 
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=sharing_usb";
constexpr const char *SHARING_BLUETOOTH_URI = 
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true&key=sharing_bluetooth";

constexpr const char *KEY_SHARING_WIFI = "settings.netmanager.sharing_wifi";
constexpr const char *KEY_SHARING_USB = "settings.netmanager.sharing_usb";
constexpr const char *KEY_SHARING_BLUETOOTH = "settings.netmanager.sharing_bluetooth";

} // namespace

class NetDataShareHelperUtils final {
public:
    NetDataShareHelperUtils();
    ~NetDataShareHelperUtils() = default;
    int32_t Query(Uri &uri, const std::string &key, std::string &value);
    int32_t Insert(Uri &uri, const std::string &key, const std::string &value);
    int32_t Update(Uri &uri, const std::string &key, const std::string &value);

private:
    std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelper();

private:
    std::shared_ptr<DataShare::DataShareHelper> dataShareHelper_ = nullptr;
};
} // namespace NetManagerStandard
} // namespace OHOS
#endif // NET_DATASHARE_UTILS_H