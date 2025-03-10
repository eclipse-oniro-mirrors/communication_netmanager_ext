/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#ifndef HWOSAPPID_H
#define HWOSAPPID_H
#include <string>
#include <vector>
#include "networksliceutil.h"

namespace OHOS {
namespace NetManagerStandard {
class HwOsAppId {
public:
    static const int NUMBER_OF_APPID_WITHOUD_OSID_PARTS = 1;
    static const int NUMBER_OF_APPID_PARTS = 2;
    static const int INVALID_INDEX = -1;
    static const std::string SEPARATOR_FOR_APPID;

    HwOsAppId() : mOsId(""), mAppId("") {}

    static HwOsAppId Create(const std::string& osAppId)
    {
        if (osAppId.empty()) {
            NETMGR_EXT_LOG_I("HwOsAppId return1");
            return HwOsAppId();
        }

        std::vector<std::string> values = Split(osAppId, SEPARATOR_FOR_APPID);
        if (values.size() == 0 || !((int)values.size() == NUMBER_OF_APPID_WITHOUD_OSID_PARTS ||
            (int)values.size() == NUMBER_OF_APPID_PARTS)) {
                return HwOsAppId();
            }

        HwOsAppId id;
        if ((int)values.size() == NUMBER_OF_APPID_PARTS) {
            id.mOsId = values[0];
            id.mAppId = values[1];
        } else {
            id.mOsId = "";
            id.mAppId = values[0];
        }
        NETMGR_EXT_LOG_I("HwOsAppId Create OsId = %{public}s, AppId = %{public}s", id.mOsId.c_str(), id.mAppId.c_str());
        return id;
    }

    std::string getOsId() const
    {
        return mOsId;
    }

    std::string getAppId() const
    {
        return mAppId;
    }

    bool operator==(const HwOsAppId& other) const
    {
        return mOsId == other.mOsId && mAppId == other.mAppId;
    }

private:
    std::string mOsId;
    std::string mAppId;
};

const inline std::string HwOsAppId::SEPARATOR_FOR_APPID = "#";
} // namespace NetManagerStandard
} // namespace OHOS
#endif  // HWOSAPPID_H
